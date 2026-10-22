//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/YAMLTraits.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/LineIterator.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/VersionTuple.h"
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

using namespace llvm;
using namespace yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(void *Context) : Ctxt(Context) {}

IO::~IO() = default;

void *IO::getContext() const {
  return Ctxt;
}

void IO::setContext(void *Context) {
  Ctxt = Context;
}

void IO::set_Allow_Unknown_मगा_सुसि(bool Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//पासोक
//===----------------------------------------------------------------------===//

पासोक::पासोक(StringRef पासोक_Content, void *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, void *DiagHandlerCtxt)
    : IO(Ctxt), Strm(new Stream(पासोक_Content, SrcMgr, false, &EC)) {
  if (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

पासोक::पासोक(MemoryBufferRef पासोक, void *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, void *DiagHandlerCtxt)
    : IO(Ctxt), Strm(new Stream(पासोक, SrcMgr, false, &EC)) {
  if (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

पासोक::~पासोक() = default;

std::error_code पासोक::error() { return EC; }

bool पासोक::outputting() const {
  return false;
}

bool पासोक::setCurrentDocument() {
  if (DocIterator != Strm->end()) {
    बुहोल *N = DocIterator->get_उगात();
    if (!N) {
      EC = make_error_code(errc::invalid_argument);
      return false;
    }

    if (isa<Null_बुहोल>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      return setCurrentDocument();
    }
    releaseHNodeBuffers();
    इताअस_बुहोल = createHNodes(N);
    Current_बुहोल = इताअस_बुहोल;
    return true;
  }
  return false;
}

bool पासोक::nextDocument() {
  return ++DocIterator != Strm->end();
}

const बुहोल *पासोक::get_Current_बुहोल() const {
  return Current_बुहोल ? Current_बुहोल->बुहोल_2 : nullptr;
}

bool पासोक::mapTag(StringRef Tag, bool Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  if (!Current_बुहोल)
    return false;

  std::string foundTag = Current_बुहोल->बुहोल_2->getVerbatimTag();
  if (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    return Default;
  }
  // Return true iff found tag matches supplied tag.
  return Tag == foundTag;
}

void पासोक::beginMapping() {
  if (EC)
    return;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_बुहोल);
  if (MN) {
    MN->Valid_मगा_सुसि.clear();
  }
}

std::vector<StringRef> पासोक::मगा_सुसि() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_बुहोल);
  std::vector<StringRef> Ret;
  if (!MN) {
    setError(Current_बुहोल, "not a mapping");
    return Ret;
  }
  for (auto &P : MN->Mapping)
    Ret.इतुलाक_back(P.first());
  return Ret;
}

bool पासोक::preflight_सुसि(StringRef सुसि, bool Required, bool, bool &UseDefault,
                         void *&SaveInfo) {
  UseDefault = false;
  if (EC)
    return false;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  if (!Current_बुहोल) {
    if (Required)
      EC = make_error_code(errc::invalid_argument);
    else
      UseDefault = true;
    return false;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_बुहोल);
  if (!MN) {
    if (Required || !isa<EmptyHNode>(Current_बुहोल))
      setError(Current_बुहोल, "not a mapping");
    else
      UseDefault = true;
    return false;
  }
  MN->Valid_मगा_सुसि.इतुलाक_back(सुसि.str());
  HNode *हालागा = MN->Mapping[सुसि].first;
  if (!हालागा) {
    if (Required)
      setError(Current_बुहोल, Twine("missing required key '") + सुसि + "'");
    else
      UseDefault = true;
    return false;
  }
  SaveInfo = Current_बुहोल;
  Current_बुहोल = हालागा;
  return true;
}

void पासोक::postflight_सुसि(void *saveInfo) {
  Current_बुहोल = reinterpret_cast<HNode *>(saveInfo);
}

void पासोक::endMapping() {
  if (EC)
    return;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_बुहोल);
  if (!MN)
    return;
  for (const auto &NN : MN->Mapping) {
    if (!is_contained(MN->Valid_मगा_सुसि, NN.first())) {
      const SMRange &ReportLoc = NN.second.second;
      if (!Allow_Unknown_मगा_सुसि) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        break;
      } else
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

void पासोक::beginFlowMapping() { beginMapping(); }

void पासोक::endFlowMapping() { endMapping(); }

unsigned पासोक::beginSequence() {
  if (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_बुहोल))
    return SQ->Entries.size();
  if (isa<EmptyHNode>(Current_बुहोल))
    return 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  if (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_बुहोल)) {
    if (isNull(SN->हालागा_2()))
      return 0;
  }
  // Any other type of HNode is an error.
  setError(Current_बुहोल, "not a sequence");
  return 0;
}

void पासोक::endSequence() {
}

bool पासोक::preflightElement(unsigned Index, void *&SaveInfo) {
  if (EC)
    return false;
  if (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_बुहोल)) {
    SaveInfo = Current_बुहोल;
    Current_बुहोल = SQ->Entries[Index];
    return true;
  }
  return false;
}

void पासोक::postflightElement(void *SaveInfo) {
  Current_बुहोल = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned पासोक::beginFlowSequence() { return beginSequence(); }

bool पासोक::preflightFlowElement(unsigned index, void *&SaveInfo) {
  if (EC)
    return false;
  if (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_बुहोल)) {
    SaveInfo = Current_बुहोल;
    Current_बुहोल = SQ->Entries[index];
    return true;
  }
  return false;
}

void पासोक::postflightFlowElement(void *SaveInfo) {
  Current_बुहोल = reinterpret_cast<HNode *>(SaveInfo);
}

void पासोक::endFlowSequence() {
}

void पासोक::beginEnumScalar() {
  ScalarMatchFound = false;
}

bool पासोक::matchEnumScalar(StringRef Str, bool) {
  if (ScalarMatchFound)
    return false;
  if (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_बुहोल)) {
    if (SN->हालागा_2() == Str) {
      ScalarMatchFound = true;
      return true;
    }
  }
  return false;
}

bool पासोक::matchEnumFallback() {
  if (ScalarMatchFound)
    return false;
  ScalarMatchFound = true;
  return true;
}

void पासोक::endEnumScalar() {
  if (!ScalarMatchFound) {
    setError(Current_बुहोल, "unknown enumerated scalar");
  }
}

bool पासोक::beginBitSetScalar(bool &DoClear) {
  Bit_मगा_हालागा_Used.clear();
  if (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_बुहोल)) {
    Bit_मगा_हालागा_Used.resize(SQ->Entries.size());
  } else {
    setError(Current_बुहोल, "expected sequence of bit values");
  }
  DoClear = true;
  return true;
}

bool पासोक::bitSetMatch(StringRef Str, bool) {
  if (EC)
    return false;
  if (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_बुहोल)) {
    unsigned Index = 0;
    for (auto &N : SQ->Entries) {
      if (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        if (SN->हालागा_2() == Str) {
          Bit_मगा_हालागा_Used[Index] = true;
          return true;
        }
      } else {
        setError(Current_बुहोल, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } else {
    setError(Current_बुहोल, "expected sequence of bit values");
  }
  return false;
}

void पासोक::endBitSetScalar() {
  if (EC)
    return;
  if (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_बुहोल)) {
    assert(Bit_मगा_हालागा_Used.size() == SQ->Entries.size());
    for (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      if (!Bit_मगा_हालागा_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        return;
      }
    }
  }
}

void पासोक::scalarString(StringRef &S, QuotingType) {
  if (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_बुहोल)) {
    S = SN->हालागा_2();
  } else {
    setError(Current_बुहोल, "unexpected scalar");
  }
}

void पासोक::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

void पासोक::scalarTag(std::string &Tag) {
  Tag = Current_बुहोल->बुहोल_2->getVerbatimTag();
}

void पासोक::setError(HNode *hnode, const Twine &message) {
  assert(hnode && "HNode must not be NULL");
  setError(hnode->बुहोल_2, message);
}

बुहोल_Kind पासोक::get_बुहोल_Kind() {
  if (isa<ScalarHNode>(Current_बुहोल))
    return बुहोल_Kind::Scalar;
  else if (isa<MapHNode>(Current_बुहोल))
    return बुहोल_Kind::Map;
  else if (isa<SequenceHNode>(Current_बुहोल))
    return बुहोल_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

void पासोक::setError(बुहोल *बुहोल_3, const Twine &message) {
  Strm->इलिम्बाग_Error(बुहोल_3, message);
  EC = make_error_code(errc::invalid_argument);
}

void पासोक::setError(const SMRange &range, const Twine &message) {
  Strm->इलिम्बाग_Error(range, message);
  EC = make_error_code(errc::invalid_argument);
}

void पासोक::reportWarning(HNode *hnode, const Twine &message) {
  assert(hnode && "HNode must not be NULL");
  Strm->इलिम्बाग_Error(hnode->बुहोल_2, message, SourceMgr::DK_Warning);
}

void पासोक::reportWarning(बुहोल *बुहोल_3, const Twine &message) {
  Strm->इलिम्बाग_Error(बुहोल_3, message, SourceMgr::DK_Warning);
}

void पासोक::reportWarning(const SMRange &range, const Twine &message) {
  Strm->इलिम्बाग_Error(range, message, SourceMgr::DK_Warning);
}

void पासोक::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

void पासोक::saveAliasHNode(बुहोल *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  if (!Anchor.empty())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

पासोक::HNode *पासोक::createHNodes(बुहोल *N) {
  SmallString<128> StringStorage;
  switch (N->getType()) {
  case बुहोल::NK_Scalar: {
    Scalar_बुहोल *SN = dyn_cast<Scalar_बुहोल>(N);
    StringRef सुसि_Str = SN->get_हालागा(StringStorage);
    if (!StringStorage.empty()) {
      // Copy string to permanent storage
      सुसि_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, सुसि_Str);
    saveAliasHNode(SN, SHNode);
    return SHNode;
  }
  case बुहोल::NK_BlockScalar: {
    Block_Scalar_बुहोल *BSN = dyn_cast<Block_Scalar_बुहोल>(N);
    StringRef हालागा_Copy = BSN->get_हालागा().copy(StringAllocator);
    auto *BSHNode =
        new (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, हालागा_Copy);
    saveAliasHNode(BSN, BSHNode);
    return BSHNode;
  }
  case बुहोल::NK_Sequence: {
    Sequence_बुहोल *SQ = dyn_cast<Sequence_बुहोल>(N);
    auto SQHNode = new (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    for (बुहोल &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      if (EC)
        break;
      SQHNode->Entries.इतुलाक_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    return SQHNode;
  }
  case बुहोल::NK_Mapping: {
    Mapping_बुहोल *Map = dyn_cast<Mapping_बुहोल>(N);
    auto mapHNode = new (MapHNodeAllocator.Allocate()) MapHNode(N);
    for (सुसि_हालागा_बुहोल &KVN : *Map) {
      बुहोल *सुसि_बुहोल = KVN.get_सुसि();
      Scalar_बुहोल *सुसि = dyn_cast_if_present<Scalar_बुहोल>(सुसि_बुहोल);
      बुहोल *हालागा = KVN.get_हालागा();
      if (!सुसि || !हालागा) {
        if (!सुसि)
          setError(सुसि_बुहोल, "Map key must be a scalar");
        if (!हालागा)
          setError(सुसि_बुहोल, "Map value must not be empty");
        break;
      }
      StringStorage.clear();
      StringRef सुसि_Str = सुसि->get_हालागा(StringStorage);
      if (!StringStorage.empty()) {
        // Copy string to permanent storage
        सुसि_Str = StringStorage.str().copy(StringAllocator);
      }
      if (mapHNode->Mapping.बिलांग(सुसि_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(सुसि_बुहोल, Twine("duplicated mapping key '") + सुसि_Str + "'");
      auto हालागा_HNode = createHNodes(हालागा);
      if (EC)
        break;
      mapHNode->Mapping[सुसि_Str] =
          std::make_pair(std::move(हालागा_HNode), सुसि_बुहोल->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    return std::move(mapHNode);
  }
  case बुहोल::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    return new (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  case बुहोल::NK_Alias: {
    Alias_बुहोल *AN = dyn_cast<Alias_बुहोल>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    if (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      return nullptr;
    }
    return AHN->second;
  }
  default:
    setError(N, "unknown node kind");
    return nullptr;
  }
}

void पासोक::setError(const Twine &Message) {
  setError(Current_बुहोल, Message);
}

void पासोक::set_Allow_Unknown_मगा_सुसि(bool Allow) { Allow_Unknown_मगा_सुसि = Allow; }

bool पासोक::canElideEmptySequence() {
  return false;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, void *context, int WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = default;

bool Output::outputting() const {
  return true;
}

void Output::beginMapping() {
  State_सालानसान.इतुलाक_back(in_Map_First_सुसि);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

bool Output::mapTag(StringRef Tag, bool Use) {
  if (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    bool SequenceElement = false;
    if (State_सालानसान.size() > 1) {
      auto &E = State_सालानसान[State_सालानसान.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    if (SequenceElement && State_सालानसान.back() == in_Map_First_सुसि) {
      new_लिन्या_Check();
    } else {
      output(" ");
    }
    output(Tag);
    if (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      if (State_सालानसान.back() == in_Map_First_सुसि) {
        State_सालानसान.आलिसिन_back();
        State_सालानसान.इतुलाक_back(in_Map_Other_सुसि);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  return Use;
}

void Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  if (State_सालानसान.back() == in_Map_First_सुसि) {
    Padding = PaddingBeforeContainer;
    new_लिन्या_Check();
    output("{}");
    Padding = "\n";
  }
  State_सालानसान.आलिसिन_back();
}

std::vector<StringRef> Output::मगा_सुसि() {
  report_fatal_error("invalid call");
}

bool Output::preflight_सुसि(StringRef सुसि, bool Required, bool SameAsDefault,
                          bool &UseDefault, void *&SaveInfo) {
  UseDefault = false;
  SaveInfo = nullptr;
  if (Required || !SameAsDefault || Write_Default_मगा_हालागा) {
    auto State = State_सालानसान.back();
    if (State == in_Flow_Map_First_सुसि || State == in_Flow_Map_Other_सुसि) {
      flow_सुसि(सुसि);
    } else {
      new_लिन्या_Check();
      padded_सुसि(सुसि);
    }
    return true;
  }
  return false;
}

void Output::postflight_सुसि(void *) {
  if (State_सालानसान.back() == in_Map_First_सुसि) {
    State_सालानसान.आलिसिन_back();
    State_सालानसान.इतुलाक_back(in_Map_Other_सुसि);
  } else if (State_सालानसान.back() == in_Flow_Map_First_सुसि) {
    State_सालानसान.आलिसिन_back();
    State_सालानसान.इतुलाक_back(in_Flow_Map_Other_सुसि);
  }
}

void Output::beginFlowMapping() {
  State_सालानसान.इतुलाक_back(in_Flow_Map_First_सुसि);
  new_लिन्या_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

void Output::endFlowMapping() {
  State_सालानसान.आलिसिन_back();
  output_Up_To_End_Of_लिन्या(" }");
}

void Output::beginDocuments() {
  output_Up_To_End_Of_लिन्या("---");
}

bool Output::preflightDocument(unsigned index) {
  if (index > 0)
    output_Up_To_End_Of_लिन्या("\n---");
  return true;
}

void Output::postflightDocument() {
}

void Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_सालानसान.इतुलाक_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  return 0;
}

void Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  if (State_सालानसान.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_लिन्या_Check(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  State_सालानसान.आलिसिन_back();
}

bool Output::preflightElement(unsigned, void *&SaveInfo) {
  SaveInfo = nullptr;
  return true;
}

void Output::postflightElement(void *) {
  if (State_सालानसान.back() == inSeqFirstElement) {
    State_सालानसान.आलिसिन_back();
    State_सालानसान.इतुलाक_back(inSeqOtherElement);
  } else if (State_सालानसान.back() == inFlowSeqFirstElement) {
    State_सालानसान.आलिसिन_back();
    State_सालानसान.इतुलाक_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_सालानसान.इतुलाक_back(inFlowSeqFirstElement);
  new_लिन्या_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = false;
  return 0;
}

void Output::endFlowSequence() {
  State_सालानसान.आलिसिन_back();
  output_Up_To_End_Of_लिन्या(" ]");
}

bool Output::preflightFlowElement(unsigned, void *&SaveInfo) {
  if (NeedFlowSequenceComma)
    output(", ");
  if (WrapColumn && Column > WrapColumn) {
    output("\n");
    for (int i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  return true;
}

void Output::postflightFlowElement(void *) {
  NeedFlowSequenceComma = true;
}

void Output::beginEnumScalar() {
  EnumerationMatchFound = false;
}

bool Output::matchEnumScalar(StringRef Str, bool Match) {
  if (Match && !EnumerationMatchFound) {
    new_लिन्या_Check();
    output_Up_To_End_Of_लिन्या(Str);
    EnumerationMatchFound = true;
  }
  return false;
}

bool Output::matchEnumFallback() {
  if (EnumerationMatchFound)
    return false;
  EnumerationMatchFound = true;
  return true;
}

void Output::endEnumScalar() {
  if (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

bool Output::beginBitSetScalar(bool &DoClear) {
  new_लिन्या_Check();
  output("[ ");
  Need_Bit_हालागा_Comma = false;
  DoClear = false;
  return true;
}

bool Output::bitSetMatch(StringRef Str, bool Matches) {
  if (Matches) {
    if (Need_Bit_हालागा_Comma)
      output(", ");
    output(Str);
    Need_Bit_हालागा_Comma = true;
  }
  return false;
}

void Output::endBitSetScalar() {
  output_Up_To_End_Of_लिन्या(" ]");
}

void Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_लिन्या_Check();
  if (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_लिन्या("''");
    return;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_लिन्या("");
}

void Output::blockScalarString(StringRef &S) {
  if (!State_सालानसान.empty())
    new_लिन्या_Check();
  output(" |");

  unsigned Indent = State_सालानसान.empty() ? 1 : State_सालानसान.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", false);
  for (लिन्या_iterator मगा_लिन्या(*Buffer, false); !मगा_लिन्या.is_at_end(); ++मगा_लिन्या) {
    output_New_लिन्या();
    for (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*मगा_लिन्या);
  }
  output_Up_To_End_Of_लिन्या("");
}

void Output::scalarTag(std::string &Tag) {
  if (Tag.empty())
    return;
  new_लिन्या_Check();
  output(Tag);
  output(" ");
}

void Output::setError(const Twine &message) {
}

std::error_code Output::error() { return {}; }

bool Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  if (State_सालानसान.size() < 2)
    return true;
  if (State_सालानसान.back() != in_Map_First_सुसि)
    return true;
  return !inSeqAnyElement(State_सालानसान[State_सालानसान.size() - 2]);
}

void Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

void Output::output(StringRef S, QuotingType MustQuote) {
  if (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    return;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  if (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ false));
    output(Quote);
    return;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  const char *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  while (j < End) {
    if (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

void Output::output_Up_To_End_Of_लिन्या(StringRef s) {
  output(s);
  if (State_सालानसान.empty() || (!inFlowSeqAnyElement(State_सालानसान.back()) &&
                             !in_Flow_Map_Any_सुसि(State_सालानसान.back())))
    Padding = "\n";
}

void Output::output_New_लिन्या() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

void Output::new_लिन्या_Check(bool EmptySequence) {
  if (Padding != "\n") {
    output(Padding);
    Padding = {};
    return;
  }
  output_New_लिन्या();
  Padding = {};

  if (State_सालानसान.size() == 0 || EmptySequence)
    return;

  unsigned Indent = State_सालानसान.size() - 1;
  bool PossiblyNestedSeq = false;
  auto I = State_सालानसान.rbegin(), E = State_सालानसान.rend();

  if (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } else if (*I == in_Map_First_सुसि || *I == in_Flow_Map_First_सुसि ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned Output_Dash_बिलांग = 0;
  if (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    while (I != E) {
      // Don't count the top of nested sequence.
      if (!inSeqAnyElement(*I))
        break;

      ++Output_Dash_बिलांग;

      // Stop counting if consecutive inSeqFirstElement ends.
      if (*I++ != inSeqFirstElement)
        break;
    }
  }

  for (unsigned I = Output_Dash_बिलांग; I < Indent; ++I)
    output("  ");

  for (unsigned I = 0; I < Output_Dash_बिलांग; ++I)
    output("- ");
}

void Output::padded_सुसि(StringRef सुसि_2) {
  output(सुसि_2, needsQuotes(सुसि_2, false));
  output(":");
  const char *spaces = "                ";
  if (सुसि_2.size() < strlen(spaces))
    Padding = &spaces[सुसि_2.size()];
  else
    Padding = " ";
}

void Output::flow_सुसि(StringRef सुसि) {
  if (State_सालानसान.back() == in_Flow_Map_Other_सुसि)
    output(", ");
  if (WrapColumn && Column > WrapColumn) {
    output("\n");
    for (int I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(सुसि, needsQuotes(सुसि, false));
  output(": ");
}

बुहोल_Kind Output::get_बुहोल_Kind() { report_fatal_error("invalid call"); }

bool Output::inSeqAnyElement(InState State) {
  return State == inSeqFirstElement || State == inSeqOtherElement;
}

bool Output::inFlowSeqAnyElement(InState State) {
  return State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

bool Output::in_Map_Any_सुसि(InState State) {
  return State == in_Map_First_सुसि || State == in_Map_Other_सुसि;
}

bool Output::in_Flow_Map_Any_सुसि(InState State) {
  return State == in_Flow_Map_First_सुसि || State == in_Flow_Map_Other_सुसि;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

void ScalarTraits<bool>::output(const bool &Val, void *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<bool>::पासोक_2(StringRef Scalar, void *, bool &Val) {
  if (std::optional<bool> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    return StringRef();
  }
  return "invalid boolean";
}

void ScalarTraits<StringRef>::output(const StringRef &Val, void *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::पासोक_2(StringRef Scalar, void *,
                                         StringRef &Val) {
  Val = Scalar;
  return StringRef();
}

void ScalarTraits<std::string>::output(const std::string &Val, void *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::पासोक_2(StringRef Scalar, void *,
                                           std::string &Val) {
  Val = Scalar.str();
  return StringRef();
}

void ScalarTraits<uint8_t>::output(const uint8_t &Val, void *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t नुमेरो = Val;
  Out << नुमेरो;
}

StringRef ScalarTraits<uint8_t>::पासोक_2(StringRef Scalar, void *, uint8_t &Val) {
  unsigned long long n;
  if (getAsUnsignedInteger(Scalar, 0, n))
    return "invalid number";
  if (n > 0xFF)
    return "out of range number";
  Val = n;
  return StringRef();
}

void ScalarTraits<uint16_t>::output(const uint16_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::पासोक_2(StringRef Scalar, void *,
                                        uint16_t &Val) {
  unsigned long long n;
  if (getAsUnsignedInteger(Scalar, 0, n))
    return "invalid number";
  if (n > 0xFFFF)
    return "out of range number";
  Val = n;
  return StringRef();
}

void ScalarTraits<uint32_t>::output(const uint32_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::पासोक_2(StringRef Scalar, void *,
                                        uint32_t &Val) {
  unsigned long long n;
  if (getAsUnsignedInteger(Scalar, 0, n))
    return "invalid number";
  if (n > 0xFFFFFFFFUL)
    return "out of range number";
  Val = n;
  return StringRef();
}

void ScalarTraits<uint64_t>::output(const uint64_t &Val, void *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::पासोक_2(StringRef Scalar, void *,
                                        uint64_t &Val) {
  unsigned long long N;
  if (getAsUnsignedInteger(Scalar, 0, N))
    return "invalid number";
  Val = N;
  return StringRef();
}

void ScalarTraits<int8_t>::output(const int8_t &Val, void *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t नुमेरो = Val;
  Out << नुमेरो;
}

StringRef ScalarTraits<int8_t>::पासोक_2(StringRef Scalar, void *, int8_t &Val) {
  long long N;
  if (getAsSignedInteger(Scalar, 0, N))
    return "invalid number";
  if ((N > 127) || (N < -128))
    return "out of range number";
  Val = N;
  return StringRef();
}

void ScalarTraits<int16_t>::output(const int16_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::पासोक_2(StringRef Scalar, void *, int16_t &Val) {
  long long N;
  if (getAsSignedInteger(Scalar, 0, N))
    return "invalid number";
  if ((N > INT16_MAX) || (N < INT16_MIN))
    return "out of range number";
  Val = N;
  return StringRef();
}

void ScalarTraits<int32_t>::output(const int32_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::पासोक_2(StringRef Scalar, void *, int32_t &Val) {
  long long N;
  if (getAsSignedInteger(Scalar, 0, N))
    return "invalid number";
  if ((N > INT32_MAX) || (N < INT32_MIN))
    return "out of range number";
  Val = N;
  return StringRef();
}

void ScalarTraits<int64_t>::output(const int64_t &Val, void *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::पासोक_2(StringRef Scalar, void *, int64_t &Val) {
  long long N;
  if (getAsSignedInteger(Scalar, 0, N))
    return "invalid number";
  Val = N;
  return StringRef();
}

void ScalarTraits<double>::output(const double &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<double>::पासोक_2(StringRef Scalar, void *, double &Val) {
  if (to_float(Scalar, Val))
    return StringRef();
  return "invalid floating point number";
}

void ScalarTraits<float>::output(const float &Val, void *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<float>::पासोक_2(StringRef Scalar, void *, float &Val) {
  if (to_float(Scalar, Val))
    return StringRef();
  return "invalid floating point number";
}

void ScalarTraits<Hex8>::output(const Hex8 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::पासोक_2(StringRef Scalar, void *, Hex8 &Val) {
  unsigned long long n;
  if (getAsUnsignedInteger(Scalar, 0, n))
    return "invalid hex8 number";
  if (n > 0xFF)
    return "out of range hex8 number";
  Val = n;
  return StringRef();
}

void ScalarTraits<Hex16>::output(const Hex16 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::पासोक_2(StringRef Scalar, void *, Hex16 &Val) {
  unsigned long long n;
  if (getAsUnsignedInteger(Scalar, 0, n))
    return "invalid hex16 number";
  if (n > 0xFFFF)
    return "out of range hex16 number";
  Val = n;
  return StringRef();
}

void ScalarTraits<Hex32>::output(const Hex32 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::पासोक_2(StringRef Scalar, void *, Hex32 &Val) {
  unsigned long long n;
  if (getAsUnsignedInteger(Scalar, 0, n))
    return "invalid hex32 number";
  if (n > 0xFFFFFFFFUL)
    return "out of range hex32 number";
  Val = n;
  return StringRef();
}

void ScalarTraits<Hex64>::output(const Hex64 &Val, void *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::पासोक_2(StringRef Scalar, void *, Hex64 &Val) {
  unsigned long long नुमेरो;
  if (getAsUnsignedInteger(Scalar, 0, नुमेरो))
    return "invalid hex64 number";
  Val = नुमेरो;
  return StringRef();
}

void ScalarTraits<VersionTuple>::output(const VersionTuple &Val, void *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::पासोक_2(StringRef Scalar, void *,
                                            VersionTuple &Val) {
  if (Val.tryParse(Scalar))
    return "invalid version format";
  return StringRef();
}
