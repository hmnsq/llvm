//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/Support/YAMLTraits.h"
#অন্তর্ভাৱ "llvm/ADT/STLExtras.h"
#অন্তর্ভাৱ "llvm/ADT/SmallString.h"
#অন্তর্ভাৱ "llvm/ADT/StringExtras.h"
#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ "llvm/ADT/Twine.h"
#অন্তর্ভাৱ "llvm/Support/Casting.h"
#অন্তর্ভাৱ "llvm/Support/Errc.h"
#অন্তর্ভাৱ "llvm/Support/ErrorHandling.h"
#অন্তর্ভাৱ "llvm/Support/Format.h"
#অন্তর্ভাৱ "llvm/Support/LineIterator.h"
#অন্তর্ভাৱ "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাৱ "llvm/Support/VersionTuple.h"
#অন্তর্ভাৱ "llvm/Support/YAMLParser.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"
#অন্তর্ভাৱ <cassert>
#অন্তর্ভাৱ <cstdint>
#অন্তর্ভাৱ <cstring>
#অন্তর্ভাৱ <string>
#অন্তর্ভাৱ <vector>

using নামাকাশ llvm;
using নামাকাশ yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(শূন্য *Context) : Ctxt(Context) {}

IO::~IO() = স্ৱভাৱ;

শূন্য *IO::getContext() ধ্রুৱ {
  ঘূৰাই Ctxt;
}

শূন্য IO::setContext(শূন্য *Context) {
  Ctxt = Context;
}

শূন্য IO::set_Allow_Unknown_চাবিগুলি(তার্কিক Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//প্ৰবেশ
//===----------------------------------------------------------------------===//

প্ৰবেশ::প্ৰবেশ(StringRef প্ৰবেশ_Content, শূন্য *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, শূন্য *DiagHandlerCtxt)
    : IO(Ctxt), Strm(নূতন Stream(প্ৰবেশ_Content, SrcMgr, মিছা, &EC)) {
  যদি (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

প্ৰবেশ::প্ৰবেশ(MemoryBufferRef প্ৰবেশ, শূন্য *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, শূন্য *DiagHandlerCtxt)
    : IO(Ctxt), Strm(নূতন Stream(প্ৰবেশ, SrcMgr, মিছা, &EC)) {
  যদি (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

প্ৰবেশ::~প্ৰবেশ() = স্ৱভাৱ;

std::error_code প্ৰবেশ::error() { ঘূৰাই EC; }

তার্কিক প্ৰবেশ::outputting() ধ্রুৱ {
  ঘূৰাই মিছা;
}

তার্কিক প্ৰবেশ::setCurrentDocument() {
  যদি (DocIterator != Strm->end()) {
    গ্ৰন্থি *N = DocIterator->get_মূল();
    যদি (!N) {
      EC = make_error_code(errc::invalid_argument);
      ঘূৰাই মিছা;
    }

    যদি (isa<Null_গ্ৰন্থি>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      ঘূৰাই setCurrentDocument();
    }
    releaseHNodeBuffers();
    শীৰ্ষ_গ্ৰন্থি = createHNodes(N);
    Current_গ্ৰন্থি = শীৰ্ষ_গ্ৰন্থি;
    ঘূৰাই সঁচা;
  }
  ঘূৰাই মিছা;
}

তার্কিক প্ৰবেশ::nextDocument() {
  ঘূৰাই ++DocIterator != Strm->end();
}

ধ্রুৱ গ্ৰন্থি *প্ৰবেশ::get_Current_গ্ৰন্থি() ধ্রুৱ {
  ঘূৰাই Current_গ্ৰন্থি ? Current_গ্ৰন্থি->গ্ৰন্থি_2 : nullptr;
}

তার্কিক প্ৰবেশ::mapTag(StringRef Tag, তার্কিক Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  যদি (!Current_গ্ৰন্থি)
    ঘূৰাই মিছা;

  std::string foundTag = Current_গ্ৰন্থি->গ্ৰন্থি_2->getVerbatimTag();
  যদি (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    ঘূৰাই Default;
  }
  // Return true iff found tag matches supplied tag.
  ঘূৰাই Tag == foundTag;
}

শূন্য প্ৰবেশ::beginMapping() {
  যদি (EC)
    ঘূৰাই;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_গ্ৰন্থি);
  যদি (MN) {
    MN->Valid_চাবিগুলি.clear();
  }
}

std::vector<StringRef> প্ৰবেশ::চাবিগুলি() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_গ্ৰন্থি);
  std::vector<StringRef> Ret;
  যদি (!MN) {
    setError(Current_গ্ৰন্থি, "not a mapping");
    ঘূৰাই Ret;
  }
  বাবে (auto &P : MN->Mapping)
    Ret.ঠেলো_back(P.first());
  ঘূৰাই Ret;
}

তার্কিক প্ৰবেশ::preflight_চাবি(StringRef চাবি, তার্কিক Required, তার্কিক, তার্কিক &UseDefault,
                         শূন্য *&SaveInfo) {
  UseDefault = মিছা;
  যদি (EC)
    ঘূৰাই মিছা;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  যদি (!Current_গ্ৰন্থি) {
    যদি (Required)
      EC = make_error_code(errc::invalid_argument);
    অন্যথা
      UseDefault = সঁচা;
    ঘূৰাই মিছা;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_গ্ৰন্থি);
  যদি (!MN) {
    যদি (Required || !isa<EmptyHNode>(Current_গ্ৰন্থি))
      setError(Current_গ্ৰন্থি, "not a mapping");
    অন্যথা
      UseDefault = সঁচা;
    ঘূৰাই মিছা;
  }
  MN->Valid_চাবিগুলি.ঠেলো_back(চাবি.str());
  HNode *মান = MN->Mapping[চাবি].first;
  যদি (!মান) {
    যদি (Required)
      setError(Current_গ্ৰন্থি, Twine("missing required key '") + চাবি + "'");
    অন্যথা
      UseDefault = সঁচা;
    ঘূৰাই মিছা;
  }
  SaveInfo = Current_গ্ৰন্থি;
  Current_গ্ৰন্থি = মান;
  ঘূৰাই সঁচা;
}

শূন্য প্ৰবেশ::postflight_চাবি(শূন্য *saveInfo) {
  Current_গ্ৰন্থি = reinterpret_cast<HNode *>(saveInfo);
}

শূন্য প্ৰবেশ::endMapping() {
  যদি (EC)
    ঘূৰাই;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_গ্ৰন্থি);
  যদি (!MN)
    ঘূৰাই;
  বাবে (ধ্রুৱ auto &NN : MN->Mapping) {
    যদি (!is_contained(MN->Valid_চাবিগুলি, NN.first())) {
      ধ্রুৱ SMRange &ReportLoc = NN.second.second;
      যদি (!Allow_Unknown_চাবিগুলি) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        ভঙ্গ;
      } অন্যথা
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

শূন্য প্ৰবেশ::beginFlowMapping() { beginMapping(); }

শূন্য প্ৰবেশ::endFlowMapping() { endMapping(); }

unsigned প্ৰবেশ::beginSequence() {
  যদি (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_গ্ৰন্থি))
    ঘূৰাই SQ->Entries.size();
  যদি (isa<EmptyHNode>(Current_গ্ৰন্থি))
    ঘূৰাই 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  যদি (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_গ্ৰন্থি)) {
    যদি (isNull(SN->মান_2()))
      ঘূৰাই 0;
  }
  // Any other type of HNode is an error.
  setError(Current_গ্ৰন্থি, "not a sequence");
  ঘূৰাই 0;
}

শূন্য প্ৰবেশ::endSequence() {
}

তার্কিক প্ৰবেশ::preflightElement(unsigned Index, শূন্য *&SaveInfo) {
  যদি (EC)
    ঘূৰাই মিছা;
  যদি (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_গ্ৰন্থি)) {
    SaveInfo = Current_গ্ৰন্থি;
    Current_গ্ৰন্থি = SQ->Entries[Index];
    ঘূৰাই সঁচা;
  }
  ঘূৰাই মিছা;
}

শূন্য প্ৰবেশ::postflightElement(শূন্য *SaveInfo) {
  Current_গ্ৰন্থি = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned প্ৰবেশ::beginFlowSequence() { ঘূৰাই beginSequence(); }

তার্কিক প্ৰবেশ::preflightFlowElement(unsigned index, শূন্য *&SaveInfo) {
  যদি (EC)
    ঘূৰাই মিছা;
  যদি (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_গ্ৰন্থি)) {
    SaveInfo = Current_গ্ৰন্থি;
    Current_গ্ৰন্থি = SQ->Entries[index];
    ঘূৰাই সঁচা;
  }
  ঘূৰাই মিছা;
}

শূন্য প্ৰবেশ::postflightFlowElement(শূন্য *SaveInfo) {
  Current_গ্ৰন্থি = reinterpret_cast<HNode *>(SaveInfo);
}

শূন্য প্ৰবেশ::endFlowSequence() {
}

শূন্য প্ৰবেশ::beginEnumScalar() {
  ScalarMatchFound = মিছা;
}

তার্কিক প্ৰবেশ::matchEnumScalar(StringRef Str, তার্কিক) {
  যদি (ScalarMatchFound)
    ঘূৰাই মিছা;
  যদি (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_গ্ৰন্থি)) {
    যদি (SN->মান_2() == Str) {
      ScalarMatchFound = সঁচা;
      ঘূৰাই সঁচা;
    }
  }
  ঘূৰাই মিছা;
}

তার্কিক প্ৰবেশ::matchEnumFallback() {
  যদি (ScalarMatchFound)
    ঘূৰাই মিছা;
  ScalarMatchFound = সঁচা;
  ঘূৰাই সঁচা;
}

শূন্য প্ৰবেশ::endEnumScalar() {
  যদি (!ScalarMatchFound) {
    setError(Current_গ্ৰন্থি, "unknown enumerated scalar");
  }
}

তার্কিক প্ৰবেশ::beginBitSetScalar(তার্কিক &DoClear) {
  Bit_মানসমূহ_Used.clear();
  যদি (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_গ্ৰন্থি)) {
    Bit_মানসমূহ_Used.resize(SQ->Entries.size());
  } অন্যথা {
    setError(Current_গ্ৰন্থি, "expected sequence of bit values");
  }
  DoClear = সঁচা;
  ঘূৰাই সঁচা;
}

তার্কিক প্ৰবেশ::bitSetMatch(StringRef Str, তার্কিক) {
  যদি (EC)
    ঘূৰাই মিছা;
  যদি (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_গ্ৰন্থি)) {
    unsigned Index = 0;
    বাবে (auto &N : SQ->Entries) {
      যদি (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        যদি (SN->মান_2() == Str) {
          Bit_মানসমূহ_Used[Index] = সঁচা;
          ঘূৰাই সঁচা;
        }
      } অন্যথা {
        setError(Current_গ্ৰন্থি, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } অন্যথা {
    setError(Current_গ্ৰন্থি, "expected sequence of bit values");
  }
  ঘূৰাই মিছা;
}

শূন্য প্ৰবেশ::endBitSetScalar() {
  যদি (EC)
    ঘূৰাই;
  যদি (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_গ্ৰন্থি)) {
    প্রতিজ্ঞা(Bit_মানসমূহ_Used.size() == SQ->Entries.size());
    বাবে (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      যদি (!Bit_মানসমূহ_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        ঘূৰাই;
      }
    }
  }
}

শূন্য প্ৰবেশ::scalarString(StringRef &S, QuotingType) {
  যদি (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_গ্ৰন্থি)) {
    S = SN->মান_2();
  } অন্যথা {
    setError(Current_গ্ৰন্থি, "unexpected scalar");
  }
}

শূন্য প্ৰবেশ::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

শূন্য প্ৰবেশ::scalarTag(std::string &Tag) {
  Tag = Current_গ্ৰন্থি->গ্ৰন্থি_2->getVerbatimTag();
}

শূন্য প্ৰবেশ::setError(HNode *hnode, ধ্রুৱ Twine &message) {
  প্রতিজ্ঞা(hnode && "HNode must not be NULL");
  setError(hnode->গ্ৰন্থি_2, message);
}

গ্ৰন্থি_Kind প্ৰবেশ::get_গ্ৰন্থি_Kind() {
  যদি (isa<ScalarHNode>(Current_গ্ৰন্থি))
    ঘূৰাই গ্ৰন্থি_Kind::Scalar;
  অন্যথা যদি (isa<MapHNode>(Current_গ্ৰন্থি))
    ঘূৰাই গ্ৰন্থি_Kind::Map;
  অন্যথা যদি (isa<SequenceHNode>(Current_গ্ৰন্থি))
    ঘূৰাই গ্ৰন্থি_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

শূন্য প্ৰবেশ::setError(গ্ৰন্থি *গ্ৰন্থি_3, ধ্রুৱ Twine &message) {
  Strm->printError(গ্ৰন্থি_3, message);
  EC = make_error_code(errc::invalid_argument);
}

শূন্য প্ৰবেশ::setError(ধ্রুৱ SMRange &range, ধ্রুৱ Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

শূন্য প্ৰবেশ::reportWarning(HNode *hnode, ধ্রুৱ Twine &message) {
  প্রতিজ্ঞা(hnode && "HNode must not be NULL");
  Strm->printError(hnode->গ্ৰন্থি_2, message, SourceMgr::DK_Warning);
}

শূন্য প্ৰবেশ::reportWarning(গ্ৰন্থি *গ্ৰন্থি_3, ধ্রুৱ Twine &message) {
  Strm->printError(গ্ৰন্থি_3, message, SourceMgr::DK_Warning);
}

শূন্য প্ৰবেশ::reportWarning(ধ্রুৱ SMRange &range, ধ্রুৱ Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

শূন্য প্ৰবেশ::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

শূন্য প্ৰবেশ::saveAliasHNode(গ্ৰন্থি *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  যদি (!Anchor.empty())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

প্ৰবেশ::HNode *প্ৰবেশ::createHNodes(গ্ৰন্থি *N) {
  SmallString<128> StringStorage;
  ৱিকল্প (N->getType()) {
  পক্ষ গ্ৰন্থি::NK_Scalar: {
    Scalar_গ্ৰন্থি *SN = dyn_cast<Scalar_গ্ৰন্থি>(N);
    StringRef চাবি_Str = SN->get_মান(StringStorage);
    যদি (!StringStorage.empty()) {
      // Copy string to permanent storage
      চাবি_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = নূতন (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, চাবি_Str);
    saveAliasHNode(SN, SHNode);
    ঘূৰাই SHNode;
  }
  পক্ষ গ্ৰন্থি::NK_BlockScalar: {
    Block_Scalar_গ্ৰন্থি *BSN = dyn_cast<Block_Scalar_গ্ৰন্থি>(N);
    StringRef মান_Copy = BSN->get_মান().copy(StringAllocator);
    auto *BSHNode =
        নূতন (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, মান_Copy);
    saveAliasHNode(BSN, BSHNode);
    ঘূৰাই BSHNode;
  }
  পক্ষ গ্ৰন্থি::NK_Sequence: {
    Sequence_গ্ৰন্থি *SQ = dyn_cast<Sequence_গ্ৰন্থি>(N);
    auto SQHNode = নূতন (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    বাবে (গ্ৰন্থি &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      যদি (EC)
        ভঙ্গ;
      SQHNode->Entries.ঠেলো_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    ঘূৰাই SQHNode;
  }
  পক্ষ গ্ৰন্থি::NK_Mapping: {
    Mapping_গ্ৰন্থি *Map = dyn_cast<Mapping_গ্ৰন্থি>(N);
    auto mapHNode = নূতন (MapHNodeAllocator.Allocate()) MapHNode(N);
    বাবে (চাবি_মান_গ্ৰন্থি &KVN : *Map) {
      গ্ৰন্থি *চাবি_গ্ৰন্থি = KVN.get_চাবি();
      Scalar_গ্ৰন্থি *চাবি = dyn_cast_if_present<Scalar_গ্ৰন্থি>(চাবি_গ্ৰন্থি);
      গ্ৰন্থি *মান = KVN.get_মান();
      যদি (!চাবি || !মান) {
        যদি (!চাবি)
          setError(চাবি_গ্ৰন্থি, "Map key must be a scalar");
        যদি (!মান)
          setError(চাবি_গ্ৰন্থি, "Map value must not be empty");
        ভঙ্গ;
      }
      StringStorage.clear();
      StringRef চাবি_Str = চাবি->get_মান(StringStorage);
      যদি (!StringStorage.empty()) {
        // Copy string to permanent storage
        চাবি_Str = StringStorage.str().copy(StringAllocator);
      }
      যদি (mapHNode->Mapping.গুনতি(চাবি_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(চাবি_গ্ৰন্থি, Twine("duplicated mapping key '") + চাবি_Str + "'");
      auto মান_HNode = createHNodes(মান);
      যদি (EC)
        ভঙ্গ;
      mapHNode->Mapping[চাবি_Str] =
          std::make_pair(std::move(মান_HNode), চাবি_গ্ৰন্থি->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    ঘূৰাই std::move(mapHNode);
  }
  পক্ষ গ্ৰন্থি::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    ঘূৰাই নূতন (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  পক্ষ গ্ৰন্থি::NK_Alias: {
    Alias_গ্ৰন্থি *AN = dyn_cast<Alias_গ্ৰন্থি>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    যদি (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      ঘূৰাই nullptr;
    }
    ঘূৰাই AHN->second;
  }
  স্ৱভাৱ:
    setError(N, "unknown node kind");
    ঘূৰাই nullptr;
  }
}

শূন্য প্ৰবেশ::setError(ধ্রুৱ Twine &Message) {
  setError(Current_গ্ৰন্থি, Message);
}

শূন্য প্ৰবেশ::set_Allow_Unknown_চাবিগুলি(তার্কিক Allow) { Allow_Unknown_চাবিগুলি = Allow; }

তার্কিক প্ৰবেশ::canElideEmptySequence() {
  ঘূৰাই মিছা;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, শূন্য *context, পূর্ণাঙ্ক WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = স্ৱভাৱ;

তার্কিক Output::outputting() ধ্রুৱ {
  ঘূৰাই সঁচা;
}

শূন্য Output::beginMapping() {
  State_স্তূপ.ঠেলো_back(in_Map_First_চাবি);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

তার্কিক Output::mapTag(StringRef Tag, তার্কিক Use) {
  যদি (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    তার্কিক SequenceElement = মিছা;
    যদি (State_স্তূপ.size() > 1) {
      auto &E = State_স্তূপ[State_স্তূপ.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    যদি (SequenceElement && State_স্তূপ.back() == in_Map_First_চাবি) {
      new_সাৰি_Check();
    } অন্যথা {
      output(" ");
    }
    output(Tag);
    যদি (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      যদি (State_স্তূপ.back() == in_Map_First_চাবি) {
        State_স্তূপ.বেৰ_কৰো_back();
        State_স্তূপ.ঠেলো_back(in_Map_Other_চাবি);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  ঘূৰাই Use;
}

শূন্য Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  যদি (State_স্তূপ.back() == in_Map_First_চাবি) {
    Padding = PaddingBeforeContainer;
    new_সাৰি_Check();
    output("{}");
    Padding = "\n";
  }
  State_স্তূপ.বেৰ_কৰো_back();
}

std::vector<StringRef> Output::চাবিগুলি() {
  report_fatal_error("invalid call");
}

তার্কিক Output::preflight_চাবি(StringRef চাবি, তার্কিক Required, তার্কিক SameAsDefault,
                          তার্কিক &UseDefault, শূন্য *&SaveInfo) {
  UseDefault = মিছা;
  SaveInfo = nullptr;
  যদি (Required || !SameAsDefault || Write_Default_মানসমূহ) {
    auto State = State_স্তূপ.back();
    যদি (State == in_Flow_Map_First_চাবি || State == in_Flow_Map_Other_চাবি) {
      flow_চাবি(চাবি);
    } অন্যথা {
      new_সাৰি_Check();
      padded_চাবি(চাবি);
    }
    ঘূৰাই সঁচা;
  }
  ঘূৰাই মিছা;
}

শূন্য Output::postflight_চাবি(শূন্য *) {
  যদি (State_স্তূপ.back() == in_Map_First_চাবি) {
    State_স্তূপ.বেৰ_কৰো_back();
    State_স্তূপ.ঠেলো_back(in_Map_Other_চাবি);
  } অন্যথা যদি (State_স্তূপ.back() == in_Flow_Map_First_চাবি) {
    State_স্তূপ.বেৰ_কৰো_back();
    State_স্তূপ.ঠেলো_back(in_Flow_Map_Other_চাবি);
  }
}

শূন্য Output::beginFlowMapping() {
  State_স্তূপ.ঠেলো_back(in_Flow_Map_First_চাবি);
  new_সাৰি_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

শূন্য Output::endFlowMapping() {
  State_স্তূপ.বেৰ_কৰো_back();
  output_Up_To_End_Of_সাৰি(" }");
}

শূন্য Output::beginDocuments() {
  output_Up_To_End_Of_সাৰি("---");
}

তার্কিক Output::preflightDocument(unsigned index) {
  যদি (index > 0)
    output_Up_To_End_Of_সাৰি("\n---");
  ঘূৰাই সঁচা;
}

শূন্য Output::postflightDocument() {
}

শূন্য Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_স্তূপ.ঠেলো_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  ঘূৰাই 0;
}

শূন্য Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  যদি (State_স্তূপ.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_সাৰি_Check(/*EmptySequence=*/সঁচা);
    output("[]");
    Padding = "\n";
  }
  State_স্তূপ.বেৰ_কৰো_back();
}

তার্কিক Output::preflightElement(unsigned, শূন্য *&SaveInfo) {
  SaveInfo = nullptr;
  ঘূৰাই সঁচা;
}

শূন্য Output::postflightElement(শূন্য *) {
  যদি (State_স্তূপ.back() == inSeqFirstElement) {
    State_স্তূপ.বেৰ_কৰো_back();
    State_স্তূপ.ঠেলো_back(inSeqOtherElement);
  } অন্যথা যদি (State_স্তূপ.back() == inFlowSeqFirstElement) {
    State_স্তূপ.বেৰ_কৰো_back();
    State_স্তূপ.ঠেলো_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_স্তূপ.ঠেলো_back(inFlowSeqFirstElement);
  new_সাৰি_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = মিছা;
  ঘূৰাই 0;
}

শূন্য Output::endFlowSequence() {
  State_স্তূপ.বেৰ_কৰো_back();
  output_Up_To_End_Of_সাৰি(" ]");
}

তার্কিক Output::preflightFlowElement(unsigned, শূন্য *&SaveInfo) {
  যদি (NeedFlowSequenceComma)
    output(", ");
  যদি (WrapColumn && Column > WrapColumn) {
    output("\n");
    বাবে (পূর্ণাঙ্ক i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  ঘূৰাই সঁচা;
}

শূন্য Output::postflightFlowElement(শূন্য *) {
  NeedFlowSequenceComma = সঁচা;
}

শূন্য Output::beginEnumScalar() {
  EnumerationMatchFound = মিছা;
}

তার্কিক Output::matchEnumScalar(StringRef Str, তার্কিক Match) {
  যদি (Match && !EnumerationMatchFound) {
    new_সাৰি_Check();
    output_Up_To_End_Of_সাৰি(Str);
    EnumerationMatchFound = সঁচা;
  }
  ঘূৰাই মিছা;
}

তার্কিক Output::matchEnumFallback() {
  যদি (EnumerationMatchFound)
    ঘূৰাই মিছা;
  EnumerationMatchFound = সঁচা;
  ঘূৰাই সঁচা;
}

শূন্য Output::endEnumScalar() {
  যদি (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

তার্কিক Output::beginBitSetScalar(তার্কিক &DoClear) {
  new_সাৰি_Check();
  output("[ ");
  Need_Bit_মান_Comma = মিছা;
  DoClear = মিছা;
  ঘূৰাই সঁচা;
}

তার্কিক Output::bitSetMatch(StringRef Str, তার্কিক Matches) {
  যদি (Matches) {
    যদি (Need_Bit_মান_Comma)
      output(", ");
    output(Str);
    Need_Bit_মান_Comma = সঁচা;
  }
  ঘূৰাই মিছা;
}

শূন্য Output::endBitSetScalar() {
  output_Up_To_End_Of_সাৰি(" ]");
}

শূন্য Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_সাৰি_Check();
  যদি (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_সাৰি("''");
    ঘূৰাই;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_সাৰি("");
}

শূন্য Output::blockScalarString(StringRef &S) {
  যদি (!State_স্তূপ.empty())
    new_সাৰি_Check();
  output(" |");

  unsigned Indent = State_স্তূপ.empty() ? 1 : State_স্তূপ.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", মিছা);
  বাবে (সাৰি_iterator সাৰিগুলি(*Buffer, মিছা); !সাৰিগুলি.is_at_end(); ++সাৰিগুলি) {
    output_New_সাৰি();
    বাবে (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*সাৰিগুলি);
  }
  output_Up_To_End_Of_সাৰি("");
}

শূন্য Output::scalarTag(std::string &Tag) {
  যদি (Tag.empty())
    ঘূৰাই;
  new_সাৰি_Check();
  output(Tag);
  output(" ");
}

শূন্য Output::setError(ধ্রুৱ Twine &message) {
}

std::error_code Output::error() { ঘূৰাই {}; }

তার্কিক Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  যদি (State_স্তূপ.size() < 2)
    ঘূৰাই সঁচা;
  যদি (State_স্তূপ.back() != in_Map_First_চাবি)
    ঘূৰাই সঁচা;
  ঘূৰাই !inSeqAnyElement(State_স্তূপ[State_স্তূপ.size() - 2]);
}

শূন্য Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

শূন্য Output::output(StringRef S, QuotingType MustQuote) {
  যদি (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    ঘূৰাই;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  যদি (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ মিছা));
    output(Quote);
    ঘূৰাই;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  ধ্রুৱ ৱর্ণ *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  যেতিয়ালৈ (j < End) {
    যদি (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

শূন্য Output::output_Up_To_End_Of_সাৰি(StringRef s) {
  output(s);
  যদি (State_স্তূপ.empty() || (!inFlowSeqAnyElement(State_স্তূপ.back()) &&
                             !in_Flow_Map_Any_চাবি(State_স্তূপ.back())))
    Padding = "\n";
}

শূন্য Output::output_New_সাৰি() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

শূন্য Output::new_সাৰি_Check(তার্কিক EmptySequence) {
  যদি (Padding != "\n") {
    output(Padding);
    Padding = {};
    ঘূৰাই;
  }
  output_New_সাৰি();
  Padding = {};

  যদি (State_স্তূপ.size() == 0 || EmptySequence)
    ঘূৰাই;

  unsigned Indent = State_স্তূপ.size() - 1;
  তার্কিক PossiblyNestedSeq = মিছা;
  auto I = State_স্তূপ.rbegin(), E = State_স্তূপ.rend();

  যদি (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = সঁচা; // Not possibly but always.
    ++Indent;
  } অন্যথা যদি (*I == in_Map_First_চাবি || *I == in_Flow_Map_First_চাবি ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = সঁচা;
    ++I; // Skip back().
  }

  unsigned Output_Dash_গুনতি = 0;
  যদি (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    যেতিয়ালৈ (I != E) {
      // Don't count the top of nested sequence.
      যদি (!inSeqAnyElement(*I))
        ভঙ্গ;

      ++Output_Dash_গুনতি;

      // Stop counting if consecutive inSeqFirstElement ends.
      যদি (*I++ != inSeqFirstElement)
        ভঙ্গ;
    }
  }

  বাবে (unsigned I = Output_Dash_গুনতি; I < Indent; ++I)
    output("  ");

  বাবে (unsigned I = 0; I < Output_Dash_গুনতি; ++I)
    output("- ");
}

শূন্য Output::padded_চাবি(StringRef চাবি_2) {
  output(চাবি_2, needsQuotes(চাবি_2, মিছা));
  output(":");
  ধ্রুৱ ৱর্ণ *spaces = "                ";
  যদি (চাবি_2.size() < strlen(spaces))
    Padding = &spaces[চাবি_2.size()];
  অন্যথা
    Padding = " ";
}

শূন্য Output::flow_চাবি(StringRef চাবি) {
  যদি (State_স্তূপ.back() == in_Flow_Map_Other_চাবি)
    output(", ");
  যদি (WrapColumn && Column > WrapColumn) {
    output("\n");
    বাবে (পূর্ণাঙ্ক I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(চাবি, needsQuotes(চাবি, মিছা));
  output(": ");
}

গ্ৰন্থি_Kind Output::get_গ্ৰন্থি_Kind() { report_fatal_error("invalid call"); }

তার্কিক Output::inSeqAnyElement(InState State) {
  ঘূৰাই State == inSeqFirstElement || State == inSeqOtherElement;
}

তার্কিক Output::inFlowSeqAnyElement(InState State) {
  ঘূৰাই State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

তার্কিক Output::in_Map_Any_চাবি(InState State) {
  ঘূৰাই State == in_Map_First_চাবি || State == in_Map_Other_চাবি;
}

তার্কিক Output::in_Flow_Map_Any_চাবি(InState State) {
  ঘূৰাই State == in_Flow_Map_First_চাবি || State == in_Flow_Map_Other_চাবি;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

শূন্য ScalarTraits<তার্কিক>::output(ধ্রুৱ তার্কিক &Val, শূন্য *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<তার্কিক>::প্ৰবেশ_2(StringRef Scalar, শূন্য *, তার্কিক &Val) {
  যদি (std::optional<তার্কিক> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    ঘূৰাই StringRef();
  }
  ঘূৰাই "invalid boolean";
}

শূন্য ScalarTraits<StringRef>::output(ধ্রুৱ StringRef &Val, শূন্য *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::প্ৰবেশ_2(StringRef Scalar, শূন্য *,
                                         StringRef &Val) {
  Val = Scalar;
  ঘূৰাই StringRef();
}

শূন্য ScalarTraits<std::string>::output(ধ্রুৱ std::string &Val, শূন্য *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::প্ৰবেশ_2(StringRef Scalar, শূন্য *,
                                           std::string &Val) {
  Val = Scalar.str();
  ঘূৰাই StringRef();
}

শূন্য ScalarTraits<uint8_t>::output(ধ্রুৱ uint8_t &Val, শূন্য *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t অঙ্ক = Val;
  Out << অঙ্ক;
}

StringRef ScalarTraits<uint8_t>::প্ৰবেশ_2(StringRef Scalar, শূন্য *, uint8_t &Val) {
  unsigned long long n;
  যদি (getAsUnsignedInteger(Scalar, 0, n))
    ঘূৰাই "invalid number";
  যদি (n > 0xFF)
    ঘূৰাই "out of range number";
  Val = n;
  ঘূৰাই StringRef();
}

শূন্য ScalarTraits<uint16_t>::output(ধ্রুৱ uint16_t &Val, শূন্য *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::প্ৰবেশ_2(StringRef Scalar, শূন্য *,
                                        uint16_t &Val) {
  unsigned long long n;
  যদি (getAsUnsignedInteger(Scalar, 0, n))
    ঘূৰাই "invalid number";
  যদি (n > 0xFFFF)
    ঘূৰাই "out of range number";
  Val = n;
  ঘূৰাই StringRef();
}

শূন্য ScalarTraits<uint32_t>::output(ধ্রুৱ uint32_t &Val, শূন্য *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::প্ৰবেশ_2(StringRef Scalar, শূন্য *,
                                        uint32_t &Val) {
  unsigned long long n;
  যদি (getAsUnsignedInteger(Scalar, 0, n))
    ঘূৰাই "invalid number";
  যদি (n > 0xFFFFFFFFUL)
    ঘূৰাই "out of range number";
  Val = n;
  ঘূৰাই StringRef();
}

শূন্য ScalarTraits<uint64_t>::output(ধ্রুৱ uint64_t &Val, শূন্য *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::প্ৰবেশ_2(StringRef Scalar, শূন্য *,
                                        uint64_t &Val) {
  unsigned long long N;
  যদি (getAsUnsignedInteger(Scalar, 0, N))
    ঘূৰাই "invalid number";
  Val = N;
  ঘূৰাই StringRef();
}

শূন্য ScalarTraits<int8_t>::output(ধ্রুৱ int8_t &Val, শূন্য *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t অঙ্ক = Val;
  Out << অঙ্ক;
}

StringRef ScalarTraits<int8_t>::প্ৰবেশ_2(StringRef Scalar, শূন্য *, int8_t &Val) {
  long long N;
  যদি (getAsSignedInteger(Scalar, 0, N))
    ঘূৰাই "invalid number";
  যদি ((N > 127) || (N < -128))
    ঘূৰাই "out of range number";
  Val = N;
  ঘূৰাই StringRef();
}

শূন্য ScalarTraits<int16_t>::output(ধ্রুৱ int16_t &Val, শূন্য *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::প্ৰবেশ_2(StringRef Scalar, শূন্য *, int16_t &Val) {
  long long N;
  যদি (getAsSignedInteger(Scalar, 0, N))
    ঘূৰাই "invalid number";
  যদি ((N > INT16_MAX) || (N < INT16_MIN))
    ঘূৰাই "out of range number";
  Val = N;
  ঘূৰাই StringRef();
}

শূন্য ScalarTraits<int32_t>::output(ধ্রুৱ int32_t &Val, শূন্য *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::প্ৰবেশ_2(StringRef Scalar, শূন্য *, int32_t &Val) {
  long long N;
  যদি (getAsSignedInteger(Scalar, 0, N))
    ঘূৰাই "invalid number";
  যদি ((N > INT32_MAX) || (N < INT32_MIN))
    ঘূৰাই "out of range number";
  Val = N;
  ঘূৰাই StringRef();
}

শূন্য ScalarTraits<int64_t>::output(ধ্রুৱ int64_t &Val, শূন্য *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::প্ৰবেশ_2(StringRef Scalar, শূন্য *, int64_t &Val) {
  long long N;
  যদি (getAsSignedInteger(Scalar, 0, N))
    ঘূৰাই "invalid number";
  Val = N;
  ঘূৰাই StringRef();
}

শূন্য ScalarTraits<দ্ৱিগুণ>::output(ধ্রুৱ দ্ৱিগুণ &Val, শূন্য *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<দ্ৱিগুণ>::প্ৰবেশ_2(StringRef Scalar, শূন্য *, দ্ৱিগুণ &Val) {
  যদি (to_float(Scalar, Val))
    ঘূৰাই StringRef();
  ঘূৰাই "invalid floating point number";
}

শূন্য ScalarTraits<ভিন্ন>::output(ধ্রুৱ ভিন্ন &Val, শূন্য *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<ভিন্ন>::প্ৰবেশ_2(StringRef Scalar, শূন্য *, ভিন্ন &Val) {
  যদি (to_float(Scalar, Val))
    ঘূৰাই StringRef();
  ঘূৰাই "invalid floating point number";
}

শূন্য ScalarTraits<Hex8>::output(ধ্রুৱ Hex8 &Val, শূন্য *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::প্ৰবেশ_2(StringRef Scalar, শূন্য *, Hex8 &Val) {
  unsigned long long n;
  যদি (getAsUnsignedInteger(Scalar, 0, n))
    ঘূৰাই "invalid hex8 number";
  যদি (n > 0xFF)
    ঘূৰাই "out of range hex8 number";
  Val = n;
  ঘূৰাই StringRef();
}

শূন্য ScalarTraits<Hex16>::output(ধ্রুৱ Hex16 &Val, শূন্য *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::প্ৰবেশ_2(StringRef Scalar, শূন্য *, Hex16 &Val) {
  unsigned long long n;
  যদি (getAsUnsignedInteger(Scalar, 0, n))
    ঘূৰাই "invalid hex16 number";
  যদি (n > 0xFFFF)
    ঘূৰাই "out of range hex16 number";
  Val = n;
  ঘূৰাই StringRef();
}

শূন্য ScalarTraits<Hex32>::output(ধ্রুৱ Hex32 &Val, শূন্য *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::প্ৰবেশ_2(StringRef Scalar, শূন্য *, Hex32 &Val) {
  unsigned long long n;
  যদি (getAsUnsignedInteger(Scalar, 0, n))
    ঘূৰাই "invalid hex32 number";
  যদি (n > 0xFFFFFFFFUL)
    ঘূৰাই "out of range hex32 number";
  Val = n;
  ঘূৰাই StringRef();
}

শূন্য ScalarTraits<Hex64>::output(ধ্রুৱ Hex64 &Val, শূন্য *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::প্ৰবেশ_2(StringRef Scalar, শূন্য *, Hex64 &Val) {
  unsigned long long অঙ্ক;
  যদি (getAsUnsignedInteger(Scalar, 0, অঙ্ক))
    ঘূৰাই "invalid hex64 number";
  Val = অঙ্ক;
  ঘূৰাই StringRef();
}

শূন্য ScalarTraits<VersionTuple>::output(ধ্রুৱ VersionTuple &Val, শূন্য *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::প্ৰবেশ_2(StringRef Scalar, শূন্য *,
                                            VersionTuple &Val) {
  যদি (Val.tryParse(Scalar))
    ঘূৰাই "invalid version format";
  ঘূৰাই StringRef();
}
