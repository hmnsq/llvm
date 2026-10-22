//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/YAMLTraits.h"
#অন্তর্ভাব "llvm/ADT/STLExtras.h"
#অন্তর্ভাব "llvm/ADT/SmallString.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/ADT/Twine.h"
#অন্তর্ভাব "llvm/Support/Casting.h"
#অন্তর্ভাব "llvm/Support/Errc.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব "llvm/Support/Format.h"
#অন্তর্ভাব "llvm/Support/LineIterator.h"
#অন্তর্ভাব "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাব "llvm/Support/VersionTuple.h"
#অন্তর্ভাব "llvm/Support/YAMLParser.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <cassert>
#অন্তর্ভাব <cstdint>
#অন্তর্ভাব <cstring>
#অন্তর্ভাব <string>
#অন্তর্ভাব <vector>

using নামাকাশ llvm;
using নামাকাশ yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(শূন্য *Context) : Ctxt(Context) {}

IO::~IO() = পূর্বনির্ধারিত;

শূন্য *IO::getContext() ধ্রুবক {
  ফেরত Ctxt;
}

শূন্য IO::setContext(শূন্য *Context) {
  Ctxt = Context;
}

শূন্য IO::set_Allow_Unknown_চাবিগুলি(যৌক্তিক Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//প্রবেশ
//===----------------------------------------------------------------------===//

প্রবেশ::প্রবেশ(StringRef প্রবেশ_Content, শূন্য *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, শূন্য *DiagHandlerCtxt)
    : IO(Ctxt), Strm(নতুন Stream(প্রবেশ_Content, SrcMgr, মিথ্যা, &EC)) {
  যদি (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

প্রবেশ::প্রবেশ(Memory_বাফার_Ref প্রবেশ, শূন্য *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, শূন্য *DiagHandlerCtxt)
    : IO(Ctxt), Strm(নতুন Stream(প্রবেশ, SrcMgr, মিথ্যা, &EC)) {
  যদি (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

প্রবেশ::~প্রবেশ() = পূর্বনির্ধারিত;

std::error_code প্রবেশ::error() { ফেরত EC; }

যৌক্তিক প্রবেশ::outputting() ধ্রুবক {
  ফেরত মিথ্যা;
}

যৌক্তিক প্রবেশ::setCurrentDocument() {
  যদি (DocIterator != Strm->end()) {
    গ্রন্থি *N = DocIterator->get_মূল();
    যদি (!N) {
      EC = make_error_code(errc::invalid_argument);
      ফেরত মিথ্যা;
    }

    যদি (isa<Null_গ্রন্থি>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      ফেরত setCurrentDocument();
    }
    releaseHNodeBuffers();
    শীর্ষ_গ্রন্থি = createHNodes(N);
    Current_গ্রন্থি = শীর্ষ_গ্রন্থি;
    ফেরত সত্য;
  }
  ফেরত মিথ্যা;
}

যৌক্তিক প্রবেশ::পরবর্তী_Document() {
  ফেরত ++DocIterator != Strm->end();
}

ধ্রুবক গ্রন্থি *প্রবেশ::get_Current_গ্রন্থি() ধ্রুবক {
  ফেরত Current_গ্রন্থি ? Current_গ্রন্থি->গ্রন্থি_2 : nullptr;
}

যৌক্তিক প্রবেশ::mapTag(StringRef Tag, যৌক্তিক Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  যদি (!Current_গ্রন্থি)
    ফেরত মিথ্যা;

  std::string foundTag = Current_গ্রন্থি->গ্রন্থি_2->getVerbatimTag();
  যদি (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    ফেরত Default;
  }
  // Return true iff found tag matches supplied tag.
  ফেরত Tag == foundTag;
}

শূন্য প্রবেশ::beginMapping() {
  যদি (EC)
    ফেরত;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_গ্রন্থি);
  যদি (MN) {
    MN->Valid_চাবিগুলি.clear();
  }
}

std::vector<StringRef> প্রবেশ::চাবিগুলি() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_গ্রন্থি);
  std::vector<StringRef> Ret;
  যদি (!MN) {
    setError(Current_গ্রন্থি, "not a mapping");
    ফেরত Ret;
  }
  জন্য (auto &P : MN->Mapping)
    Ret.ঠেলো_back(P.first());
  ফেরত Ret;
}

যৌক্তিক প্রবেশ::preflight_চাবি(StringRef চাবি, যৌক্তিক Required, যৌক্তিক, যৌক্তিক &UseDefault,
                         শূন্য *&SaveInfo) {
  UseDefault = মিথ্যা;
  যদি (EC)
    ফেরত মিথ্যা;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  যদি (!Current_গ্রন্থি) {
    যদি (Required)
      EC = make_error_code(errc::invalid_argument);
    অন্যথা
      UseDefault = সত্য;
    ফেরত মিথ্যা;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_গ্রন্থি);
  যদি (!MN) {
    যদি (Required || !isa<EmptyHNode>(Current_গ্রন্থি))
      setError(Current_গ্রন্থি, "not a mapping");
    অন্যথা
      UseDefault = সত্য;
    ফেরত মিথ্যা;
  }
  MN->Valid_চাবিগুলি.ঠেলো_back(চাবি.str());
  HNode *মান = MN->Mapping[চাবি].first;
  যদি (!মান) {
    যদি (Required)
      setError(Current_গ্রন্থি, Twine("missing required key '") + চাবি + "'");
    অন্যথা
      UseDefault = সত্য;
    ফেরত মিথ্যা;
  }
  SaveInfo = Current_গ্রন্থি;
  Current_গ্রন্থি = মান;
  ফেরত সত্য;
}

শূন্য প্রবেশ::postflight_চাবি(শূন্য *saveInfo) {
  Current_গ্রন্থি = reinterpret_cast<HNode *>(saveInfo);
}

শূন্য প্রবেশ::endMapping() {
  যদি (EC)
    ফেরত;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_গ্রন্থি);
  যদি (!MN)
    ফেরত;
  জন্য (ধ্রুবক auto &NN : MN->Mapping) {
    যদি (!is_contained(MN->Valid_চাবিগুলি, NN.first())) {
      ধ্রুবক SMRange &ReportLoc = NN.second.second;
      যদি (!Allow_Unknown_চাবিগুলি) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        বিরতি;
      } অন্যথা
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

শূন্য প্রবেশ::beginFlowMapping() { beginMapping(); }

শূন্য প্রবেশ::endFlowMapping() { endMapping(); }

unsigned প্রবেশ::beginSequence() {
  যদি (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_গ্রন্থি))
    ফেরত SQ->Entries.আকার_3();
  যদি (isa<EmptyHNode>(Current_গ্রন্থি))
    ফেরত 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  যদি (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_গ্রন্থি)) {
    যদি (isNull(SN->মান_2()))
      ফেরত 0;
  }
  // Any other type of HNode is an error.
  setError(Current_গ্রন্থি, "not a sequence");
  ফেরত 0;
}

শূন্য প্রবেশ::endSequence() {
}

যৌক্তিক প্রবেশ::preflightElement(unsigned সূচক_2, শূন্য *&SaveInfo) {
  যদি (EC)
    ফেরত মিথ্যা;
  যদি (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_গ্রন্থি)) {
    SaveInfo = Current_গ্রন্থি;
    Current_গ্রন্থি = SQ->Entries[সূচক_2];
    ফেরত সত্য;
  }
  ফেরত মিথ্যা;
}

শূন্য প্রবেশ::postflightElement(শূন্য *SaveInfo) {
  Current_গ্রন্থি = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned প্রবেশ::beginFlowSequence() { ফেরত beginSequence(); }

যৌক্তিক প্রবেশ::preflightFlowElement(unsigned সূচক, শূন্য *&SaveInfo) {
  যদি (EC)
    ফেরত মিথ্যা;
  যদি (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_গ্রন্থি)) {
    SaveInfo = Current_গ্রন্থি;
    Current_গ্রন্থি = SQ->Entries[সূচক];
    ফেরত সত্য;
  }
  ফেরত মিথ্যা;
}

শূন্য প্রবেশ::postflightFlowElement(শূন্য *SaveInfo) {
  Current_গ্রন্থি = reinterpret_cast<HNode *>(SaveInfo);
}

শূন্য প্রবেশ::endFlowSequence() {
}

শূন্য প্রবেশ::beginEnumScalar() {
  ScalarMatchFound = মিথ্যা;
}

যৌক্তিক প্রবেশ::matchEnumScalar(StringRef Str, যৌক্তিক) {
  যদি (ScalarMatchFound)
    ফেরত মিথ্যা;
  যদি (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_গ্রন্থি)) {
    যদি (SN->মান_2() == Str) {
      ScalarMatchFound = সত্য;
      ফেরত সত্য;
    }
  }
  ফেরত মিথ্যা;
}

যৌক্তিক প্রবেশ::matchEnumFallback() {
  যদি (ScalarMatchFound)
    ফেরত মিথ্যা;
  ScalarMatchFound = সত্য;
  ফেরত সত্য;
}

শূন্য প্রবেশ::endEnumScalar() {
  যদি (!ScalarMatchFound) {
    setError(Current_গ্রন্থি, "unknown enumerated scalar");
  }
}

যৌক্তিক প্রবেশ::beginBitSetScalar(যৌক্তিক &DoClear) {
  Bit_মানসমূহ_Used.clear();
  যদি (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_গ্রন্থি)) {
    Bit_মানসমূহ_Used.resize(SQ->Entries.আকার_3());
  } অন্যথা {
    setError(Current_গ্রন্থি, "expected sequence of bit values");
  }
  DoClear = সত্য;
  ফেরত সত্য;
}

যৌক্তিক প্রবেশ::bitSetMatch(StringRef Str, যৌক্তিক) {
  যদি (EC)
    ফেরত মিথ্যা;
  যদি (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_গ্রন্থি)) {
    unsigned সূচক_2 = 0;
    জন্য (auto &N : SQ->Entries) {
      যদি (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        যদি (SN->মান_2() == Str) {
          Bit_মানসমূহ_Used[সূচক_2] = সত্য;
          ফেরত সত্য;
        }
      } অন্যথা {
        setError(Current_গ্রন্থি, "unexpected scalar in sequence of bit values");
      }
      ++সূচক_2;
    }
  } অন্যথা {
    setError(Current_গ্রন্থি, "expected sequence of bit values");
  }
  ফেরত মিথ্যা;
}

শূন্য প্রবেশ::endBitSetScalar() {
  যদি (EC)
    ফেরত;
  যদি (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_গ্রন্থি)) {
    প্রতিজ্ঞা(Bit_মানসমূহ_Used.আকার_3() == SQ->Entries.আকার_3());
    জন্য (unsigned i = 0; i < SQ->Entries.আকার_3(); ++i) {
      যদি (!Bit_মানসমূহ_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        ফেরত;
      }
    }
  }
}

শূন্য প্রবেশ::scalarString(StringRef &S, QuotingType) {
  যদি (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_গ্রন্থি)) {
    S = SN->মান_2();
  } অন্যথা {
    setError(Current_গ্রন্থি, "unexpected scalar");
  }
}

শূন্য প্রবেশ::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

শূন্য প্রবেশ::scalarTag(std::string &Tag) {
  Tag = Current_গ্রন্থি->গ্রন্থি_2->getVerbatimTag();
}

শূন্য প্রবেশ::setError(HNode *hnode, ধ্রুবক Twine &message) {
  প্রতিজ্ঞা(hnode && "HNode must not be NULL");
  setError(hnode->গ্রন্থি_2, message);
}

গ্রন্থি_Kind প্রবেশ::get_গ্রন্থি_Kind() {
  যদি (isa<ScalarHNode>(Current_গ্রন্থি))
    ফেরত গ্রন্থি_Kind::Scalar;
  অন্যথা যদি (isa<MapHNode>(Current_গ্রন্থি))
    ফেরত গ্রন্থি_Kind::Map;
  অন্যথা যদি (isa<SequenceHNode>(Current_গ্রন্থি))
    ফেরত গ্রন্থি_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

শূন্য প্রবেশ::setError(গ্রন্থি *গ্রন্থি_3, ধ্রুবক Twine &message) {
  Strm->printError(গ্রন্থি_3, message);
  EC = make_error_code(errc::invalid_argument);
}

শূন্য প্রবেশ::setError(ধ্রুবক SMRange &range, ধ্রুবক Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

শূন্য প্রবেশ::reportWarning(HNode *hnode, ধ্রুবক Twine &message) {
  প্রতিজ্ঞা(hnode && "HNode must not be NULL");
  Strm->printError(hnode->গ্রন্থি_2, message, SourceMgr::DK_Warning);
}

শূন্য প্রবেশ::reportWarning(গ্রন্থি *গ্রন্থি_3, ধ্রুবক Twine &message) {
  Strm->printError(গ্রন্থি_3, message, SourceMgr::DK_Warning);
}

শূন্য প্রবেশ::reportWarning(ধ্রুবক SMRange &range, ধ্রুবক Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

শূন্য প্রবেশ::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

শূন্য প্রবেশ::saveAliasHNode(গ্রন্থি *N, HNode *HN) {
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

প্রবেশ::HNode *প্রবেশ::createHNodes(গ্রন্থি *N) {
  SmallString<128> StringStorage;
  নির্বাচন (N->getType()) {
  ক্ষেত্র গ্রন্থি::NK_Scalar: {
    Scalar_গ্রন্থি *SN = dyn_cast<Scalar_গ্রন্থি>(N);
    StringRef চাবি_Str = SN->get_মান(StringStorage);
    যদি (!StringStorage.empty()) {
      // Copy string to permanent storage
      চাবি_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = নতুন (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, চাবি_Str);
    saveAliasHNode(SN, SHNode);
    ফেরত SHNode;
  }
  ক্ষেত্র গ্রন্থি::NK_BlockScalar: {
    Block_Scalar_গ্রন্থি *BSN = dyn_cast<Block_Scalar_গ্রন্থি>(N);
    StringRef মান_Copy = BSN->get_মান().copy(StringAllocator);
    auto *BSHNode =
        নতুন (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, মান_Copy);
    saveAliasHNode(BSN, BSHNode);
    ফেরত BSHNode;
  }
  ক্ষেত্র গ্রন্থি::NK_Sequence: {
    Sequence_গ্রন্থি *SQ = dyn_cast<Sequence_গ্রন্থি>(N);
    auto SQHNode = নতুন (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    জন্য (গ্রন্থি &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      যদি (EC)
        বিরতি;
      SQHNode->Entries.ঠেলো_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    ফেরত SQHNode;
  }
  ক্ষেত্র গ্রন্থি::NK_Mapping: {
    Mapping_গ্রন্থি *Map = dyn_cast<Mapping_গ্রন্থি>(N);
    auto mapHNode = নতুন (MapHNodeAllocator.Allocate()) MapHNode(N);
    জন্য (চাবি_মান_গ্রন্থি &KVN : *Map) {
      গ্রন্থি *চাবি_গ্রন্থি = KVN.get_চাবি();
      Scalar_গ্রন্থি *চাবি = dyn_cast_if_present<Scalar_গ্রন্থি>(চাবি_গ্রন্থি);
      গ্রন্থি *মান = KVN.get_মান();
      যদি (!চাবি || !মান) {
        যদি (!চাবি)
          setError(চাবি_গ্রন্থি, "Map key must be a scalar");
        যদি (!মান)
          setError(চাবি_গ্রন্থি, "Map value must not be empty");
        বিরতি;
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
        setError(চাবি_গ্রন্থি, Twine("duplicated mapping key '") + চাবি_Str + "'");
      auto মান_HNode = createHNodes(মান);
      যদি (EC)
        বিরতি;
      mapHNode->Mapping[চাবি_Str] =
          std::make_pair(std::move(মান_HNode), চাবি_গ্রন্থি->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    ফেরত std::move(mapHNode);
  }
  ক্ষেত্র গ্রন্থি::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    ফেরত নতুন (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  ক্ষেত্র গ্রন্থি::NK_Alias: {
    Alias_গ্রন্থি *AN = dyn_cast<Alias_গ্রন্থি>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    যদি (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      ফেরত nullptr;
    }
    ফেরত AHN->second;
  }
  পূর্বনির্ধারিত:
    setError(N, "unknown node kind");
    ফেরত nullptr;
  }
}

শূন্য প্রবেশ::setError(ধ্রুবক Twine &Message) {
  setError(Current_গ্রন্থি, Message);
}

শূন্য প্রবেশ::set_Allow_Unknown_চাবিগুলি(যৌক্তিক Allow) { Allow_Unknown_চাবিগুলি = Allow; }

যৌক্তিক প্রবেশ::canElideEmptySequence() {
  ফেরত মিথ্যা;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, শূন্য *context, পূর্ণসংখ্যা WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = পূর্বনির্ধারিত;

যৌক্তিক Output::outputting() ধ্রুবক {
  ফেরত সত্য;
}

শূন্য Output::beginMapping() {
  State_স্তূপ.ঠেলো_back(in_Map_First_চাবি);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

যৌক্তিক Output::mapTag(StringRef Tag, যৌক্তিক Use) {
  যদি (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    যৌক্তিক SequenceElement = মিথ্যা;
    যদি (State_স্তূপ.আকার_3() > 1) {
      auto &E = State_স্তূপ[State_স্তূপ.আকার_3() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    যদি (SequenceElement && State_স্তূপ.back() == in_Map_First_চাবি) {
      new_সারি_Check();
    } অন্যথা {
      output(" ");
    }
    output(Tag);
    যদি (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      যদি (State_স্তূপ.back() == in_Map_First_চাবি) {
        State_স্তূপ.বের_করো_back();
        State_স্তূপ.ঠেলো_back(in_Map_Other_চাবি);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  ফেরত Use;
}

শূন্য Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  যদি (State_স্তূপ.back() == in_Map_First_চাবি) {
    Padding = PaddingBeforeContainer;
    new_সারি_Check();
    output("{}");
    Padding = "\n";
  }
  State_স্তূপ.বের_করো_back();
}

std::vector<StringRef> Output::চাবিগুলি() {
  report_fatal_error("invalid call");
}

যৌক্তিক Output::preflight_চাবি(StringRef চাবি, যৌক্তিক Required, যৌক্তিক SameAsDefault,
                          যৌক্তিক &UseDefault, শূন্য *&SaveInfo) {
  UseDefault = মিথ্যা;
  SaveInfo = nullptr;
  যদি (Required || !SameAsDefault || Write_Default_মানসমূহ) {
    auto State = State_স্তূপ.back();
    যদি (State == in_Flow_Map_First_চাবি || State == in_Flow_Map_Other_চাবি) {
      flow_চাবি(চাবি);
    } অন্যথা {
      new_সারি_Check();
      padded_চাবি(চাবি);
    }
    ফেরত সত্য;
  }
  ফেরত মিথ্যা;
}

শূন্য Output::postflight_চাবি(শূন্য *) {
  যদি (State_স্তূপ.back() == in_Map_First_চাবি) {
    State_স্তূপ.বের_করো_back();
    State_স্তূপ.ঠেলো_back(in_Map_Other_চাবি);
  } অন্যথা যদি (State_স্তূপ.back() == in_Flow_Map_First_চাবি) {
    State_স্তূপ.বের_করো_back();
    State_স্তূপ.ঠেলো_back(in_Flow_Map_Other_চাবি);
  }
}

শূন্য Output::beginFlowMapping() {
  State_স্তূপ.ঠেলো_back(in_Flow_Map_First_চাবি);
  new_সারি_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

শূন্য Output::endFlowMapping() {
  State_স্তূপ.বের_করো_back();
  output_Up_To_End_Of_সারি(" }");
}

শূন্য Output::beginDocuments() {
  output_Up_To_End_Of_সারি("---");
}

যৌক্তিক Output::preflightDocument(unsigned সূচক) {
  যদি (সূচক > 0)
    output_Up_To_End_Of_সারি("\n---");
  ফেরত সত্য;
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
  ফেরত 0;
}

শূন্য Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  যদি (State_স্তূপ.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_সারি_Check(/*EmptySequence=*/সত্য);
    output("[]");
    Padding = "\n";
  }
  State_স্তূপ.বের_করো_back();
}

যৌক্তিক Output::preflightElement(unsigned, শূন্য *&SaveInfo) {
  SaveInfo = nullptr;
  ফেরত সত্য;
}

শূন্য Output::postflightElement(শূন্য *) {
  যদি (State_স্তূপ.back() == inSeqFirstElement) {
    State_স্তূপ.বের_করো_back();
    State_স্তূপ.ঠেলো_back(inSeqOtherElement);
  } অন্যথা যদি (State_স্তূপ.back() == inFlowSeqFirstElement) {
    State_স্তূপ.বের_করো_back();
    State_স্তূপ.ঠেলো_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_স্তূপ.ঠেলো_back(inFlowSeqFirstElement);
  new_সারি_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = মিথ্যা;
  ফেরত 0;
}

শূন্য Output::endFlowSequence() {
  State_স্তূপ.বের_করো_back();
  output_Up_To_End_Of_সারি(" ]");
}

যৌক্তিক Output::preflightFlowElement(unsigned, শূন্য *&SaveInfo) {
  যদি (NeedFlowSequenceComma)
    output(", ");
  যদি (WrapColumn && Column > WrapColumn) {
    output("\n");
    জন্য (পূর্ণসংখ্যা i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  ফেরত সত্য;
}

শূন্য Output::postflightFlowElement(শূন্য *) {
  NeedFlowSequenceComma = সত্য;
}

শূন্য Output::beginEnumScalar() {
  EnumerationMatchFound = মিথ্যা;
}

যৌক্তিক Output::matchEnumScalar(StringRef Str, যৌক্তিক Match) {
  যদি (Match && !EnumerationMatchFound) {
    new_সারি_Check();
    output_Up_To_End_Of_সারি(Str);
    EnumerationMatchFound = সত্য;
  }
  ফেরত মিথ্যা;
}

যৌক্তিক Output::matchEnumFallback() {
  যদি (EnumerationMatchFound)
    ফেরত মিথ্যা;
  EnumerationMatchFound = সত্য;
  ফেরত সত্য;
}

শূন্য Output::endEnumScalar() {
  যদি (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

যৌক্তিক Output::beginBitSetScalar(যৌক্তিক &DoClear) {
  new_সারি_Check();
  output("[ ");
  Need_Bit_মান_Comma = মিথ্যা;
  DoClear = মিথ্যা;
  ফেরত সত্য;
}

যৌক্তিক Output::bitSetMatch(StringRef Str, যৌক্তিক Matches) {
  যদি (Matches) {
    যদি (Need_Bit_মান_Comma)
      output(", ");
    output(Str);
    Need_Bit_মান_Comma = সত্য;
  }
  ফেরত মিথ্যা;
}

শূন্য Output::endBitSetScalar() {
  output_Up_To_End_Of_সারি(" ]");
}

শূন্য Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_সারি_Check();
  যদি (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_সারি("''");
    ফেরত;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_সারি("");
}

শূন্য Output::blockScalarString(StringRef &S) {
  যদি (!State_স্তূপ.empty())
    new_সারি_Check();
  output(" |");

  unsigned Indent = State_স্তূপ.empty() ? 1 : State_স্তূপ.আকার_3();

  auto বাফার = Memory_বাফার::get_Mem_বাফার(S, "", মিথ্যা);
  জন্য (সারি_iterator সারিগুলি(*বাফার, মিথ্যা); !সারিগুলি.is_at_end(); ++সারিগুলি) {
    output_New_সারি();
    জন্য (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*সারিগুলি);
  }
  output_Up_To_End_Of_সারি("");
}

শূন্য Output::scalarTag(std::string &Tag) {
  যদি (Tag.empty())
    ফেরত;
  new_সারি_Check();
  output(Tag);
  output(" ");
}

শূন্য Output::setError(ধ্রুবক Twine &message) {
}

std::error_code Output::error() { ফেরত {}; }

যৌক্তিক Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  যদি (State_স্তূপ.আকার_3() < 2)
    ফেরত সত্য;
  যদি (State_স্তূপ.back() != in_Map_First_চাবি)
    ফেরত সত্য;
  ফেরত !inSeqAnyElement(State_স্তূপ[State_স্তূপ.আকার_3() - 2]);
}

শূন্য Output::output(StringRef s) {
  Column += s.আকার_3();
  Out << s;
}

শূন্য Output::output(StringRef S, QuotingType MustQuote) {
  যদি (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    ফেরত;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  যদি (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ মিথ্যা));
    output(Quote);
    ফেরত;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.আকার_3();
  ধ্রুবক অক্ষর *Base = S.তথ্য();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  যখন (j < End) {
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

শূন্য Output::output_Up_To_End_Of_সারি(StringRef s) {
  output(s);
  যদি (State_স্তূপ.empty() || (!inFlowSeqAnyElement(State_স্তূপ.back()) &&
                             !in_Flow_Map_Any_চাবি(State_স্তূপ.back())))
    Padding = "\n";
}

শূন্য Output::output_New_সারি() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

শূন্য Output::new_সারি_Check(যৌক্তিক EmptySequence) {
  যদি (Padding != "\n") {
    output(Padding);
    Padding = {};
    ফেরত;
  }
  output_New_সারি();
  Padding = {};

  যদি (State_স্তূপ.আকার_3() == 0 || EmptySequence)
    ফেরত;

  unsigned Indent = State_স্তূপ.আকার_3() - 1;
  যৌক্তিক PossiblyNestedSeq = মিথ্যা;
  auto I = State_স্তূপ.rbegin(), E = State_স্তূপ.rend();

  যদি (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = সত্য; // Not possibly but always.
    ++Indent;
  } অন্যথা যদি (*I == in_Map_First_চাবি || *I == in_Flow_Map_First_চাবি ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = সত্য;
    ++I; // Skip back().
  }

  unsigned Output_Dash_গুনতি = 0;
  যদি (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    যখন (I != E) {
      // Don't count the top of nested sequence.
      যদি (!inSeqAnyElement(*I))
        বিরতি;

      ++Output_Dash_গুনতি;

      // Stop counting if consecutive inSeqFirstElement ends.
      যদি (*I++ != inSeqFirstElement)
        বিরতি;
    }
  }

  জন্য (unsigned I = Output_Dash_গুনতি; I < Indent; ++I)
    output("  ");

  জন্য (unsigned I = 0; I < Output_Dash_গুনতি; ++I)
    output("- ");
}

শূন্য Output::padded_চাবি(StringRef চাবি_2) {
  output(চাবি_2, needsQuotes(চাবি_2, মিথ্যা));
  output(":");
  ধ্রুবক অক্ষর *spaces = "                ";
  যদি (চাবি_2.আকার_3() < strlen(spaces))
    Padding = &spaces[চাবি_2.আকার_3()];
  অন্যথা
    Padding = " ";
}

শূন্য Output::flow_চাবি(StringRef চাবি) {
  যদি (State_স্তূপ.back() == in_Flow_Map_Other_চাবি)
    output(", ");
  যদি (WrapColumn && Column > WrapColumn) {
    output("\n");
    জন্য (পূর্ণসংখ্যা I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(চাবি, needsQuotes(চাবি, মিথ্যা));
  output(": ");
}

গ্রন্থি_Kind Output::get_গ্রন্থি_Kind() { report_fatal_error("invalid call"); }

যৌক্তিক Output::inSeqAnyElement(InState State) {
  ফেরত State == inSeqFirstElement || State == inSeqOtherElement;
}

যৌক্তিক Output::inFlowSeqAnyElement(InState State) {
  ফেরত State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

যৌক্তিক Output::in_Map_Any_চাবি(InState State) {
  ফেরত State == in_Map_First_চাবি || State == in_Map_Other_চাবি;
}

যৌক্তিক Output::in_Flow_Map_Any_চাবি(InState State) {
  ফেরত State == in_Flow_Map_First_চাবি || State == in_Flow_Map_Other_চাবি;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

শূন্য ScalarTraits<যৌক্তিক>::output(ধ্রুবক যৌক্তিক &Val, শূন্য *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<যৌক্তিক>::প্রবেশ_2(StringRef Scalar, শূন্য *, যৌক্তিক &Val) {
  যদি (std::optional<যৌক্তিক> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    ফেরত StringRef();
  }
  ফেরত "invalid boolean";
}

শূন্য ScalarTraits<StringRef>::output(ধ্রুবক StringRef &Val, শূন্য *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::প্রবেশ_2(StringRef Scalar, শূন্য *,
                                         StringRef &Val) {
  Val = Scalar;
  ফেরত StringRef();
}

শূন্য ScalarTraits<std::string>::output(ধ্রুবক std::string &Val, শূন্য *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::প্রবেশ_2(StringRef Scalar, শূন্য *,
                                           std::string &Val) {
  Val = Scalar.str();
  ফেরত StringRef();
}

শূন্য ScalarTraits<uint8_t>::output(ধ্রুবক uint8_t &Val, শূন্য *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t অঙ্ক = Val;
  Out << অঙ্ক;
}

StringRef ScalarTraits<uint8_t>::প্রবেশ_2(StringRef Scalar, শূন্য *, uint8_t &Val) {
  unsigned long long n;
  যদি (getAsUnsignedInteger(Scalar, 0, n))
    ফেরত "invalid number";
  যদি (n > 0xFF)
    ফেরত "out of range number";
  Val = n;
  ফেরত StringRef();
}

শূন্য ScalarTraits<uint16_t>::output(ধ্রুবক uint16_t &Val, শূন্য *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::প্রবেশ_2(StringRef Scalar, শূন্য *,
                                        uint16_t &Val) {
  unsigned long long n;
  যদি (getAsUnsignedInteger(Scalar, 0, n))
    ফেরত "invalid number";
  যদি (n > 0xFFFF)
    ফেরত "out of range number";
  Val = n;
  ফেরত StringRef();
}

শূন্য ScalarTraits<uint32_t>::output(ধ্রুবক uint32_t &Val, শূন্য *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::প্রবেশ_2(StringRef Scalar, শূন্য *,
                                        uint32_t &Val) {
  unsigned long long n;
  যদি (getAsUnsignedInteger(Scalar, 0, n))
    ফেরত "invalid number";
  যদি (n > 0xFFFFFFFFUL)
    ফেরত "out of range number";
  Val = n;
  ফেরত StringRef();
}

শূন্য ScalarTraits<uint64_t>::output(ধ্রুবক uint64_t &Val, শূন্য *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::প্রবেশ_2(StringRef Scalar, শূন্য *,
                                        uint64_t &Val) {
  unsigned long long N;
  যদি (getAsUnsignedInteger(Scalar, 0, N))
    ফেরত "invalid number";
  Val = N;
  ফেরত StringRef();
}

শূন্য ScalarTraits<int8_t>::output(ধ্রুবক int8_t &Val, শূন্য *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t অঙ্ক = Val;
  Out << অঙ্ক;
}

StringRef ScalarTraits<int8_t>::প্রবেশ_2(StringRef Scalar, শূন্য *, int8_t &Val) {
  long long N;
  যদি (getAsSignedInteger(Scalar, 0, N))
    ফেরত "invalid number";
  যদি ((N > 127) || (N < -128))
    ফেরত "out of range number";
  Val = N;
  ফেরত StringRef();
}

শূন্য ScalarTraits<int16_t>::output(ধ্রুবক int16_t &Val, শূন্য *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::প্রবেশ_2(StringRef Scalar, শূন্য *, int16_t &Val) {
  long long N;
  যদি (getAsSignedInteger(Scalar, 0, N))
    ফেরত "invalid number";
  যদি ((N > INT16_MAX) || (N < INT16_MIN))
    ফেরত "out of range number";
  Val = N;
  ফেরত StringRef();
}

শূন্য ScalarTraits<int32_t>::output(ধ্রুবক int32_t &Val, শূন্য *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::প্রবেশ_2(StringRef Scalar, শূন্য *, int32_t &Val) {
  long long N;
  যদি (getAsSignedInteger(Scalar, 0, N))
    ফেরত "invalid number";
  যদি ((N > INT32_MAX) || (N < INT32_MIN))
    ফেরত "out of range number";
  Val = N;
  ফেরত StringRef();
}

শূন্য ScalarTraits<int64_t>::output(ধ্রুবক int64_t &Val, শূন্য *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::প্রবেশ_2(StringRef Scalar, শূন্য *, int64_t &Val) {
  long long N;
  যদি (getAsSignedInteger(Scalar, 0, N))
    ফেরত "invalid number";
  Val = N;
  ফেরত StringRef();
}

শূন্য ScalarTraits<দ্বিগুণ>::output(ধ্রুবক দ্বিগুণ &Val, শূন্য *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<দ্বিগুণ>::প্রবেশ_2(StringRef Scalar, শূন্য *, দ্বিগুণ &Val) {
  যদি (to_float(Scalar, Val))
    ফেরত StringRef();
  ফেরত "invalid floating point number";
}

শূন্য ScalarTraits<দশমিক>::output(ধ্রুবক দশমিক &Val, শূন্য *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<দশমিক>::প্রবেশ_2(StringRef Scalar, শূন্য *, দশমিক &Val) {
  যদি (to_float(Scalar, Val))
    ফেরত StringRef();
  ফেরত "invalid floating point number";
}

শূন্য ScalarTraits<Hex8>::output(ধ্রুবক Hex8 &Val, শূন্য *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::প্রবেশ_2(StringRef Scalar, শূন্য *, Hex8 &Val) {
  unsigned long long n;
  যদি (getAsUnsignedInteger(Scalar, 0, n))
    ফেরত "invalid hex8 number";
  যদি (n > 0xFF)
    ফেরত "out of range hex8 number";
  Val = n;
  ফেরত StringRef();
}

শূন্য ScalarTraits<Hex16>::output(ধ্রুবক Hex16 &Val, শূন্য *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::প্রবেশ_2(StringRef Scalar, শূন্য *, Hex16 &Val) {
  unsigned long long n;
  যদি (getAsUnsignedInteger(Scalar, 0, n))
    ফেরত "invalid hex16 number";
  যদি (n > 0xFFFF)
    ফেরত "out of range hex16 number";
  Val = n;
  ফেরত StringRef();
}

শূন্য ScalarTraits<Hex32>::output(ধ্রুবক Hex32 &Val, শূন্য *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::প্রবেশ_2(StringRef Scalar, শূন্য *, Hex32 &Val) {
  unsigned long long n;
  যদি (getAsUnsignedInteger(Scalar, 0, n))
    ফেরত "invalid hex32 number";
  যদি (n > 0xFFFFFFFFUL)
    ফেরত "out of range hex32 number";
  Val = n;
  ফেরত StringRef();
}

শূন্য ScalarTraits<Hex64>::output(ধ্রুবক Hex64 &Val, শূন্য *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::প্রবেশ_2(StringRef Scalar, শূন্য *, Hex64 &Val) {
  unsigned long long অঙ্ক;
  যদি (getAsUnsignedInteger(Scalar, 0, অঙ্ক))
    ফেরত "invalid hex64 number";
  Val = অঙ্ক;
  ফেরত StringRef();
}

শূন্য ScalarTraits<VersionTuple>::output(ধ্রুবক VersionTuple &Val, শূন্য *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::প্রবেশ_2(StringRef Scalar, শূন্য *,
                                            VersionTuple &Val) {
  যদি (Val.tryParse(Scalar))
    ফেরত "invalid version format";
  ফেরত StringRef();
}
