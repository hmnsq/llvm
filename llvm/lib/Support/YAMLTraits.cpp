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

فراغ IO::set_Allow_Unknown_کلیلەکان(منطقي Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//تێخستن
//===----------------------------------------------------------------------===//

تێخستن::تێخستن(StringRef تێخستن_Content, فراغ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد Stream(تێخستن_Content, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

تێخستن::تێخستن(MemoryBufferRef تێخستن, فراغ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد Stream(تێخستن, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

تێخستن::~تێخستن() = افتراضي;

std::error_code تێخستن::error() { أرجع EC; }

منطقي تێخستن::outputting() ثابت {
  أرجع خطأ;
}

منطقي تێخستن::setCurrentDocument() {
  إذا (DocIterator != Strm->end()) {
    گرێ *N = DocIterator->get_ڕەگ();
    إذا (!N) {
      EC = make_error_code(errc::invalid_argument);
      أرجع خطأ;
    }

    إذا (isa<Null_گرێ>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      أرجع setCurrentDocument();
    }
    releaseHNodeBuffers();
    لووتکە_گرێ = createHNodes(N);
    Current_گرێ = لووتکە_گرێ;
    أرجع true;
  }
  أرجع خطأ;
}

منطقي تێخستن::nextDocument() {
  أرجع ++DocIterator != Strm->end();
}

ثابت گرێ *تێخستن::get_Current_گرێ() ثابت {
  أرجع Current_گرێ ? Current_گرێ->گرێ_2 : nullptr;
}

منطقي تێخستن::mapTag(StringRef Tag, منطقي Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  إذا (!Current_گرێ)
    أرجع خطأ;

  std::string foundTag = Current_گرێ->گرێ_2->getVerbatimTag();
  إذا (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    أرجع Default;
  }
  // Return true iff found tag matches supplied tag.
  أرجع Tag == foundTag;
}

فراغ تێخستن::beginMapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_گرێ);
  إذا (MN) {
    MN->Valid_کلیلەکان.clear();
  }
}

std::vector<StringRef> تێخستن::کلیلەکان() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_گرێ);
  std::vector<StringRef> Ret;
  إذا (!MN) {
    setError(Current_گرێ, "not a mapping");
    أرجع Ret;
  }
  لكل (auto &P : MN->Mapping)
    Ret.پاڵنان_back(P.first());
  أرجع Ret;
}

منطقي تێخستن::preflight_کلیل(StringRef کلیل, منطقي Required, منطقي, منطقي &UseDefault,
                         فراغ *&SaveInfo) {
  UseDefault = خطأ;
  إذا (EC)
    أرجع خطأ;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  إذا (!Current_گرێ) {
    إذا (Required)
      EC = make_error_code(errc::invalid_argument);
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_گرێ);
  إذا (!MN) {
    إذا (Required || !isa<EmptyHNode>(Current_گرێ))
      setError(Current_گرێ, "not a mapping");
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }
  MN->Valid_کلیلەکان.پاڵنان_back(کلیل.str());
  HNode *بەها = MN->Mapping[کلیل].first;
  إذا (!بەها) {
    إذا (Required)
      setError(Current_گرێ, Twine("missing required key '") + کلیل + "'");
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }
  SaveInfo = Current_گرێ;
  Current_گرێ = بەها;
  أرجع true;
}

فراغ تێخستن::postflight_کلیل(فراغ *saveInfo) {
  Current_گرێ = reinterpret_cast<HNode *>(saveInfo);
}

فراغ تێخستن::endMapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_گرێ);
  إذا (!MN)
    أرجع;
  لكل (ثابت auto &NN : MN->Mapping) {
    إذا (!is_contained(MN->Valid_کلیلەکان, NN.first())) {
      ثابت SMRange &ReportLoc = NN.second.second;
      إذا (!Allow_Unknown_کلیلەکان) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        اكسر;
      } وإلا
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

فراغ تێخستن::beginFlowMapping() { beginMapping(); }

فراغ تێخستن::endFlowMapping() { endMapping(); }

unsigned تێخستن::beginSequence() {
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرێ))
    أرجع SQ->Entries.size();
  إذا (isa<EmptyHNode>(Current_گرێ))
    أرجع 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گرێ)) {
    إذا (isNull(SN->بەها_2()))
      أرجع 0;
  }
  // Any other type of HNode is an error.
  setError(Current_گرێ, "not a sequence");
  أرجع 0;
}

فراغ تێخستن::endSequence() {
}

منطقي تێخستن::preflightElement(unsigned Index, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرێ)) {
    SaveInfo = Current_گرێ;
    Current_گرێ = SQ->Entries[Index];
    أرجع true;
  }
  أرجع خطأ;
}

فراغ تێخستن::postflightElement(فراغ *SaveInfo) {
  Current_گرێ = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned تێخستن::beginFlowSequence() { أرجع beginSequence(); }

منطقي تێخستن::preflightFlowElement(unsigned index, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرێ)) {
    SaveInfo = Current_گرێ;
    Current_گرێ = SQ->Entries[index];
    أرجع true;
  }
  أرجع خطأ;
}

فراغ تێخستن::postflightFlowElement(فراغ *SaveInfo) {
  Current_گرێ = reinterpret_cast<HNode *>(SaveInfo);
}

فراغ تێخستن::endFlowSequence() {
}

فراغ تێخستن::beginEnumScalar() {
  ScalarMatchFound = خطأ;
}

منطقي تێخستن::matchEnumScalar(StringRef Str, منطقي) {
  إذا (ScalarMatchFound)
    أرجع خطأ;
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گرێ)) {
    إذا (SN->بەها_2() == Str) {
      ScalarMatchFound = true;
      أرجع true;
    }
  }
  أرجع خطأ;
}

منطقي تێخستن::matchEnumFallback() {
  إذا (ScalarMatchFound)
    أرجع خطأ;
  ScalarMatchFound = true;
  أرجع true;
}

فراغ تێخستن::endEnumScalar() {
  إذا (!ScalarMatchFound) {
    setError(Current_گرێ, "unknown enumerated scalar");
  }
}

منطقي تێخستن::beginBitSetScalar(منطقي &DoClear) {
  Bit_بەهاکان_Used.clear();
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرێ)) {
    Bit_بەهاکان_Used.resize(SQ->Entries.size());
  } وإلا {
    setError(Current_گرێ, "expected sequence of bit values");
  }
  DoClear = true;
  أرجع true;
}

منطقي تێخستن::bitSetMatch(StringRef Str, منطقي) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرێ)) {
    unsigned Index = 0;
    لكل (auto &N : SQ->Entries) {
      إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        إذا (SN->بەها_2() == Str) {
          Bit_بەهاکان_Used[Index] = true;
          أرجع true;
        }
      } وإلا {
        setError(Current_گرێ, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } وإلا {
    setError(Current_گرێ, "expected sequence of bit values");
  }
  أرجع خطأ;
}

فراغ تێخستن::endBitSetScalar() {
  إذا (EC)
    أرجع;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرێ)) {
    أكد(Bit_بەهاکان_Used.size() == SQ->Entries.size());
    لكل (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      إذا (!Bit_بەهاکان_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        أرجع;
      }
    }
  }
}

فراغ تێخستن::scalarString(StringRef &S, QuotingType) {
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گرێ)) {
    S = SN->بەها_2();
  } وإلا {
    setError(Current_گرێ, "unexpected scalar");
  }
}

فراغ تێخستن::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

فراغ تێخستن::scalarTag(std::string &Tag) {
  Tag = Current_گرێ->گرێ_2->getVerbatimTag();
}

فراغ تێخستن::setError(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  setError(hnode->گرێ_2, message);
}

گرێ_Kind تێخستن::get_گرێ_Kind() {
  إذا (isa<ScalarHNode>(Current_گرێ))
    أرجع گرێ_Kind::Scalar;
  وإلا إذا (isa<MapHNode>(Current_گرێ))
    أرجع گرێ_Kind::Map;
  وإلا إذا (isa<SequenceHNode>(Current_گرێ))
    أرجع گرێ_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

فراغ تێخستن::setError(گرێ *گرێ_3, ثابت Twine &message) {
  Strm->printError(گرێ_3, message);
  EC = make_error_code(errc::invalid_argument);
}

فراغ تێخستن::setError(ثابت SMRange &range, ثابت Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

فراغ تێخستن::reportWarning(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  Strm->printError(hnode->گرێ_2, message, SourceMgr::DK_Warning);
}

فراغ تێخستن::reportWarning(گرێ *گرێ_3, ثابت Twine &message) {
  Strm->printError(گرێ_3, message, SourceMgr::DK_Warning);
}

فراغ تێخستن::reportWarning(ثابت SMRange &range, ثابت Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

فراغ تێخستن::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

فراغ تێخستن::saveAliasHNode(گرێ *N, HNode *HN) {
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

تێخستن::HNode *تێخستن::createHNodes(گرێ *N) {
  SmallString<128> StringStorage;
  بدل (N->getType()) {
  حالة گرێ::NK_Scalar: {
    Scalar_گرێ *SN = dyn_cast<Scalar_گرێ>(N);
    StringRef کلیل_Str = SN->get_بەها(StringStorage);
    إذا (!StringStorage.empty()) {
      // Copy string to permanent storage
      کلیل_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, کلیل_Str);
    saveAliasHNode(SN, SHNode);
    أرجع SHNode;
  }
  حالة گرێ::NK_BlockScalar: {
    Block_Scalar_گرێ *BSN = dyn_cast<Block_Scalar_گرێ>(N);
    StringRef بەها_Copy = BSN->get_بەها().copy(StringAllocator);
    auto *BSHNode =
        جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, بەها_Copy);
    saveAliasHNode(BSN, BSHNode);
    أرجع BSHNode;
  }
  حالة گرێ::NK_Sequence: {
    Sequence_گرێ *SQ = dyn_cast<Sequence_گرێ>(N);
    auto SQHNode = جديد (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    لكل (گرێ &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      إذا (EC)
        اكسر;
      SQHNode->Entries.پاڵنان_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    أرجع SQHNode;
  }
  حالة گرێ::NK_Mapping: {
    Mapping_گرێ *Map = dyn_cast<Mapping_گرێ>(N);
    auto mapHNode = جديد (MapHNodeAllocator.Allocate()) MapHNode(N);
    لكل (کلیل_بەها_گرێ &KVN : *Map) {
      گرێ *کلیل_گرێ = KVN.get_کلیل();
      Scalar_گرێ *کلیل = dyn_cast_if_present<Scalar_گرێ>(کلیل_گرێ);
      گرێ *بەها = KVN.get_بەها();
      إذا (!کلیل || !بەها) {
        إذا (!کلیل)
          setError(کلیل_گرێ, "Map key must be a scalar");
        إذا (!بەها)
          setError(کلیل_گرێ, "Map value must not be empty");
        اكسر;
      }
      StringStorage.clear();
      StringRef کلیل_Str = کلیل->get_بەها(StringStorage);
      إذا (!StringStorage.empty()) {
        // Copy string to permanent storage
        کلیل_Str = StringStorage.str().copy(StringAllocator);
      }
      إذا (mapHNode->Mapping.ژماردن(کلیل_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(کلیل_گرێ, Twine("duplicated mapping key '") + کلیل_Str + "'");
      auto بەها_HNode = createHNodes(بەها);
      إذا (EC)
        اكسر;
      mapHNode->Mapping[کلیل_Str] =
          std::make_pair(std::move(بەها_HNode), کلیل_گرێ->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    أرجع std::move(mapHNode);
  }
  حالة گرێ::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    أرجع جديد (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  حالة گرێ::NK_Alias: {
    Alias_گرێ *AN = dyn_cast<Alias_گرێ>(N);
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

فراغ تێخستن::setError(ثابت Twine &Message) {
  setError(Current_گرێ, Message);
}

فراغ تێخستن::set_Allow_Unknown_کلیلەکان(منطقي Allow) { Allow_Unknown_کلیلەکان = Allow; }

منطقي تێخستن::canElideEmptySequence() {
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
  State_کەڵەکە.پاڵنان_back(in_Map_First_کلیل);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

منطقي Output::mapTag(StringRef Tag, منطقي Use) {
  إذا (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    منطقي SequenceElement = خطأ;
    إذا (State_کەڵەکە.size() > 1) {
      auto &E = State_کەڵەکە[State_کەڵەکە.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    إذا (SequenceElement && State_کەڵەکە.back() == in_Map_First_کلیل) {
      new_هێڵ_Check();
    } وإلا {
      output(" ");
    }
    output(Tag);
    إذا (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      إذا (State_کەڵەکە.back() == in_Map_First_کلیل) {
        State_کەڵەکە.دەرهێنان_back();
        State_کەڵەکە.پاڵنان_back(in_Map_Other_کلیل);
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
  إذا (State_کەڵەکە.back() == in_Map_First_کلیل) {
    Padding = PaddingBeforeContainer;
    new_هێڵ_Check();
    output("{}");
    Padding = "\n";
  }
  State_کەڵەکە.دەرهێنان_back();
}

std::vector<StringRef> Output::کلیلەکان() {
  report_fatal_error("invalid call");
}

منطقي Output::preflight_کلیل(StringRef کلیل, منطقي Required, منطقي SameAsDefault,
                          منطقي &UseDefault, فراغ *&SaveInfo) {
  UseDefault = خطأ;
  SaveInfo = nullptr;
  إذا (Required || !SameAsDefault || Write_Default_بەهاکان) {
    auto State = State_کەڵەکە.back();
    إذا (State == in_Flow_Map_First_کلیل || State == in_Flow_Map_Other_کلیل) {
      flow_کلیل(کلیل);
    } وإلا {
      new_هێڵ_Check();
      padded_کلیل(کلیل);
    }
    أرجع true;
  }
  أرجع خطأ;
}

فراغ Output::postflight_کلیل(فراغ *) {
  إذا (State_کەڵەکە.back() == in_Map_First_کلیل) {
    State_کەڵەکە.دەرهێنان_back();
    State_کەڵەکە.پاڵنان_back(in_Map_Other_کلیل);
  } وإلا إذا (State_کەڵەکە.back() == in_Flow_Map_First_کلیل) {
    State_کەڵەکە.دەرهێنان_back();
    State_کەڵەکە.پاڵنان_back(in_Flow_Map_Other_کلیل);
  }
}

فراغ Output::beginFlowMapping() {
  State_کەڵەکە.پاڵنان_back(in_Flow_Map_First_کلیل);
  new_هێڵ_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

فراغ Output::endFlowMapping() {
  State_کەڵەکە.دەرهێنان_back();
  output_Up_To_End_Of_هێڵ(" }");
}

فراغ Output::beginDocuments() {
  output_Up_To_End_Of_هێڵ("---");
}

منطقي Output::preflightDocument(unsigned index) {
  إذا (index > 0)
    output_Up_To_End_Of_هێڵ("\n---");
  أرجع true;
}

فراغ Output::postflightDocument() {
}

فراغ Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_کەڵەکە.پاڵنان_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  أرجع 0;
}

فراغ Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  إذا (State_کەڵەکە.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_هێڵ_Check(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  State_کەڵەکە.دەرهێنان_back();
}

منطقي Output::preflightElement(unsigned, فراغ *&SaveInfo) {
  SaveInfo = nullptr;
  أرجع true;
}

فراغ Output::postflightElement(فراغ *) {
  إذا (State_کەڵەکە.back() == inSeqFirstElement) {
    State_کەڵەکە.دەرهێنان_back();
    State_کەڵەکە.پاڵنان_back(inSeqOtherElement);
  } وإلا إذا (State_کەڵەکە.back() == inFlowSeqFirstElement) {
    State_کەڵەکە.دەرهێنان_back();
    State_کەڵەکە.پاڵنان_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_کەڵەکە.پاڵنان_back(inFlowSeqFirstElement);
  new_هێڵ_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = خطأ;
  أرجع 0;
}

فراغ Output::endFlowSequence() {
  State_کەڵەکە.دەرهێنان_back();
  output_Up_To_End_Of_هێڵ(" ]");
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
    new_هێڵ_Check();
    output_Up_To_End_Of_هێڵ(Str);
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
  new_هێڵ_Check();
  output("[ ");
  Need_Bit_بەها_Comma = خطأ;
  DoClear = خطأ;
  أرجع true;
}

منطقي Output::bitSetMatch(StringRef Str, منطقي Matches) {
  إذا (Matches) {
    إذا (Need_Bit_بەها_Comma)
      output(", ");
    output(Str);
    Need_Bit_بەها_Comma = true;
  }
  أرجع خطأ;
}

فراغ Output::endBitSetScalar() {
  output_Up_To_End_Of_هێڵ(" ]");
}

فراغ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_هێڵ_Check();
  إذا (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_هێڵ("''");
    أرجع;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_هێڵ("");
}

فراغ Output::blockScalarString(StringRef &S) {
  إذا (!State_کەڵەکە.empty())
    new_هێڵ_Check();
  output(" |");

  unsigned Indent = State_کەڵەکە.empty() ? 1 : State_کەڵەکە.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", خطأ);
  لكل (هێڵ_iterator هێڵەکان(*Buffer, خطأ); !هێڵەکان.is_at_end(); ++هێڵەکان) {
    output_New_هێڵ();
    لكل (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*هێڵەکان);
  }
  output_Up_To_End_Of_هێڵ("");
}

فراغ Output::scalarTag(std::string &Tag) {
  إذا (Tag.empty())
    أرجع;
  new_هێڵ_Check();
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
  إذا (State_کەڵەکە.size() < 2)
    أرجع true;
  إذا (State_کەڵەکە.back() != in_Map_First_کلیل)
    أرجع true;
  أرجع !inSeqAnyElement(State_کەڵەکە[State_کەڵەکە.size() - 2]);
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

فراغ Output::output_Up_To_End_Of_هێڵ(StringRef s) {
  output(s);
  إذا (State_کەڵەکە.empty() || (!inFlowSeqAnyElement(State_کەڵەکە.back()) &&
                             !in_Flow_Map_Any_کلیل(State_کەڵەکە.back())))
    Padding = "\n";
}

فراغ Output::output_New_هێڵ() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

فراغ Output::new_هێڵ_Check(منطقي EmptySequence) {
  إذا (Padding != "\n") {
    output(Padding);
    Padding = {};
    أرجع;
  }
  output_New_هێڵ();
  Padding = {};

  إذا (State_کەڵەکە.size() == 0 || EmptySequence)
    أرجع;

  unsigned Indent = State_کەڵەکە.size() - 1;
  منطقي PossiblyNestedSeq = خطأ;
  auto I = State_کەڵەکە.rbegin(), E = State_کەڵەکە.rend();

  إذا (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } وإلا إذا (*I == in_Map_First_کلیل || *I == in_Flow_Map_First_کلیل ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned Output_Dash_ژماردن = 0;
  إذا (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    طالما (I != E) {
      // Don't count the top of nested sequence.
      إذا (!inSeqAnyElement(*I))
        اكسر;

      ++Output_Dash_ژماردن;

      // Stop counting if consecutive inSeqFirstElement ends.
      إذا (*I++ != inSeqFirstElement)
        اكسر;
    }
  }

  لكل (unsigned I = Output_Dash_ژماردن; I < Indent; ++I)
    output("  ");

  لكل (unsigned I = 0; I < Output_Dash_ژماردن; ++I)
    output("- ");
}

فراغ Output::padded_کلیل(StringRef کلیل_2) {
  output(کلیل_2, needsQuotes(کلیل_2, خطأ));
  output(":");
  ثابت حرف *spaces = "                ";
  إذا (کلیل_2.size() < strlen(spaces))
    Padding = &spaces[کلیل_2.size()];
  وإلا
    Padding = " ";
}

فراغ Output::flow_کلیل(StringRef کلیل) {
  إذا (State_کەڵەکە.back() == in_Flow_Map_Other_کلیل)
    output(", ");
  إذا (WrapColumn && Column > WrapColumn) {
    output("\n");
    لكل (صحيح I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(کلیل, needsQuotes(کلیل, خطأ));
  output(": ");
}

گرێ_Kind Output::get_گرێ_Kind() { report_fatal_error("invalid call"); }

منطقي Output::inSeqAnyElement(InState State) {
  أرجع State == inSeqFirstElement || State == inSeqOtherElement;
}

منطقي Output::inFlowSeqAnyElement(InState State) {
  أرجع State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

منطقي Output::in_Map_Any_کلیل(InState State) {
  أرجع State == in_Map_First_کلیل || State == in_Map_Other_کلیل;
}

منطقي Output::in_Flow_Map_Any_کلیل(InState State) {
  أرجع State == in_Flow_Map_First_کلیل || State == in_Flow_Map_Other_کلیل;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

فراغ ScalarTraits<منطقي>::output(ثابت منطقي &Val, فراغ *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<منطقي>::تێخستن_2(StringRef Scalar, فراغ *, منطقي &Val) {
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

StringRef ScalarTraits<StringRef>::تێخستن_2(StringRef Scalar, فراغ *,
                                         StringRef &Val) {
  Val = Scalar;
  أرجع StringRef();
}

فراغ ScalarTraits<std::string>::output(ثابت std::string &Val, فراغ *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::تێخستن_2(StringRef Scalar, فراغ *,
                                           std::string &Val) {
  Val = Scalar.str();
  أرجع StringRef();
}

فراغ ScalarTraits<uint8_t>::output(ثابت uint8_t &Val, فراغ *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t ژمارە = Val;
  Out << ژمارە;
}

StringRef ScalarTraits<uint8_t>::تێخستن_2(StringRef Scalar, فراغ *, uint8_t &Val) {
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

StringRef ScalarTraits<uint16_t>::تێخستن_2(StringRef Scalar, فراغ *,
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

StringRef ScalarTraits<uint32_t>::تێخستن_2(StringRef Scalar, فراغ *,
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

StringRef ScalarTraits<uint64_t>::تێخستن_2(StringRef Scalar, فراغ *,
                                        uint64_t &Val) {
  unsigned long long N;
  إذا (getAsUnsignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int8_t>::output(ثابت int8_t &Val, فراغ *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t ژمارە = Val;
  Out << ژمارە;
}

StringRef ScalarTraits<int8_t>::تێخستن_2(StringRef Scalar, فراغ *, int8_t &Val) {
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

StringRef ScalarTraits<int16_t>::تێخستن_2(StringRef Scalar, فراغ *, int16_t &Val) {
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

StringRef ScalarTraits<int32_t>::تێخستن_2(StringRef Scalar, فراغ *, int32_t &Val) {
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

StringRef ScalarTraits<int64_t>::تێخستن_2(StringRef Scalar, فراغ *, int64_t &Val) {
  long long N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<مزدوج>::output(ثابت مزدوج &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<مزدوج>::تێخستن_2(StringRef Scalar, فراغ *, مزدوج &Val) {
  إذا (to_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<عائم>::output(ثابت عائم &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<عائم>::تێخستن_2(StringRef Scalar, فراغ *, عائم &Val) {
  إذا (to_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<Hex8>::output(ثابت Hex8 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::تێخستن_2(StringRef Scalar, فراغ *, Hex8 &Val) {
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

StringRef ScalarTraits<Hex16>::تێخستن_2(StringRef Scalar, فراغ *, Hex16 &Val) {
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

StringRef ScalarTraits<Hex32>::تێخستن_2(StringRef Scalar, فراغ *, Hex32 &Val) {
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

StringRef ScalarTraits<Hex64>::تێخستن_2(StringRef Scalar, فراغ *, Hex64 &Val) {
  unsigned long long ژمارە;
  إذا (getAsUnsignedInteger(Scalar, 0, ژمارە))
    أرجع "invalid hex64 number";
  Val = ژمارە;
  أرجع StringRef();
}

فراغ ScalarTraits<VersionTuple>::output(ثابت VersionTuple &Val, فراغ *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::تێخستن_2(StringRef Scalar, فراغ *,
                                            VersionTuple &Val) {
  إذا (Val.tryParse(Scalar))
    أرجع "invalid version format";
  أرجع StringRef();
}
