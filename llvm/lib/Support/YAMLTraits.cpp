//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/YAMLTraits.h"
#அந்தர்பாவ "llvm/ADT/STLExtras.h"
#அந்தர்பாவ "llvm/ADT/SmallString.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/ADT/Twine.h"
#அந்தர்பாவ "llvm/Support/Casting.h"
#அந்தர்பாவ "llvm/Support/Errc.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ "llvm/Support/Format.h"
#அந்தர்பாவ "llvm/Support/LineIterator.h"
#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"
#அந்தர்பாவ "llvm/Support/VersionTuple.h"
#அந்தர்பாவ "llvm/Support/YAMLParser.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <cassert>
#அந்தர்பாவ <cstdint>
#அந்தர்பாவ <cstring>
#அந்தர்பாவ <string>
#அந்தர்பாவ <vector>

using நாமாகாஶ llvm;
using நாமாகாஶ yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(வெறுமை *Context) : Ctxt(Context) {}

IO::~IO() = இயல்பு;

வெறுமை *IO::getContext() மாறிலி {
  திரும்பு Ctxt;
}

வெறுமை IO::setContext(வெறுமை *Context) {
  Ctxt = Context;
}

வெறுமை IO::set_Allow_Unknown_சாவிகள்(தருக்க Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//உள்ளீடு
//===----------------------------------------------------------------------===//

உள்ளீடு::உள்ளீடு(StringRef உள்ளீடு_Content, வெறுமை *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, வெறுமை *DiagHandlerCtxt)
    : IO(Ctxt), Strm(புதிய Stream(உள்ளீடு_Content, SrcMgr, பொய், &EC)) {
  எனில் (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

உள்ளீடு::உள்ளீடு(MemoryBufferRef உள்ளீடு, வெறுமை *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, வெறுமை *DiagHandlerCtxt)
    : IO(Ctxt), Strm(புதிய Stream(உள்ளீடு, SrcMgr, பொய், &EC)) {
  எனில் (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

உள்ளீடு::~உள்ளீடு() = இயல்பு;

std::error_code உள்ளீடு::error() { திரும்பு EC; }

தருக்க உள்ளீடு::outputting() மாறிலி {
  திரும்பு பொய்;
}

தருக்க உள்ளீடு::setCurrentDocument() {
  எனில் (DocIterator != Strm->end()) {
    முனை *N = DocIterator->get_வேர்();
    எனில் (!N) {
      EC = make_error_code(errc::invalid_argument);
      திரும்பு பொய்;
    }

    எனில் (isa<Null_முனை>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      திரும்பு setCurrentDocument();
    }
    releaseHNodeBuffers();
    உச்சி_முனை = createHNodes(N);
    Current_முனை = உச்சி_முனை;
    திரும்பு உண்மை;
  }
  திரும்பு பொய்;
}

தருக்க உள்ளீடு::nextDocument() {
  திரும்பு ++DocIterator != Strm->end();
}

மாறிலி முனை *உள்ளீடு::get_Current_முனை() மாறிலி {
  திரும்பு Current_முனை ? Current_முனை->முனை_2 : nullptr;
}

தருக்க உள்ளீடு::mapTag(StringRef Tag, தருக்க Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  எனில் (!Current_முனை)
    திரும்பு பொய்;

  std::string foundTag = Current_முனை->முனை_2->getVerbatimTag();
  எனில் (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    திரும்பு Default;
  }
  // Return true iff found tag matches supplied tag.
  திரும்பு Tag == foundTag;
}

வெறுமை உள்ளீடு::beginMapping() {
  எனில் (EC)
    திரும்பு;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_முனை);
  எனில் (MN) {
    MN->Valid_சாவிகள்.clear();
  }
}

std::vector<StringRef> உள்ளீடு::சாவிகள்() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_முனை);
  std::vector<StringRef> Ret;
  எனில் (!MN) {
    setError(Current_முனை, "not a mapping");
    திரும்பு Ret;
  }
  க்கு (auto &P : MN->Mapping)
    Ret.தள்ளு_back(P.first());
  திரும்பு Ret;
}

தருக்க உள்ளீடு::preflight_சாவி(StringRef சாவி, தருக்க Required, தருக்க, தருக்க &UseDefault,
                         வெறுமை *&SaveInfo) {
  UseDefault = பொய்;
  எனில் (EC)
    திரும்பு பொய்;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  எனில் (!Current_முனை) {
    எனில் (Required)
      EC = make_error_code(errc::invalid_argument);
    இல்லையெனில்
      UseDefault = உண்மை;
    திரும்பு பொய்;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_முனை);
  எனில் (!MN) {
    எனில் (Required || !isa<EmptyHNode>(Current_முனை))
      setError(Current_முனை, "not a mapping");
    இல்லையெனில்
      UseDefault = உண்மை;
    திரும்பு பொய்;
  }
  MN->Valid_சாவிகள்.தள்ளு_back(சாவி.str());
  HNode *மதிப்பு = MN->Mapping[சாவி].first;
  எனில் (!மதிப்பு) {
    எனில் (Required)
      setError(Current_முனை, Twine("missing required key '") + சாவி + "'");
    இல்லையெனில்
      UseDefault = உண்மை;
    திரும்பு பொய்;
  }
  SaveInfo = Current_முனை;
  Current_முனை = மதிப்பு;
  திரும்பு உண்மை;
}

வெறுமை உள்ளீடு::postflight_சாவி(வெறுமை *saveInfo) {
  Current_முனை = reinterpret_cast<HNode *>(saveInfo);
}

வெறுமை உள்ளீடு::endMapping() {
  எனில் (EC)
    திரும்பு;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_முனை);
  எனில் (!MN)
    திரும்பு;
  க்கு (மாறிலி auto &NN : MN->Mapping) {
    எனில் (!is_contained(MN->Valid_சாவிகள், NN.first())) {
      மாறிலி SMRange &ReportLoc = NN.second.second;
      எனில் (!Allow_Unknown_சாவிகள்) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        நிறுத்து;
      } இல்லையெனில்
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

வெறுமை உள்ளீடு::beginFlowMapping() { beginMapping(); }

வெறுமை உள்ளீடு::endFlowMapping() { endMapping(); }

unsigned உள்ளீடு::beginSequence() {
  எனில் (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_முனை))
    திரும்பு SQ->Entries.size();
  எனில் (isa<EmptyHNode>(Current_முனை))
    திரும்பு 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  எனில் (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_முனை)) {
    எனில் (isNull(SN->மதிப்பு_2()))
      திரும்பு 0;
  }
  // Any other type of HNode is an error.
  setError(Current_முனை, "not a sequence");
  திரும்பு 0;
}

வெறுமை உள்ளீடு::endSequence() {
}

தருக்க உள்ளீடு::preflightElement(unsigned Index, வெறுமை *&SaveInfo) {
  எனில் (EC)
    திரும்பு பொய்;
  எனில் (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_முனை)) {
    SaveInfo = Current_முனை;
    Current_முனை = SQ->Entries[Index];
    திரும்பு உண்மை;
  }
  திரும்பு பொய்;
}

வெறுமை உள்ளீடு::postflightElement(வெறுமை *SaveInfo) {
  Current_முனை = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned உள்ளீடு::beginFlowSequence() { திரும்பு beginSequence(); }

தருக்க உள்ளீடு::preflightFlowElement(unsigned index, வெறுமை *&SaveInfo) {
  எனில் (EC)
    திரும்பு பொய்;
  எனில் (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_முனை)) {
    SaveInfo = Current_முனை;
    Current_முனை = SQ->Entries[index];
    திரும்பு உண்மை;
  }
  திரும்பு பொய்;
}

வெறுமை உள்ளீடு::postflightFlowElement(வெறுமை *SaveInfo) {
  Current_முனை = reinterpret_cast<HNode *>(SaveInfo);
}

வெறுமை உள்ளீடு::endFlowSequence() {
}

வெறுமை உள்ளீடு::beginEnumScalar() {
  ScalarMatchFound = பொய்;
}

தருக்க உள்ளீடு::matchEnumScalar(StringRef Str, தருக்க) {
  எனில் (ScalarMatchFound)
    திரும்பு பொய்;
  எனில் (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_முனை)) {
    எனில் (SN->மதிப்பு_2() == Str) {
      ScalarMatchFound = உண்மை;
      திரும்பு உண்மை;
    }
  }
  திரும்பு பொய்;
}

தருக்க உள்ளீடு::matchEnumFallback() {
  எனில் (ScalarMatchFound)
    திரும்பு பொய்;
  ScalarMatchFound = உண்மை;
  திரும்பு உண்மை;
}

வெறுமை உள்ளீடு::endEnumScalar() {
  எனில் (!ScalarMatchFound) {
    setError(Current_முனை, "unknown enumerated scalar");
  }
}

தருக்க உள்ளீடு::beginBitSetScalar(தருக்க &DoClear) {
  Bit_மதிப்புகள்_Used.clear();
  எனில் (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_முனை)) {
    Bit_மதிப்புகள்_Used.resize(SQ->Entries.size());
  } இல்லையெனில் {
    setError(Current_முனை, "expected sequence of bit values");
  }
  DoClear = உண்மை;
  திரும்பு உண்மை;
}

தருக்க உள்ளீடு::bitSetMatch(StringRef Str, தருக்க) {
  எனில் (EC)
    திரும்பு பொய்;
  எனில் (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_முனை)) {
    unsigned Index = 0;
    க்கு (auto &N : SQ->Entries) {
      எனில் (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        எனில் (SN->மதிப்பு_2() == Str) {
          Bit_மதிப்புகள்_Used[Index] = உண்மை;
          திரும்பு உண்மை;
        }
      } இல்லையெனில் {
        setError(Current_முனை, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } இல்லையெனில் {
    setError(Current_முனை, "expected sequence of bit values");
  }
  திரும்பு பொய்;
}

வெறுமை உள்ளீடு::endBitSetScalar() {
  எனில் (EC)
    திரும்பு;
  எனில் (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_முனை)) {
    ப்ரதிஜ்ஞா(Bit_மதிப்புகள்_Used.size() == SQ->Entries.size());
    க்கு (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      எனில் (!Bit_மதிப்புகள்_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        திரும்பு;
      }
    }
  }
}

வெறுமை உள்ளீடு::scalarString(StringRef &S, QuotingType) {
  எனில் (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_முனை)) {
    S = SN->மதிப்பு_2();
  } இல்லையெனில் {
    setError(Current_முனை, "unexpected scalar");
  }
}

வெறுமை உள்ளீடு::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

வெறுமை உள்ளீடு::scalarTag(std::string &Tag) {
  Tag = Current_முனை->முனை_2->getVerbatimTag();
}

வெறுமை உள்ளீடு::setError(HNode *hnode, மாறிலி Twine &message) {
  ப்ரதிஜ்ஞா(hnode && "HNode must not be NULL");
  setError(hnode->முனை_2, message);
}

முனை_Kind உள்ளீடு::get_முனை_Kind() {
  எனில் (isa<ScalarHNode>(Current_முனை))
    திரும்பு முனை_Kind::Scalar;
  இல்லையெனில் எனில் (isa<MapHNode>(Current_முனை))
    திரும்பு முனை_Kind::Map;
  இல்லையெனில் எனில் (isa<SequenceHNode>(Current_முனை))
    திரும்பு முனை_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

வெறுமை உள்ளீடு::setError(முனை *முனை_3, மாறிலி Twine &message) {
  Strm->printError(முனை_3, message);
  EC = make_error_code(errc::invalid_argument);
}

வெறுமை உள்ளீடு::setError(மாறிலி SMRange &range, மாறிலி Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

வெறுமை உள்ளீடு::reportWarning(HNode *hnode, மாறிலி Twine &message) {
  ப்ரதிஜ்ஞா(hnode && "HNode must not be NULL");
  Strm->printError(hnode->முனை_2, message, SourceMgr::DK_Warning);
}

வெறுமை உள்ளீடு::reportWarning(முனை *முனை_3, மாறிலி Twine &message) {
  Strm->printError(முனை_3, message, SourceMgr::DK_Warning);
}

வெறுமை உள்ளீடு::reportWarning(மாறிலி SMRange &range, மாறிலி Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

வெறுமை உள்ளீடு::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

வெறுமை உள்ளீடு::saveAliasHNode(முனை *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  எனில் (!Anchor.empty())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

உள்ளீடு::HNode *உள்ளீடு::createHNodes(முனை *N) {
  SmallString<128> StringStorage;
  தேர்வு (N->getType()) {
  நிலை முனை::NK_Scalar: {
    Scalar_முனை *SN = dyn_cast<Scalar_முனை>(N);
    StringRef சாவி_Str = SN->get_மதிப்பு(StringStorage);
    எனில் (!StringStorage.empty()) {
      // Copy string to permanent storage
      சாவி_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = புதிய (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, சாவி_Str);
    saveAliasHNode(SN, SHNode);
    திரும்பு SHNode;
  }
  நிலை முனை::NK_BlockScalar: {
    Block_Scalar_முனை *BSN = dyn_cast<Block_Scalar_முனை>(N);
    StringRef மதிப்பு_Copy = BSN->get_மதிப்பு().copy(StringAllocator);
    auto *BSHNode =
        புதிய (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, மதிப்பு_Copy);
    saveAliasHNode(BSN, BSHNode);
    திரும்பு BSHNode;
  }
  நிலை முனை::NK_Sequence: {
    Sequence_முனை *SQ = dyn_cast<Sequence_முனை>(N);
    auto SQHNode = புதிய (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    க்கு (முனை &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      எனில் (EC)
        நிறுத்து;
      SQHNode->Entries.தள்ளு_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    திரும்பு SQHNode;
  }
  நிலை முனை::NK_Mapping: {
    Mapping_முனை *Map = dyn_cast<Mapping_முனை>(N);
    auto mapHNode = புதிய (MapHNodeAllocator.Allocate()) MapHNode(N);
    க்கு (சாவி_மதிப்பு_முனை &KVN : *Map) {
      முனை *சாவி_முனை = KVN.get_சாவி();
      Scalar_முனை *சாவி = dyn_cast_if_present<Scalar_முனை>(சாவி_முனை);
      முனை *மதிப்பு = KVN.get_மதிப்பு();
      எனில் (!சாவி || !மதிப்பு) {
        எனில் (!சாவி)
          setError(சாவி_முனை, "Map key must be a scalar");
        எனில் (!மதிப்பு)
          setError(சாவி_முனை, "Map value must not be empty");
        நிறுத்து;
      }
      StringStorage.clear();
      StringRef சாவி_Str = சாவி->get_மதிப்பு(StringStorage);
      எனில் (!StringStorage.empty()) {
        // Copy string to permanent storage
        சாவி_Str = StringStorage.str().copy(StringAllocator);
      }
      எனில் (mapHNode->Mapping.மொத்தம்(சாவி_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(சாவி_முனை, Twine("duplicated mapping key '") + சாவி_Str + "'");
      auto மதிப்பு_HNode = createHNodes(மதிப்பு);
      எனில் (EC)
        நிறுத்து;
      mapHNode->Mapping[சாவி_Str] =
          std::make_pair(std::move(மதிப்பு_HNode), சாவி_முனை->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    திரும்பு std::move(mapHNode);
  }
  நிலை முனை::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    திரும்பு புதிய (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  நிலை முனை::NK_Alias: {
    Alias_முனை *AN = dyn_cast<Alias_முனை>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    எனில் (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      திரும்பு nullptr;
    }
    திரும்பு AHN->second;
  }
  இயல்பு:
    setError(N, "unknown node kind");
    திரும்பு nullptr;
  }
}

வெறுமை உள்ளீடு::setError(மாறிலி Twine &Message) {
  setError(Current_முனை, Message);
}

வெறுமை உள்ளீடு::set_Allow_Unknown_சாவிகள்(தருக்க Allow) { Allow_Unknown_சாவிகள் = Allow; }

தருக்க உள்ளீடு::canElideEmptySequence() {
  திரும்பு பொய்;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, வெறுமை *context, முழு WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = இயல்பு;

தருக்க Output::outputting() மாறிலி {
  திரும்பு உண்மை;
}

வெறுமை Output::beginMapping() {
  State_அடுக்கு.தள்ளு_back(in_Map_First_சாவி);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

தருக்க Output::mapTag(StringRef Tag, தருக்க Use) {
  எனில் (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    தருக்க SequenceElement = பொய்;
    எனில் (State_அடுக்கு.size() > 1) {
      auto &E = State_அடுக்கு[State_அடுக்கு.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    எனில் (SequenceElement && State_அடுக்கு.back() == in_Map_First_சாவி) {
      new_வரி_Check();
    } இல்லையெனில் {
      output(" ");
    }
    output(Tag);
    எனில் (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      எனில் (State_அடுக்கு.back() == in_Map_First_சாவி) {
        State_அடுக்கு.எடு_back();
        State_அடுக்கு.தள்ளு_back(in_Map_Other_சாவி);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  திரும்பு Use;
}

வெறுமை Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  எனில் (State_அடுக்கு.back() == in_Map_First_சாவி) {
    Padding = PaddingBeforeContainer;
    new_வரி_Check();
    output("{}");
    Padding = "\n";
  }
  State_அடுக்கு.எடு_back();
}

std::vector<StringRef> Output::சாவிகள்() {
  report_fatal_error("invalid call");
}

தருக்க Output::preflight_சாவி(StringRef சாவி, தருக்க Required, தருக்க SameAsDefault,
                          தருக்க &UseDefault, வெறுமை *&SaveInfo) {
  UseDefault = பொய்;
  SaveInfo = nullptr;
  எனில் (Required || !SameAsDefault || Write_Default_மதிப்புகள்) {
    auto State = State_அடுக்கு.back();
    எனில் (State == in_Flow_Map_First_சாவி || State == in_Flow_Map_Other_சாவி) {
      flow_சாவி(சாவி);
    } இல்லையெனில் {
      new_வரி_Check();
      padded_சாவி(சாவி);
    }
    திரும்பு உண்மை;
  }
  திரும்பு பொய்;
}

வெறுமை Output::postflight_சாவி(வெறுமை *) {
  எனில் (State_அடுக்கு.back() == in_Map_First_சாவி) {
    State_அடுக்கு.எடு_back();
    State_அடுக்கு.தள்ளு_back(in_Map_Other_சாவி);
  } இல்லையெனில் எனில் (State_அடுக்கு.back() == in_Flow_Map_First_சாவி) {
    State_அடுக்கு.எடு_back();
    State_அடுக்கு.தள்ளு_back(in_Flow_Map_Other_சாவி);
  }
}

வெறுமை Output::beginFlowMapping() {
  State_அடுக்கு.தள்ளு_back(in_Flow_Map_First_சாவி);
  new_வரி_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

வெறுமை Output::endFlowMapping() {
  State_அடுக்கு.எடு_back();
  output_Up_To_End_Of_வரி(" }");
}

வெறுமை Output::beginDocuments() {
  output_Up_To_End_Of_வரி("---");
}

தருக்க Output::preflightDocument(unsigned index) {
  எனில் (index > 0)
    output_Up_To_End_Of_வரி("\n---");
  திரும்பு உண்மை;
}

வெறுமை Output::postflightDocument() {
}

வெறுமை Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_அடுக்கு.தள்ளு_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  திரும்பு 0;
}

வெறுமை Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  எனில் (State_அடுக்கு.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_வரி_Check(/*EmptySequence=*/உண்மை);
    output("[]");
    Padding = "\n";
  }
  State_அடுக்கு.எடு_back();
}

தருக்க Output::preflightElement(unsigned, வெறுமை *&SaveInfo) {
  SaveInfo = nullptr;
  திரும்பு உண்மை;
}

வெறுமை Output::postflightElement(வெறுமை *) {
  எனில் (State_அடுக்கு.back() == inSeqFirstElement) {
    State_அடுக்கு.எடு_back();
    State_அடுக்கு.தள்ளு_back(inSeqOtherElement);
  } இல்லையெனில் எனில் (State_அடுக்கு.back() == inFlowSeqFirstElement) {
    State_அடுக்கு.எடு_back();
    State_அடுக்கு.தள்ளு_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_அடுக்கு.தள்ளு_back(inFlowSeqFirstElement);
  new_வரி_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = பொய்;
  திரும்பு 0;
}

வெறுமை Output::endFlowSequence() {
  State_அடுக்கு.எடு_back();
  output_Up_To_End_Of_வரி(" ]");
}

தருக்க Output::preflightFlowElement(unsigned, வெறுமை *&SaveInfo) {
  எனில் (NeedFlowSequenceComma)
    output(", ");
  எனில் (WrapColumn && Column > WrapColumn) {
    output("\n");
    க்கு (முழு i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  திரும்பு உண்மை;
}

வெறுமை Output::postflightFlowElement(வெறுமை *) {
  NeedFlowSequenceComma = உண்மை;
}

வெறுமை Output::beginEnumScalar() {
  EnumerationMatchFound = பொய்;
}

தருக்க Output::matchEnumScalar(StringRef Str, தருக்க Match) {
  எனில் (Match && !EnumerationMatchFound) {
    new_வரி_Check();
    output_Up_To_End_Of_வரி(Str);
    EnumerationMatchFound = உண்மை;
  }
  திரும்பு பொய்;
}

தருக்க Output::matchEnumFallback() {
  எனில் (EnumerationMatchFound)
    திரும்பு பொய்;
  EnumerationMatchFound = உண்மை;
  திரும்பு உண்மை;
}

வெறுமை Output::endEnumScalar() {
  எனில் (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

தருக்க Output::beginBitSetScalar(தருக்க &DoClear) {
  new_வரி_Check();
  output("[ ");
  Need_Bit_மதிப்பு_Comma = பொய்;
  DoClear = பொய்;
  திரும்பு உண்மை;
}

தருக்க Output::bitSetMatch(StringRef Str, தருக்க Matches) {
  எனில் (Matches) {
    எனில் (Need_Bit_மதிப்பு_Comma)
      output(", ");
    output(Str);
    Need_Bit_மதிப்பு_Comma = உண்மை;
  }
  திரும்பு பொய்;
}

வெறுமை Output::endBitSetScalar() {
  output_Up_To_End_Of_வரி(" ]");
}

வெறுமை Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_வரி_Check();
  எனில் (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_வரி("''");
    திரும்பு;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_வரி("");
}

வெறுமை Output::blockScalarString(StringRef &S) {
  எனில் (!State_அடுக்கு.empty())
    new_வரி_Check();
  output(" |");

  unsigned Indent = State_அடுக்கு.empty() ? 1 : State_அடுக்கு.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", பொய்);
  க்கு (வரி_iterator வரிகள்(*Buffer, பொய்); !வரிகள்.is_at_end(); ++வரிகள்) {
    output_New_வரி();
    க்கு (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*வரிகள்);
  }
  output_Up_To_End_Of_வரி("");
}

வெறுமை Output::scalarTag(std::string &Tag) {
  எனில் (Tag.empty())
    திரும்பு;
  new_வரி_Check();
  output(Tag);
  output(" ");
}

வெறுமை Output::setError(மாறிலி Twine &message) {
}

std::error_code Output::error() { திரும்பு {}; }

தருக்க Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  எனில் (State_அடுக்கு.size() < 2)
    திரும்பு உண்மை;
  எனில் (State_அடுக்கு.back() != in_Map_First_சாவி)
    திரும்பு உண்மை;
  திரும்பு !inSeqAnyElement(State_அடுக்கு[State_அடுக்கு.size() - 2]);
}

வெறுமை Output::output(StringRef s) {
  Column += s.size();
  Out << s;
}

வெறுமை Output::output(StringRef S, QuotingType MustQuote) {
  எனில் (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    திரும்பு;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  எனில் (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ பொய்));
    output(Quote);
    திரும்பு;
  }

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  மாறிலி எழுத்து *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  போது (j < End) {
    எனில் (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

வெறுமை Output::output_Up_To_End_Of_வரி(StringRef s) {
  output(s);
  எனில் (State_அடுக்கு.empty() || (!inFlowSeqAnyElement(State_அடுக்கு.back()) &&
                             !in_Flow_Map_Any_சாவி(State_அடுக்கு.back())))
    Padding = "\n";
}

வெறுமை Output::output_New_வரி() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

வெறுமை Output::new_வரி_Check(தருக்க EmptySequence) {
  எனில் (Padding != "\n") {
    output(Padding);
    Padding = {};
    திரும்பு;
  }
  output_New_வரி();
  Padding = {};

  எனில் (State_அடுக்கு.size() == 0 || EmptySequence)
    திரும்பு;

  unsigned Indent = State_அடுக்கு.size() - 1;
  தருக்க PossiblyNestedSeq = பொய்;
  auto I = State_அடுக்கு.rbegin(), E = State_அடுக்கு.rend();

  எனில் (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = உண்மை; // Not possibly but always.
    ++Indent;
  } இல்லையெனில் எனில் (*I == in_Map_First_சாவி || *I == in_Flow_Map_First_சாவி ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = உண்மை;
    ++I; // Skip back().
  }

  unsigned Output_Dash_மொத்தம் = 0;
  எனில் (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    போது (I != E) {
      // Don't count the top of nested sequence.
      எனில் (!inSeqAnyElement(*I))
        நிறுத்து;

      ++Output_Dash_மொத்தம்;

      // Stop counting if consecutive inSeqFirstElement ends.
      எனில் (*I++ != inSeqFirstElement)
        நிறுத்து;
    }
  }

  க்கு (unsigned I = Output_Dash_மொத்தம்; I < Indent; ++I)
    output("  ");

  க்கு (unsigned I = 0; I < Output_Dash_மொத்தம்; ++I)
    output("- ");
}

வெறுமை Output::padded_சாவி(StringRef சாவி_2) {
  output(சாவி_2, needsQuotes(சாவி_2, பொய்));
  output(":");
  மாறிலி எழுத்து *spaces = "                ";
  எனில் (சாவி_2.size() < strlen(spaces))
    Padding = &spaces[சாவி_2.size()];
  இல்லையெனில்
    Padding = " ";
}

வெறுமை Output::flow_சாவி(StringRef சாவி) {
  எனில் (State_அடுக்கு.back() == in_Flow_Map_Other_சாவி)
    output(", ");
  எனில் (WrapColumn && Column > WrapColumn) {
    output("\n");
    க்கு (முழு I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(சாவி, needsQuotes(சாவி, பொய்));
  output(": ");
}

முனை_Kind Output::get_முனை_Kind() { report_fatal_error("invalid call"); }

தருக்க Output::inSeqAnyElement(InState State) {
  திரும்பு State == inSeqFirstElement || State == inSeqOtherElement;
}

தருக்க Output::inFlowSeqAnyElement(InState State) {
  திரும்பு State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

தருக்க Output::in_Map_Any_சாவி(InState State) {
  திரும்பு State == in_Map_First_சாவி || State == in_Map_Other_சாவி;
}

தருக்க Output::in_Flow_Map_Any_சாவி(InState State) {
  திரும்பு State == in_Flow_Map_First_சாவி || State == in_Flow_Map_Other_சாவி;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

வெறுமை ScalarTraits<தருக்க>::output(மாறிலி தருக்க &Val, வெறுமை *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<தருக்க>::உள்ளீடு_2(StringRef Scalar, வெறுமை *, தருக்க &Val) {
  எனில் (std::optional<தருக்க> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    திரும்பு StringRef();
  }
  திரும்பு "invalid boolean";
}

வெறுமை ScalarTraits<StringRef>::output(மாறிலி StringRef &Val, வெறுமை *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::உள்ளீடு_2(StringRef Scalar, வெறுமை *,
                                         StringRef &Val) {
  Val = Scalar;
  திரும்பு StringRef();
}

வெறுமை ScalarTraits<std::string>::output(மாறிலி std::string &Val, வெறுமை *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::உள்ளீடு_2(StringRef Scalar, வெறுமை *,
                                           std::string &Val) {
  Val = Scalar.str();
  திரும்பு StringRef();
}

வெறுமை ScalarTraits<uint8_t>::output(மாறிலி uint8_t &Val, வெறுமை *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t இலக்கம் = Val;
  Out << இலக்கம்;
}

StringRef ScalarTraits<uint8_t>::உள்ளீடு_2(StringRef Scalar, வெறுமை *, uint8_t &Val) {
  unsigned long long n;
  எனில் (getAsUnsignedInteger(Scalar, 0, n))
    திரும்பு "invalid number";
  எனில் (n > 0xFF)
    திரும்பு "out of range number";
  Val = n;
  திரும்பு StringRef();
}

வெறுமை ScalarTraits<uint16_t>::output(மாறிலி uint16_t &Val, வெறுமை *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::உள்ளீடு_2(StringRef Scalar, வெறுமை *,
                                        uint16_t &Val) {
  unsigned long long n;
  எனில் (getAsUnsignedInteger(Scalar, 0, n))
    திரும்பு "invalid number";
  எனில் (n > 0xFFFF)
    திரும்பு "out of range number";
  Val = n;
  திரும்பு StringRef();
}

வெறுமை ScalarTraits<uint32_t>::output(மாறிலி uint32_t &Val, வெறுமை *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::உள்ளீடு_2(StringRef Scalar, வெறுமை *,
                                        uint32_t &Val) {
  unsigned long long n;
  எனில் (getAsUnsignedInteger(Scalar, 0, n))
    திரும்பு "invalid number";
  எனில் (n > 0xFFFFFFFFUL)
    திரும்பு "out of range number";
  Val = n;
  திரும்பு StringRef();
}

வெறுமை ScalarTraits<uint64_t>::output(மாறிலி uint64_t &Val, வெறுமை *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::உள்ளீடு_2(StringRef Scalar, வெறுமை *,
                                        uint64_t &Val) {
  unsigned long long N;
  எனில் (getAsUnsignedInteger(Scalar, 0, N))
    திரும்பு "invalid number";
  Val = N;
  திரும்பு StringRef();
}

வெறுமை ScalarTraits<int8_t>::output(மாறிலி int8_t &Val, வெறுமை *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t இலக்கம் = Val;
  Out << இலக்கம்;
}

StringRef ScalarTraits<int8_t>::உள்ளீடு_2(StringRef Scalar, வெறுமை *, int8_t &Val) {
  long long N;
  எனில் (getAsSignedInteger(Scalar, 0, N))
    திரும்பு "invalid number";
  எனில் ((N > 127) || (N < -128))
    திரும்பு "out of range number";
  Val = N;
  திரும்பு StringRef();
}

வெறுமை ScalarTraits<int16_t>::output(மாறிலி int16_t &Val, வெறுமை *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::உள்ளீடு_2(StringRef Scalar, வெறுமை *, int16_t &Val) {
  long long N;
  எனில் (getAsSignedInteger(Scalar, 0, N))
    திரும்பு "invalid number";
  எனில் ((N > INT16_MAX) || (N < INT16_MIN))
    திரும்பு "out of range number";
  Val = N;
  திரும்பு StringRef();
}

வெறுமை ScalarTraits<int32_t>::output(மாறிலி int32_t &Val, வெறுமை *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::உள்ளீடு_2(StringRef Scalar, வெறுமை *, int32_t &Val) {
  long long N;
  எனில் (getAsSignedInteger(Scalar, 0, N))
    திரும்பு "invalid number";
  எனில் ((N > INT32_MAX) || (N < INT32_MIN))
    திரும்பு "out of range number";
  Val = N;
  திரும்பு StringRef();
}

வெறுமை ScalarTraits<int64_t>::output(மாறிலி int64_t &Val, வெறுமை *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::உள்ளீடு_2(StringRef Scalar, வெறுமை *, int64_t &Val) {
  long long N;
  எனில் (getAsSignedInteger(Scalar, 0, N))
    திரும்பு "invalid number";
  Val = N;
  திரும்பு StringRef();
}

வெறுமை ScalarTraits<இரட்டை>::output(மாறிலி இரட்டை &Val, வெறுமை *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<இரட்டை>::உள்ளீடு_2(StringRef Scalar, வெறுமை *, இரட்டை &Val) {
  எனில் (to_float(Scalar, Val))
    திரும்பு StringRef();
  திரும்பு "invalid floating point number";
}

வெறுமை ScalarTraits<தசம>::output(மாறிலி தசம &Val, வெறுமை *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<தசம>::உள்ளீடு_2(StringRef Scalar, வெறுமை *, தசம &Val) {
  எனில் (to_float(Scalar, Val))
    திரும்பு StringRef();
  திரும்பு "invalid floating point number";
}

வெறுமை ScalarTraits<Hex8>::output(மாறிலி Hex8 &Val, வெறுமை *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::உள்ளீடு_2(StringRef Scalar, வெறுமை *, Hex8 &Val) {
  unsigned long long n;
  எனில் (getAsUnsignedInteger(Scalar, 0, n))
    திரும்பு "invalid hex8 number";
  எனில் (n > 0xFF)
    திரும்பு "out of range hex8 number";
  Val = n;
  திரும்பு StringRef();
}

வெறுமை ScalarTraits<Hex16>::output(மாறிலி Hex16 &Val, வெறுமை *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::உள்ளீடு_2(StringRef Scalar, வெறுமை *, Hex16 &Val) {
  unsigned long long n;
  எனில் (getAsUnsignedInteger(Scalar, 0, n))
    திரும்பு "invalid hex16 number";
  எனில் (n > 0xFFFF)
    திரும்பு "out of range hex16 number";
  Val = n;
  திரும்பு StringRef();
}

வெறுமை ScalarTraits<Hex32>::output(மாறிலி Hex32 &Val, வெறுமை *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::உள்ளீடு_2(StringRef Scalar, வெறுமை *, Hex32 &Val) {
  unsigned long long n;
  எனில் (getAsUnsignedInteger(Scalar, 0, n))
    திரும்பு "invalid hex32 number";
  எனில் (n > 0xFFFFFFFFUL)
    திரும்பு "out of range hex32 number";
  Val = n;
  திரும்பு StringRef();
}

வெறுமை ScalarTraits<Hex64>::output(மாறிலி Hex64 &Val, வெறுமை *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::உள்ளீடு_2(StringRef Scalar, வெறுமை *, Hex64 &Val) {
  unsigned long long இலக்கம்;
  எனில் (getAsUnsignedInteger(Scalar, 0, இலக்கம்))
    திரும்பு "invalid hex64 number";
  Val = இலக்கம்;
  திரும்பு StringRef();
}

வெறுமை ScalarTraits<VersionTuple>::output(மாறிலி VersionTuple &Val, வெறுமை *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::உள்ளீடு_2(StringRef Scalar, வெறுமை *,
                                            VersionTuple &Val) {
  எனில் (Val.tryParse(Scalar))
    திரும்பு "invalid version format";
  திரும்பு StringRef();
}
