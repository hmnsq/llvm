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

فراغ IO::set_Allow_Unknown_مبوداي(منطقي Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//شغرو
//===----------------------------------------------------------------------===//

شغرو::شغرو(StringRef شغرو_Content, فراغ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد Stream(شغرو_Content, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

شغرو::شغرو(MemoryBufferRef شغرو, فراغ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد Stream(شغرو, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

شغرو::~شغرو() = افتراضي;

std::error_code شغرو::error() { أرجع EC; }

منطقي شغرو::outputting() ثابت {
  أرجع خطأ;
}

منطقي شغرو::setCurrentDocument() {
  إذا (DocIterator != Strm->end()) {
    كلي *N = DocIterator->get_سيو();
    إذا (!N) {
      EC = make_error_code(errc::invalid_argument);
      أرجع خطأ;
    }

    إذا (isa<Null_كلي>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      أرجع setCurrentDocument();
    }
    releaseHNodeBuffers();
    كولي_كلي = createHNodes(N);
    Current_كلي = كولي_كلي;
    أرجع true;
  }
  أرجع خطأ;
}

منطقي شغرو::nextDocument() {
  أرجع ++DocIterator != Strm->end();
}

ثابت كلي *شغرو::get_Current_كلي() ثابت {
  أرجع Current_كلي ? Current_كلي->كلي_2 : nullptr;
}

منطقي شغرو::mapTag(StringRef Tag, منطقي Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  إذا (!Current_كلي)
    أرجع خطأ;

  std::string foundTag = Current_كلي->كلي_2->getVerbatimTag();
  إذا (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    أرجع Default;
  }
  // Return true iff found tag matches supplied tag.
  أرجع Tag == foundTag;
}

فراغ شغرو::beginMapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_كلي);
  إذا (MN) {
    MN->Valid_مبوداي.clear();
  }
}

std::vector<StringRef> شغرو::مبوداي() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_كلي);
  std::vector<StringRef> Ret;
  إذا (!MN) {
    setError(Current_كلي, "not a mapping");
    أرجع Ret;
  }
  لكل (auto &P : MN->Mapping)
    Ret.تور_back(P.first());
  أرجع Ret;
}

منطقي شغرو::preflight_مبودي(StringRef مبودي, منطقي Required, منطقي, منطقي &UseDefault,
                         فراغ *&SaveInfo) {
  UseDefault = خطأ;
  إذا (EC)
    أرجع خطأ;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  إذا (!Current_كلي) {
    إذا (Required)
      EC = make_error_code(errc::invalid_argument);
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_كلي);
  إذا (!MN) {
    إذا (Required || !isa<EmptyHNode>(Current_كلي))
      setError(Current_كلي, "not a mapping");
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }
  MN->Valid_مبوداي.تور_back(مبودي.str());
  HNode *درج = MN->Mapping[مبودي].first;
  إذا (!درج) {
    إذا (Required)
      setError(Current_كلي, Twine("missing required key '") + مبودي + "'");
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }
  SaveInfo = Current_كلي;
  Current_كلي = درج;
  أرجع true;
}

فراغ شغرو::postflight_مبودي(فراغ *saveInfo) {
  Current_كلي = reinterpret_cast<HNode *>(saveInfo);
}

فراغ شغرو::endMapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_كلي);
  إذا (!MN)
    أرجع;
  لكل (ثابت auto &NN : MN->Mapping) {
    إذا (!is_contained(MN->Valid_مبوداي, NN.first())) {
      ثابت SMRange &ReportLoc = NN.second.second;
      إذا (!Allow_Unknown_مبوداي) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        اكسر;
      } وإلا
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

فراغ شغرو::beginFlowMapping() { beginMapping(); }

فراغ شغرو::endFlowMapping() { endMapping(); }

unsigned شغرو::beginSequence() {
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_كلي))
    أرجع SQ->Entries.size();
  إذا (isa<EmptyHNode>(Current_كلي))
    أرجع 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_كلي)) {
    إذا (isNull(SN->درج_2()))
      أرجع 0;
  }
  // Any other type of HNode is an error.
  setError(Current_كلي, "not a sequence");
  أرجع 0;
}

فراغ شغرو::endSequence() {
}

منطقي شغرو::preflightElement(unsigned Index, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_كلي)) {
    SaveInfo = Current_كلي;
    Current_كلي = SQ->Entries[Index];
    أرجع true;
  }
  أرجع خطأ;
}

فراغ شغرو::postflightElement(فراغ *SaveInfo) {
  Current_كلي = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned شغرو::beginFlowSequence() { أرجع beginSequence(); }

منطقي شغرو::preflightFlowElement(unsigned index, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_كلي)) {
    SaveInfo = Current_كلي;
    Current_كلي = SQ->Entries[index];
    أرجع true;
  }
  أرجع خطأ;
}

فراغ شغرو::postflightFlowElement(فراغ *SaveInfo) {
  Current_كلي = reinterpret_cast<HNode *>(SaveInfo);
}

فراغ شغرو::endFlowSequence() {
}

فراغ شغرو::beginEnumScalar() {
  ScalarMatchFound = خطأ;
}

منطقي شغرو::matchEnumScalar(StringRef Str, منطقي) {
  إذا (ScalarMatchFound)
    أرجع خطأ;
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_كلي)) {
    إذا (SN->درج_2() == Str) {
      ScalarMatchFound = true;
      أرجع true;
    }
  }
  أرجع خطأ;
}

منطقي شغرو::matchEnumFallback() {
  إذا (ScalarMatchFound)
    أرجع خطأ;
  ScalarMatchFound = true;
  أرجع true;
}

فراغ شغرو::endEnumScalar() {
  إذا (!ScalarMatchFound) {
    setError(Current_كلي, "unknown enumerated scalar");
  }
}

منطقي شغرو::beginBitSetScalar(منطقي &DoClear) {
  Bit_درجوجي_Used.clear();
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_كلي)) {
    Bit_درجوجي_Used.resize(SQ->Entries.size());
  } وإلا {
    setError(Current_كلي, "expected sequence of bit values");
  }
  DoClear = true;
  أرجع true;
}

منطقي شغرو::bitSetMatch(StringRef Str, منطقي) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_كلي)) {
    unsigned Index = 0;
    لكل (auto &N : SQ->Entries) {
      إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        إذا (SN->درج_2() == Str) {
          Bit_درجوجي_Used[Index] = true;
          أرجع true;
        }
      } وإلا {
        setError(Current_كلي, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } وإلا {
    setError(Current_كلي, "expected sequence of bit values");
  }
  أرجع خطأ;
}

فراغ شغرو::endBitSetScalar() {
  إذا (EC)
    أرجع;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_كلي)) {
    أكد(Bit_درجوجي_Used.size() == SQ->Entries.size());
    لكل (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      إذا (!Bit_درجوجي_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        أرجع;
      }
    }
  }
}

فراغ شغرو::scalarString(StringRef &S, QuotingType) {
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_كلي)) {
    S = SN->درج_2();
  } وإلا {
    setError(Current_كلي, "unexpected scalar");
  }
}

فراغ شغرو::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

فراغ شغرو::scalarTag(std::string &Tag) {
  Tag = Current_كلي->كلي_2->getVerbatimTag();
}

فراغ شغرو::setError(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  setError(hnode->كلي_2, message);
}

كلي_Kind شغرو::get_كلي_Kind() {
  إذا (isa<ScalarHNode>(Current_كلي))
    أرجع كلي_Kind::Scalar;
  وإلا إذا (isa<MapHNode>(Current_كلي))
    أرجع كلي_Kind::Map;
  وإلا إذا (isa<SequenceHNode>(Current_كلي))
    أرجع كلي_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

فراغ شغرو::setError(كلي *كلي_3, ثابت Twine &message) {
  Strm->بغ_Error(كلي_3, message);
  EC = make_error_code(errc::invalid_argument);
}

فراغ شغرو::setError(ثابت SMRange &range, ثابت Twine &message) {
  Strm->بغ_Error(range, message);
  EC = make_error_code(errc::invalid_argument);
}

فراغ شغرو::reportWarning(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  Strm->بغ_Error(hnode->كلي_2, message, SourceMgr::DK_Warning);
}

فراغ شغرو::reportWarning(كلي *كلي_3, ثابت Twine &message) {
  Strm->بغ_Error(كلي_3, message, SourceMgr::DK_Warning);
}

فراغ شغرو::reportWarning(ثابت SMRange &range, ثابت Twine &message) {
  Strm->بغ_Error(range, message, SourceMgr::DK_Warning);
}

فراغ شغرو::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

فراغ شغرو::saveAliasHNode(كلي *N, HNode *HN) {
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

شغرو::HNode *شغرو::createHNodes(كلي *N) {
  SmallString<128> StringStorage;
  بدل (N->getType()) {
  حالة كلي::NK_Scalar: {
    Scalar_كلي *SN = dyn_cast<Scalar_كلي>(N);
    StringRef مبودي_Str = SN->get_درج(StringStorage);
    إذا (!StringStorage.empty()) {
      // Copy string to permanent storage
      مبودي_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, مبودي_Str);
    saveAliasHNode(SN, SHNode);
    أرجع SHNode;
  }
  حالة كلي::NK_BlockScalar: {
    Block_Scalar_كلي *BSN = dyn_cast<Block_Scalar_كلي>(N);
    StringRef درج_Copy = BSN->get_درج().copy(StringAllocator);
    auto *BSHNode =
        جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, درج_Copy);
    saveAliasHNode(BSN, BSHNode);
    أرجع BSHNode;
  }
  حالة كلي::NK_Sequence: {
    Sequence_كلي *SQ = dyn_cast<Sequence_كلي>(N);
    auto SQHNode = جديد (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    لكل (كلي &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      إذا (EC)
        اكسر;
      SQHNode->Entries.تور_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    أرجع SQHNode;
  }
  حالة كلي::NK_Mapping: {
    Mapping_كلي *Map = dyn_cast<Mapping_كلي>(N);
    auto mapHNode = جديد (MapHNodeAllocator.Allocate()) MapHNode(N);
    لكل (مبودي_درج_كلي &KVN : *Map) {
      كلي *مبودي_كلي = KVN.get_مبودي();
      Scalar_كلي *مبودي = dyn_cast_if_present<Scalar_كلي>(مبودي_كلي);
      كلي *درج = KVN.get_درج();
      إذا (!مبودي || !درج) {
        إذا (!مبودي)
          setError(مبودي_كلي, "Map key must be a scalar");
        إذا (!درج)
          setError(مبودي_كلي, "Map value must not be empty");
        اكسر;
      }
      StringStorage.clear();
      StringRef مبودي_Str = مبودي->get_درج(StringStorage);
      إذا (!StringStorage.empty()) {
        // Copy string to permanent storage
        مبودي_Str = StringStorage.str().copy(StringAllocator);
      }
      إذا (mapHNode->Mapping.كدي(مبودي_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(مبودي_كلي, Twine("duplicated mapping key '") + مبودي_Str + "'");
      auto درج_HNode = createHNodes(درج);
      إذا (EC)
        اكسر;
      mapHNode->Mapping[مبودي_Str] =
          std::make_pair(std::move(درج_HNode), مبودي_كلي->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    أرجع std::move(mapHNode);
  }
  حالة كلي::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    أرجع جديد (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  حالة كلي::NK_Alias: {
    Alias_كلي *AN = dyn_cast<Alias_كلي>(N);
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

فراغ شغرو::setError(ثابت Twine &Message) {
  setError(Current_كلي, Message);
}

فراغ شغرو::set_Allow_Unknown_مبوداي(منطقي Allow) { Allow_Unknown_مبوداي = Allow; }

منطقي شغرو::canElideEmptySequence() {
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
  State_تر.تور_back(in_Map_First_مبودي);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

منطقي Output::mapTag(StringRef Tag, منطقي Use) {
  إذا (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    منطقي SequenceElement = خطأ;
    إذا (State_تر.size() > 1) {
      auto &E = State_تر[State_تر.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    إذا (SequenceElement && State_تر.back() == in_Map_First_مبودي) {
      new_لي_Check();
    } وإلا {
      output(" ");
    }
    output(Tag);
    إذا (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      إذا (State_تر.back() == in_Map_First_مبودي) {
        State_تر.چرو_back();
        State_تر.تور_back(in_Map_Other_مبودي);
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
  إذا (State_تر.back() == in_Map_First_مبودي) {
    Padding = PaddingBeforeContainer;
    new_لي_Check();
    output("{}");
    Padding = "\n";
  }
  State_تر.چرو_back();
}

std::vector<StringRef> Output::مبوداي() {
  report_fatal_error("invalid call");
}

منطقي Output::preflight_مبودي(StringRef مبودي, منطقي Required, منطقي SameAsDefault,
                          منطقي &UseDefault, فراغ *&SaveInfo) {
  UseDefault = خطأ;
  SaveInfo = nullptr;
  إذا (Required || !SameAsDefault || Write_Default_درجوجي) {
    auto State = State_تر.back();
    إذا (State == in_Flow_Map_First_مبودي || State == in_Flow_Map_Other_مبودي) {
      flow_مبودي(مبودي);
    } وإلا {
      new_لي_Check();
      padded_مبودي(مبودي);
    }
    أرجع true;
  }
  أرجع خطأ;
}

فراغ Output::postflight_مبودي(فراغ *) {
  إذا (State_تر.back() == in_Map_First_مبودي) {
    State_تر.چرو_back();
    State_تر.تور_back(in_Map_Other_مبودي);
  } وإلا إذا (State_تر.back() == in_Flow_Map_First_مبودي) {
    State_تر.چرو_back();
    State_تر.تور_back(in_Flow_Map_Other_مبودي);
  }
}

فراغ Output::beginFlowMapping() {
  State_تر.تور_back(in_Flow_Map_First_مبودي);
  new_لي_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

فراغ Output::endFlowMapping() {
  State_تر.چرو_back();
  output_Up_To_End_Of_لي(" }");
}

فراغ Output::beginDocuments() {
  output_Up_To_End_Of_لي("---");
}

منطقي Output::preflightDocument(unsigned index) {
  إذا (index > 0)
    output_Up_To_End_Of_لي("\n---");
  أرجع true;
}

فراغ Output::postflightDocument() {
}

فراغ Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_تر.تور_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  أرجع 0;
}

فراغ Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  إذا (State_تر.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_لي_Check(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  State_تر.چرو_back();
}

منطقي Output::preflightElement(unsigned, فراغ *&SaveInfo) {
  SaveInfo = nullptr;
  أرجع true;
}

فراغ Output::postflightElement(فراغ *) {
  إذا (State_تر.back() == inSeqFirstElement) {
    State_تر.چرو_back();
    State_تر.تور_back(inSeqOtherElement);
  } وإلا إذا (State_تر.back() == inFlowSeqFirstElement) {
    State_تر.چرو_back();
    State_تر.تور_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_تر.تور_back(inFlowSeqFirstElement);
  new_لي_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = خطأ;
  أرجع 0;
}

فراغ Output::endFlowSequence() {
  State_تر.چرو_back();
  output_Up_To_End_Of_لي(" ]");
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
    new_لي_Check();
    output_Up_To_End_Of_لي(Str);
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
  new_لي_Check();
  output("[ ");
  Need_Bit_درج_Comma = خطأ;
  DoClear = خطأ;
  أرجع true;
}

منطقي Output::bitSetMatch(StringRef Str, منطقي Matches) {
  إذا (Matches) {
    إذا (Need_Bit_درج_Comma)
      output(", ");
    output(Str);
    Need_Bit_درج_Comma = true;
  }
  أرجع خطأ;
}

فراغ Output::endBitSetScalar() {
  output_Up_To_End_Of_لي(" ]");
}

فراغ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_لي_Check();
  إذا (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_لي("''");
    أرجع;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_لي("");
}

فراغ Output::blockScalarString(StringRef &S) {
  إذا (!State_تر.empty())
    new_لي_Check();
  output(" |");

  unsigned Indent = State_تر.empty() ? 1 : State_تر.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", خطأ);
  لكل (لي_iterator ليوك(*Buffer, خطأ); !ليوك.is_at_end(); ++ليوك) {
    output_New_لي();
    لكل (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*ليوك);
  }
  output_Up_To_End_Of_لي("");
}

فراغ Output::scalarTag(std::string &Tag) {
  إذا (Tag.empty())
    أرجع;
  new_لي_Check();
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
  إذا (State_تر.size() < 2)
    أرجع true;
  إذا (State_تر.back() != in_Map_First_مبودي)
    أرجع true;
  أرجع !inSeqAnyElement(State_تر[State_تر.size() - 2]);
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

فراغ Output::output_Up_To_End_Of_لي(StringRef s) {
  output(s);
  إذا (State_تر.empty() || (!inFlowSeqAnyElement(State_تر.back()) &&
                             !in_Flow_Map_Any_مبودي(State_تر.back())))
    Padding = "\n";
}

فراغ Output::output_New_لي() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

فراغ Output::new_لي_Check(منطقي EmptySequence) {
  إذا (Padding != "\n") {
    output(Padding);
    Padding = {};
    أرجع;
  }
  output_New_لي();
  Padding = {};

  إذا (State_تر.size() == 0 || EmptySequence)
    أرجع;

  unsigned Indent = State_تر.size() - 1;
  منطقي PossiblyNestedSeq = خطأ;
  auto I = State_تر.rbegin(), E = State_تر.rend();

  إذا (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } وإلا إذا (*I == in_Map_First_مبودي || *I == in_Flow_Map_First_مبودي ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned Output_Dash_كدي = 0;
  إذا (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    طالما (I != E) {
      // Don't count the top of nested sequence.
      إذا (!inSeqAnyElement(*I))
        اكسر;

      ++Output_Dash_كدي;

      // Stop counting if consecutive inSeqFirstElement ends.
      إذا (*I++ != inSeqFirstElement)
        اكسر;
    }
  }

  لكل (unsigned I = Output_Dash_كدي; I < Indent; ++I)
    output("  ");

  لكل (unsigned I = 0; I < Output_Dash_كدي; ++I)
    output("- ");
}

فراغ Output::padded_مبودي(StringRef مبودي_2) {
  output(مبودي_2, needsQuotes(مبودي_2, خطأ));
  output(":");
  ثابت حرف *spaces = "                ";
  إذا (مبودي_2.size() < strlen(spaces))
    Padding = &spaces[مبودي_2.size()];
  وإلا
    Padding = " ";
}

فراغ Output::flow_مبودي(StringRef مبودي) {
  إذا (State_تر.back() == in_Flow_Map_Other_مبودي)
    output(", ");
  إذا (WrapColumn && Column > WrapColumn) {
    output("\n");
    لكل (صحيح I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(مبودي, needsQuotes(مبودي, خطأ));
  output(": ");
}

كلي_Kind Output::get_كلي_Kind() { report_fatal_error("invalid call"); }

منطقي Output::inSeqAnyElement(InState State) {
  أرجع State == inSeqFirstElement || State == inSeqOtherElement;
}

منطقي Output::inFlowSeqAnyElement(InState State) {
  أرجع State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

منطقي Output::in_Map_Any_مبودي(InState State) {
  أرجع State == in_Map_First_مبودي || State == in_Map_Other_مبودي;
}

منطقي Output::in_Flow_Map_Any_مبودي(InState State) {
  أرجع State == in_Flow_Map_First_مبودي || State == in_Flow_Map_Other_مبودي;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

فراغ ScalarTraits<منطقي>::output(ثابت منطقي &Val, فراغ *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<منطقي>::شغرو_2(StringRef Scalar, فراغ *, منطقي &Val) {
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

StringRef ScalarTraits<StringRef>::شغرو_2(StringRef Scalar, فراغ *,
                                         StringRef &Val) {
  Val = Scalar;
  أرجع StringRef();
}

فراغ ScalarTraits<std::string>::output(ثابت std::string &Val, فراغ *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::شغرو_2(StringRef Scalar, فراغ *,
                                           std::string &Val) {
  Val = Scalar.str();
  أرجع StringRef();
}

فراغ ScalarTraits<uint8_t>::output(ثابت uint8_t &Val, فراغ *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t لمب = Val;
  Out << لمب;
}

StringRef ScalarTraits<uint8_t>::شغرو_2(StringRef Scalar, فراغ *, uint8_t &Val) {
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

StringRef ScalarTraits<uint16_t>::شغرو_2(StringRef Scalar, فراغ *,
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

StringRef ScalarTraits<uint32_t>::شغرو_2(StringRef Scalar, فراغ *,
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

StringRef ScalarTraits<uint64_t>::شغرو_2(StringRef Scalar, فراغ *,
                                        uint64_t &Val) {
  unsigned long long N;
  إذا (getAsUnsignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int8_t>::output(ثابت int8_t &Val, فراغ *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t لمب = Val;
  Out << لمب;
}

StringRef ScalarTraits<int8_t>::شغرو_2(StringRef Scalar, فراغ *, int8_t &Val) {
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

StringRef ScalarTraits<int16_t>::شغرو_2(StringRef Scalar, فراغ *, int16_t &Val) {
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

StringRef ScalarTraits<int32_t>::شغرو_2(StringRef Scalar, فراغ *, int32_t &Val) {
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

StringRef ScalarTraits<int64_t>::شغرو_2(StringRef Scalar, فراغ *, int64_t &Val) {
  long long N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<مزدوج>::output(ثابت مزدوج &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<مزدوج>::شغرو_2(StringRef Scalar, فراغ *, مزدوج &Val) {
  إذا (to_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<عائم>::output(ثابت عائم &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<عائم>::شغرو_2(StringRef Scalar, فراغ *, عائم &Val) {
  إذا (to_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<Hex8>::output(ثابت Hex8 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::شغرو_2(StringRef Scalar, فراغ *, Hex8 &Val) {
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

StringRef ScalarTraits<Hex16>::شغرو_2(StringRef Scalar, فراغ *, Hex16 &Val) {
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

StringRef ScalarTraits<Hex32>::شغرو_2(StringRef Scalar, فراغ *, Hex32 &Val) {
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

StringRef ScalarTraits<Hex64>::شغرو_2(StringRef Scalar, فراغ *, Hex64 &Val) {
  unsigned long long لمب;
  إذا (getAsUnsignedInteger(Scalar, 0, لمب))
    أرجع "invalid hex64 number";
  Val = لمب;
  أرجع StringRef();
}

فراغ ScalarTraits<VersionTuple>::output(ثابت VersionTuple &Val, فراغ *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::شغرو_2(StringRef Scalar, فراغ *,
                                            VersionTuple &Val) {
  إذا (Val.tryParse(Scalar))
    أرجع "invalid version format";
  أرجع StringRef();
}
