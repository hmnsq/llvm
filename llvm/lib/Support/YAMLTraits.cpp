//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/YAMLTraits.h"
#అన్తర్భావ "llvm/ADT/STLExtras.h"
#అన్తర్భావ "llvm/ADT/SmallString.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/ADT/Twine.h"
#అన్తర్భావ "llvm/Support/Casting.h"
#అన్తర్భావ "llvm/Support/Errc.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ "llvm/Support/Format.h"
#అన్తర్భావ "llvm/Support/LineIterator.h"
#అన్తర్భావ "llvm/Support/MemoryBuffer.h"
#అన్తర్భావ "llvm/Support/VersionTuple.h"
#అన్తర్భావ "llvm/Support/YAMLParser.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <cassert>
#అన్తర్భావ <cstdint>
#అన్తర్భావ <cstring>
#అన్తర్భావ <string>
#అన్తర్భావ <vector>

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(శూన్య *Context) : Ctxt(Context) {}

IO::~IO() = అప్రమేయ;

శూన్య *IO::getContext() స్థిరరాశి {
  తిరిగి Ctxt;
}

శూన్య IO::సమితి_Context(శూన్య *Context) {
  Ctxt = Context;
}

శూన్య IO::సమితి_Allow_Unknown_కీలకాలు(తార్కిక Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//ఆదానం
//===----------------------------------------------------------------------===//

ఆదానం::ఆదానం(StringRef ఆదానం_Content, శూన్య *Ctxt,
             ఆధారం_Mgr::DiagHandlerTy DiagHandler, శూన్య *DiagHandlerCtxt)
    : IO(Ctxt), Strm(కొత్త ప్రవాహం(ఆదానం_Content, SrcMgr, అబద్ధం, &EC)) {
  ఒకవేళ (DiagHandler)
    SrcMgr.సమితి_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

ఆదానం::ఆదానం(జ్ఞాపకం_బఫరు_Ref ఆదానం, శూన్య *Ctxt,
             ఆధారం_Mgr::DiagHandlerTy DiagHandler, శూన్య *DiagHandlerCtxt)
    : IO(Ctxt), Strm(కొత్త ప్రవాహం(ఆదానం, SrcMgr, అబద్ధం, &EC)) {
  ఒకవేళ (DiagHandler)
    SrcMgr.సమితి_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

ఆదానం::~ఆదానం() = అప్రమేయ;

std::error_సంకేతం ఆదానం::error() { తిరిగి EC; }

తార్కిక ఆదానం::outputting() స్థిరరాశి {
  తిరిగి అబద్ధం;
}

తార్కిక ఆదానం::సమితి_Current_Document() {
  ఒకవేళ (DocIterator != Strm->చివర()) {
    కణుపు *N = DocIterator->get_మూలం();
    ఒకవేళ (!N) {
      EC = make_error_సంకేతం(errc::invalid_వాదం);
      తిరిగి అబద్ధం;
    }

    ఒకవేళ (isa<Null_కణుపు>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      తిరిగి సమితి_Current_Document();
    }
    releaseHNodeBuffers();
    శిఖరం_కణుపు = createHNodes(N);
    Current_కణుపు = శిఖరం_కణుపు;
    తిరిగి నిజం;
  }
  తిరిగి అబద్ధం;
}

తార్కిక ఆదానం::తదుపరి_Document() {
  తిరిగి ++DocIterator != Strm->చివర();
}

స్థిరరాశి కణుపు *ఆదానం::get_Current_కణుపు() స్థిరరాశి {
  తిరిగి Current_కణుపు ? Current_కణుపు->కణుపు_2 : శూన్యసూచిక;
}

తార్కిక ఆదానం::map_ట్యాగ్(StringRef ట్యాగ్, తార్కిక Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  ఒకవేళ (!Current_కణుపు)
    తిరిగి అబద్ధం;

  std::string దొరికింది_ట్యాగ్ = Current_కణుపు->కణుపు_2->get_Verbatim_ట్యాగ్();
  ఒకవేళ (దొరికింది_ట్యాగ్.ఖాళీలు()) {
    // If no tag found and 'Tag' is the default, say it was found.
    తిరిగి Default;
  }
  // Return true iff found tag matches supplied tag.
  తిరిగి ట్యాగ్ == దొరికింది_ట్యాగ్;
}

శూన్య ఆదానం::beginMapping() {
  ఒకవేళ (EC)
    తిరిగి;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_కణుపు);
  ఒకవేళ (MN) {
    MN->Valid_కీలకాలు.clear();
  }
}

std::సదిశ<StringRef> ఆదానం::కీలకాలు() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_కణుపు);
  std::సదిశ<StringRef> Ret;
  ఒకవేళ (!MN) {
    సమితి_Error(Current_కణుపు, "not a mapping");
    తిరిగి Ret;
  }
  కొరకు (స్వయంచాలక &P : MN->Mapping)
    Ret.నెట్టు_వెనుక(P.మొదటి_2());
  తిరిగి Ret;
}

తార్కిక ఆదానం::preflight_కీలకం(StringRef కీలకం, తార్కిక Required, తార్కిక, తార్కిక &UseDefault,
                         శూన్య *&SaveInfo) {
  UseDefault = అబద్ధం;
  ఒకవేళ (EC)
    తిరిగి అబద్ధం;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  ఒకవేళ (!Current_కణుపు) {
    ఒకవేళ (Required)
      EC = make_error_సంకేతం(errc::invalid_వాదం);
    లేకపోతే
      UseDefault = నిజం;
    తిరిగి అబద్ధం;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_కణుపు);
  ఒకవేళ (!MN) {
    ఒకవేళ (Required || !isa<ఖాళీలు_HNode>(Current_కణుపు))
      సమితి_Error(Current_కణుపు, "not a mapping");
    లేకపోతే
      UseDefault = నిజం;
    తిరిగి అబద్ధం;
  }
  MN->Valid_కీలకాలు.నెట్టు_వెనుక(కీలకం.str());
  HNode *విలువ = MN->Mapping[కీలకం].మొదటి_2;
  ఒకవేళ (!విలువ) {
    ఒకవేళ (Required)
      సమితి_Error(Current_కణుపు, Twine("missing required key '") + కీలకం + "'");
    లేకపోతే
      UseDefault = నిజం;
    తిరిగి అబద్ధం;
  }
  SaveInfo = Current_కణుపు;
  Current_కణుపు = విలువ;
  తిరిగి నిజం;
}

శూన్య ఆదానం::postflight_కీలకం(శూన్య *saveInfo) {
  Current_కణుపు = reinterpret_cast<HNode *>(saveInfo);
}

శూన్య ఆదానం::చివర_Mapping() {
  ఒకవేళ (EC)
    తిరిగి;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_కణుపు);
  ఒకవేళ (!MN)
    తిరిగి;
  కొరకు (స్థిరరాశి స్వయంచాలక &NN : MN->Mapping) {
    ఒకవేళ (!is_contained(MN->Valid_కీలకాలు, NN.మొదటి_2())) {
      స్థిరరాశి SMRange &ReportLoc = NN.రెండవ_2.రెండవ_2;
      ఒకవేళ (!Allow_Unknown_కీలకాలు) {
        సమితి_Error(ReportLoc, Twine("unknown key '") + NN.మొదటి_2() + "'");
        ఆపు;
      } లేకపోతే
        reportWarning(ReportLoc, Twine("unknown key '") + NN.మొదటి_2() + "'");
    }
  }
}

శూన్య ఆదానం::beginFlowMapping() { beginMapping(); }

శూన్య ఆదానం::చివర_Flow_Mapping() { చివర_Mapping(); }

గుర్తులేని ఆదానం::beginSequence() {
  ఒకవేళ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_కణుపు))
    తిరిగి SQ->Entries.కొలత_2();
  ఒకవేళ (isa<ఖాళీలు_HNode>(Current_కణుపు))
    తిరిగి 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  ఒకవేళ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_కణుపు)) {
    ఒకవేళ (isNull(SN->విలువ_2()))
      తిరిగి 0;
  }
  // Any other type of HNode is an error.
  సమితి_Error(Current_కణుపు, "not a sequence");
  తిరిగి 0;
}

శూన్య ఆదానం::చివర_Sequence() {
}

తార్కిక ఆదానం::preflightElement(గుర్తులేని సూచీ_2, శూన్య *&SaveInfo) {
  ఒకవేళ (EC)
    తిరిగి అబద్ధం;
  ఒకవేళ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_కణుపు)) {
    SaveInfo = Current_కణుపు;
    Current_కణుపు = SQ->Entries[సూచీ_2];
    తిరిగి నిజం;
  }
  తిరిగి అబద్ధం;
}

శూన్య ఆదానం::postflightElement(శూన్య *SaveInfo) {
  Current_కణుపు = reinterpret_cast<HNode *>(SaveInfo);
}

గుర్తులేని ఆదానం::beginFlowSequence() { తిరిగి beginSequence(); }

తార్కిక ఆదానం::preflightFlowElement(గుర్తులేని సూచీ, శూన్య *&SaveInfo) {
  ఒకవేళ (EC)
    తిరిగి అబద్ధం;
  ఒకవేళ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_కణుపు)) {
    SaveInfo = Current_కణుపు;
    Current_కణుపు = SQ->Entries[సూచీ];
    తిరిగి నిజం;
  }
  తిరిగి అబద్ధం;
}

శూన్య ఆదానం::postflightFlowElement(శూన్య *SaveInfo) {
  Current_కణుపు = reinterpret_cast<HNode *>(SaveInfo);
}

శూన్య ఆదానం::చివర_Flow_Sequence() {
}

శూన్య ఆదానం::beginEnumScalar() {
  Scalar_సరిపోలిక_దొరికింది = అబద్ధం;
}

తార్కిక ఆదానం::సరిపోలిక_Enum_Scalar(StringRef Str, తార్కిక) {
  ఒకవేళ (Scalar_సరిపోలిక_దొరికింది)
    తిరిగి అబద్ధం;
  ఒకవేళ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_కణుపు)) {
    ఒకవేళ (SN->విలువ_2() == Str) {
      Scalar_సరిపోలిక_దొరికింది = నిజం;
      తిరిగి నిజం;
    }
  }
  తిరిగి అబద్ధం;
}

తార్కిక ఆదానం::సరిపోలిక_Enum_Fallback() {
  ఒకవేళ (Scalar_సరిపోలిక_దొరికింది)
    తిరిగి అబద్ధం;
  Scalar_సరిపోలిక_దొరికింది = నిజం;
  తిరిగి నిజం;
}

శూన్య ఆదానం::చివర_Enum_Scalar() {
  ఒకవేళ (!Scalar_సరిపోలిక_దొరికింది) {
    సమితి_Error(Current_కణుపు, "unknown enumerated scalar");
  }
}

తార్కిక ఆదానం::begin_బిట్టు_సమితి_Scalar(తార్కిక &DoClear) {
  బిట్టు_విలువలు_వాడిన.clear();
  ఒకవేళ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_కణుపు)) {
    బిట్టు_విలువలు_వాడిన.resize(SQ->Entries.కొలత_2());
  } లేకపోతే {
    సమితి_Error(Current_కణుపు, "expected sequence of bit values");
  }
  DoClear = నిజం;
  తిరిగి నిజం;
}

తార్కిక ఆదానం::బిట్టు_సమితి_సరిపోలిక(StringRef Str, తార్కిక) {
  ఒకవేళ (EC)
    తిరిగి అబద్ధం;
  ఒకవేళ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_కణుపు)) {
    గుర్తులేని సూచీ_2 = 0;
    కొరకు (స్వయంచాలక &N : SQ->Entries) {
      ఒకవేళ (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        ఒకవేళ (SN->విలువ_2() == Str) {
          బిట్టు_విలువలు_వాడిన[సూచీ_2] = నిజం;
          తిరిగి నిజం;
        }
      } లేకపోతే {
        సమితి_Error(Current_కణుపు, "unexpected scalar in sequence of bit values");
      }
      ++సూచీ_2;
    }
  } లేకపోతే {
    సమితి_Error(Current_కణుపు, "expected sequence of bit values");
  }
  తిరిగి అబద్ధం;
}

శూన్య ఆదానం::చివర_బిట్టు_సమితి_Scalar() {
  ఒకవేళ (EC)
    తిరిగి;
  ఒకవేళ (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_కణుపు)) {
    ప్రతిజ్ఞా(బిట్టు_విలువలు_వాడిన.కొలత_2() == SQ->Entries.కొలత_2());
    కొరకు (గుర్తులేని i = 0; i < SQ->Entries.కొలత_2(); ++i) {
      ఒకవేళ (!బిట్టు_విలువలు_వాడిన[i]) {
        సమితి_Error(SQ->Entries[i], "unknown bit value");
        తిరిగి;
      }
    }
  }
}

శూన్య ఆదానం::scalarString(StringRef &S, QuotingType) {
  ఒకవేళ (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_కణుపు)) {
    S = SN->విలువ_2();
  } లేకపోతే {
    సమితి_Error(Current_కణుపు, "unexpected scalar");
  }
}

శూన్య ఆదానం::ఖండం_Scalar_String(StringRef &S) { scalarString(S, QuotingType::None); }

శూన్య ఆదానం::scalar_ట్యాగ్(std::string &ట్యాగ్) {
  ట్యాగ్ = Current_కణుపు->కణుపు_2->get_Verbatim_ట్యాగ్();
}

శూన్య ఆదానం::సమితి_Error(HNode *hnode, స్థిరరాశి Twine &సందేశం_2) {
  ప్రతిజ్ఞా(hnode && "HNode must not be NULL");
  సమితి_Error(hnode->కణుపు_2, సందేశం_2);
}

కణుపు_రకం ఆదానం::get_కణుపు_రకం() {
  ఒకవేళ (isa<ScalarHNode>(Current_కణుపు))
    తిరిగి కణుపు_రకం::Scalar;
  లేకపోతే ఒకవేళ (isa<MapHNode>(Current_కణుపు))
    తిరిగి కణుపు_రకం::Map;
  లేకపోతే ఒకవేళ (isa<SequenceHNode>(Current_కణుపు))
    తిరిగి కణుపు_రకం::Sequence;
  llvm_unreachable("Unsupported node kind");
}

శూన్య ఆదానం::సమితి_Error(కణుపు *కణుపు_3, స్థిరరాశి Twine &సందేశం_2) {
  Strm->printError(కణుపు_3, సందేశం_2);
  EC = make_error_సంకేతం(errc::invalid_వాదం);
}

శూన్య ఆదానం::సమితి_Error(స్థిరరాశి SMRange &range, స్థిరరాశి Twine &సందేశం_2) {
  Strm->printError(range, సందేశం_2);
  EC = make_error_సంకేతం(errc::invalid_వాదం);
}

శూన్య ఆదానం::reportWarning(HNode *hnode, స్థిరరాశి Twine &సందేశం_2) {
  ప్రతిజ్ఞా(hnode && "HNode must not be NULL");
  Strm->printError(hnode->కణుపు_2, సందేశం_2, ఆధారం_Mgr::DK_Warning);
}

శూన్య ఆదానం::reportWarning(కణుపు *కణుపు_3, స్థిరరాశి Twine &సందేశం_2) {
  Strm->printError(కణుపు_3, సందేశం_2, ఆధారం_Mgr::DK_Warning);
}

శూన్య ఆదానం::reportWarning(స్థిరరాశి SMRange &range, స్థిరరాశి Twine &సందేశం_2) {
  Strm->printError(range, సందేశం_2, ఆధారం_Mgr::DK_Warning);
}

శూన్య ఆదానం::releaseHNodeBuffers() {
  ఖాళీలు_HNode_Allocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

శూన్య ఆదానం::saveAliasHNode(కణుపు *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  ఒకవేళ (!Anchor.ఖాళీలు())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

ఆదానం::HNode *ఆదానం::createHNodes(కణుపు *N) {
  SmallString<128> StringStorage;
  ఎంపిక (N->getType()) {
  స్థితి కణుపు::NK_Scalar: {
    Scalar_కణుపు *SN = dyn_cast<Scalar_కణుపు>(N);
    StringRef కీలకం_Str = SN->get_విలువ(StringStorage);
    ఒకవేళ (!StringStorage.ఖాళీలు()) {
      // Copy string to permanent storage
      కీలకం_Str = StringStorage.str().copy(StringAllocator);
    }
    స్వయంచాలక *SHNode = కొత్త (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, కీలకం_Str);
    saveAliasHNode(SN, SHNode);
    తిరిగి SHNode;
  }
  స్థితి కణుపు::NK_ఖండం_Scalar: {
    ఖండం_Scalar_కణుపు *BSN = dyn_cast<ఖండం_Scalar_కణుపు>(N);
    StringRef విలువ_Copy = BSN->get_విలువ().copy(StringAllocator);
    స్వయంచాలక *BSHNode =
        కొత్త (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, విలువ_Copy);
    saveAliasHNode(BSN, BSHNode);
    తిరిగి BSHNode;
  }
  స్థితి కణుపు::NK_Sequence: {
    Sequence_కణుపు *SQ = dyn_cast<Sequence_కణుపు>(N);
    స్వయంచాలక SQHNode = కొత్త (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    కొరకు (కణుపు &SN : *SQ) {
      స్వయంచాలక నమోదు_2 = createHNodes(&SN);
      ఒకవేళ (EC)
        ఆపు;
      SQHNode->Entries.నెట్టు_వెనుక(నమోదు_2);
    }
    saveAliasHNode(SQ, SQHNode);
    తిరిగి SQHNode;
  }
  స్థితి కణుపు::NK_Mapping: {
    Mapping_కణుపు *Map = dyn_cast<Mapping_కణుపు>(N);
    స్వయంచాలక mapHNode = కొత్త (MapHNodeAllocator.Allocate()) MapHNode(N);
    కొరకు (కీలకం_విలువ_కణుపు &KVN : *Map) {
      కణుపు *కీలకం_కణుపు = KVN.get_కీలకం();
      Scalar_కణుపు *కీలకం = dyn_cast_if_present<Scalar_కణుపు>(కీలకం_కణుపు);
      కణుపు *విలువ = KVN.get_విలువ();
      ఒకవేళ (!కీలకం || !విలువ) {
        ఒకవేళ (!కీలకం)
          సమితి_Error(కీలకం_కణుపు, "Map key must be a scalar");
        ఒకవేళ (!విలువ)
          సమితి_Error(కీలకం_కణుపు, "Map value must not be empty");
        ఆపు;
      }
      StringStorage.clear();
      StringRef కీలకం_Str = కీలకం->get_విలువ(StringStorage);
      ఒకవేళ (!StringStorage.ఖాళీలు()) {
        // Copy string to permanent storage
        కీలకం_Str = StringStorage.str().copy(StringAllocator);
      }
      ఒకవేళ (mapHNode->Mapping.లెక్క(కీలకం_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        సమితి_Error(కీలకం_కణుపు, Twine("duplicated mapping key '") + కీలకం_Str + "'");
      స్వయంచాలక విలువ_HNode = createHNodes(విలువ);
      ఒకవేళ (EC)
        ఆపు;
      mapHNode->Mapping[కీలకం_Str] =
          std::make_pair(std::move(విలువ_HNode), కీలకం_కణుపు->get_ఆధారం_Range());
    }
    saveAliasHNode(Map, mapHNode);
    తిరిగి std::move(mapHNode);
  }
  స్థితి కణుపు::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    తిరిగి కొత్త (ఖాళీలు_HNode_Allocator.Allocate()) ఖాళీలు_HNode(N);
  స్థితి కణుపు::NK_Alias: {
    Alias_కణుపు *AN = dyn_cast<Alias_కణుపు>(N);
    స్వయంచాలక AliasName = AN->getName();
    స్వయంచాలక AHN = AliasMap.కనుగొను(AliasName);
    ఒకవేళ (AHN == AliasMap.చివర()) {
      సమితి_Error(AN, Twine("undefined alias '" + AliasName + "'"));
      తిరిగి శూన్యసూచిక;
    }
    తిరిగి AHN->రెండవ_2;
  }
  అప్రమేయ:
    సమితి_Error(N, "unknown node kind");
    తిరిగి శూన్యసూచిక;
  }
}

శూన్య ఆదానం::సమితి_Error(స్థిరరాశి Twine &సందేశం_3) {
  సమితి_Error(Current_కణుపు, సందేశం_3);
}

శూన్య ఆదానం::సమితి_Allow_Unknown_కీలకాలు(తార్కిక Allow) { Allow_Unknown_కీలకాలు = Allow; }

తార్కిక ఆదానం::can_Elide_ఖాళీలు_Sequence() {
  తిరిగి అబద్ధం;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, శూన్య *context, పూర్ణాంకం Wrap_స్తంభం)
    : IO(context), బహిర్గతం(yout), Wrap_స్తంభం(Wrap_స్తంభం) {}

Output::~Output() = అప్రమేయ;

తార్కిక Output::outputting() స్థిరరాశి {
  తిరిగి నిజం;
}

శూన్య Output::beginMapping() {
  State_దొంతర.నెట్టు_వెనుక(లోపల_Map_మొదటి_కీలకం);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

తార్కిక Output::map_ట్యాగ్(StringRef ట్యాగ్, తార్కిక Use) {
  ఒకవేళ (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    తార్కిక SequenceElement = అబద్ధం;
    ఒకవేళ (State_దొంతర.కొలత_2() > 1) {
      స్వయంచాలక &E = State_దొంతర[State_దొంతర.కొలత_2() - 2];
      SequenceElement = లోపల_Seq_Any_Element(E) || లోపల_Flow_Seq_Any_Element(E);
    }
    ఒకవేళ (SequenceElement && State_దొంతర.వెనుక() == లోపల_Map_మొదటి_కీలకం) {
      new_పంక్తి_Check();
    } లేకపోతే {
      output(" ");
    }
    output(ట్యాగ్);
    ఒకవేళ (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      ఒకవేళ (State_దొంతర.వెనుక() == లోపల_Map_మొదటి_కీలకం) {
        State_దొంతర.తీయి_వెనుక();
        State_దొంతర.నెట్టు_వెనుక(లోపల_Map_Other_కీలకం);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  తిరిగి Use;
}

శూన్య Output::చివర_Mapping() {
  // If we did not map anything, we should explicitly emit an empty map
  ఒకవేళ (State_దొంతర.వెనుక() == లోపల_Map_మొదటి_కీలకం) {
    Padding = PaddingBeforeContainer;
    new_పంక్తి_Check();
    output("{}");
    Padding = "\n";
  }
  State_దొంతర.తీయి_వెనుక();
}

std::సదిశ<StringRef> Output::కీలకాలు() {
  report_fatal_error("invalid call");
}

తార్కిక Output::preflight_కీలకం(StringRef కీలకం, తార్కిక Required, తార్కిక SameAsDefault,
                          తార్కిక &UseDefault, శూన్య *&SaveInfo) {
  UseDefault = అబద్ధం;
  SaveInfo = శూన్యసూచిక;
  ఒకవేళ (Required || !SameAsDefault || Write_Default_విలువలు) {
    స్వయంచాలక State = State_దొంతర.వెనుక();
    ఒకవేళ (State == లోపల_Flow_Map_మొదటి_కీలకం || State == లోపల_Flow_Map_Other_కీలకం) {
      flow_కీలకం(కీలకం);
    } లేకపోతే {
      new_పంక్తి_Check();
      padded_కీలకం(కీలకం);
    }
    తిరిగి నిజం;
  }
  తిరిగి అబద్ధం;
}

శూన్య Output::postflight_కీలకం(శూన్య *) {
  ఒకవేళ (State_దొంతర.వెనుక() == లోపల_Map_మొదటి_కీలకం) {
    State_దొంతర.తీయి_వెనుక();
    State_దొంతర.నెట్టు_వెనుక(లోపల_Map_Other_కీలకం);
  } లేకపోతే ఒకవేళ (State_దొంతర.వెనుక() == లోపల_Flow_Map_మొదటి_కీలకం) {
    State_దొంతర.తీయి_వెనుక();
    State_దొంతర.నెట్టు_వెనుక(లోపల_Flow_Map_Other_కీలకం);
  }
}

శూన్య Output::beginFlowMapping() {
  State_దొంతర.నెట్టు_వెనుక(లోపల_Flow_Map_మొదటి_కీలకం);
  new_పంక్తి_Check();
  స్తంభం_At_Map_Flow_ప్రారంభం = స్తంభం;
  output("{ ");
}

శూన్య Output::చివర_Flow_Mapping() {
  State_దొంతర.తీయి_వెనుక();
  output_పైకి_గమ్యం_చివర_యొక్క_పంక్తి(" }");
}

శూన్య Output::beginDocuments() {
  output_పైకి_గమ్యం_చివర_యొక్క_పంక్తి("---");
}

తార్కిక Output::preflightDocument(గుర్తులేని సూచీ) {
  ఒకవేళ (సూచీ > 0)
    output_పైకి_గమ్యం_చివర_యొక్క_పంక్తి("\n---");
  తిరిగి నిజం;
}

శూన్య Output::postflightDocument() {
}

శూన్య Output::చివర_Documents() {
  output("\n...\n");
}

గుర్తులేని Output::beginSequence() {
  State_దొంతర.నెట్టు_వెనుక(లోపల_Seq_మొదటి_Element);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  తిరిగి 0;
}

శూన్య Output::చివర_Sequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  ఒకవేళ (State_దొంతర.వెనుక() == లోపల_Seq_మొదటి_Element) {
    Padding = PaddingBeforeContainer;
    new_పంక్తి_Check(/*EmptySequence=*/నిజం);
    output("[]");
    Padding = "\n";
  }
  State_దొంతర.తీయి_వెనుక();
}

తార్కిక Output::preflightElement(గుర్తులేని, శూన్య *&SaveInfo) {
  SaveInfo = శూన్యసూచిక;
  తిరిగి నిజం;
}

శూన్య Output::postflightElement(శూన్య *) {
  ఒకవేళ (State_దొంతర.వెనుక() == లోపల_Seq_మొదటి_Element) {
    State_దొంతర.తీయి_వెనుక();
    State_దొంతర.నెట్టు_వెనుక(లోపల_Seq_Other_Element);
  } లేకపోతే ఒకవేళ (State_దొంతర.వెనుక() == లోపల_Flow_Seq_మొదటి_Element) {
    State_దొంతర.తీయి_వెనుక();
    State_దొంతర.నెట్టు_వెనుక(లోపల_Flow_Seq_Other_Element);
  }
}

గుర్తులేని Output::beginFlowSequence() {
  State_దొంతర.నెట్టు_వెనుక(లోపల_Flow_Seq_మొదటి_Element);
  new_పంక్తి_Check();
  స్తంభం_At_Flow_ప్రారంభం = స్తంభం;
  output("[ ");
  NeedFlowSequenceComma = అబద్ధం;
  తిరిగి 0;
}

శూన్య Output::చివర_Flow_Sequence() {
  State_దొంతర.తీయి_వెనుక();
  output_పైకి_గమ్యం_చివర_యొక్క_పంక్తి(" ]");
}

తార్కిక Output::preflightFlowElement(గుర్తులేని, శూన్య *&SaveInfo) {
  ఒకవేళ (NeedFlowSequenceComma)
    output(", ");
  ఒకవేళ (Wrap_స్తంభం && స్తంభం > Wrap_స్తంభం) {
    output("\n");
    కొరకు (పూర్ణాంకం i = 0; i < స్తంభం_At_Flow_ప్రారంభం; ++i)
      output(" ");
    స్తంభం = స్తంభం_At_Flow_ప్రారంభం;
    output("  ");
  }
  SaveInfo = శూన్యసూచిక;
  తిరిగి నిజం;
}

శూన్య Output::postflightFlowElement(శూన్య *) {
  NeedFlowSequenceComma = నిజం;
}

శూన్య Output::beginEnumScalar() {
  Enumeration_సరిపోలిక_దొరికింది = అబద్ధం;
}

తార్కిక Output::సరిపోలిక_Enum_Scalar(StringRef Str, తార్కిక సరిపోలిక_2) {
  ఒకవేళ (సరిపోలిక_2 && !Enumeration_సరిపోలిక_దొరికింది) {
    new_పంక్తి_Check();
    output_పైకి_గమ్యం_చివర_యొక్క_పంక్తి(Str);
    Enumeration_సరిపోలిక_దొరికింది = నిజం;
  }
  తిరిగి అబద్ధం;
}

తార్కిక Output::సరిపోలిక_Enum_Fallback() {
  ఒకవేళ (Enumeration_సరిపోలిక_దొరికింది)
    తిరిగి అబద్ధం;
  Enumeration_సరిపోలిక_దొరికింది = నిజం;
  తిరిగి నిజం;
}

శూన్య Output::చివర_Enum_Scalar() {
  ఒకవేళ (!Enumeration_సరిపోలిక_దొరికింది)
    llvm_unreachable("bad runtime enum value");
}

తార్కిక Output::begin_బిట్టు_సమితి_Scalar(తార్కిక &DoClear) {
  new_పంక్తి_Check();
  output("[ ");
  Need_బిట్టు_విలువ_Comma = అబద్ధం;
  DoClear = అబద్ధం;
  తిరిగి నిజం;
}

తార్కిక Output::బిట్టు_సమితి_సరిపోలిక(StringRef Str, తార్కిక Matches) {
  ఒకవేళ (Matches) {
    ఒకవేళ (Need_బిట్టు_విలువ_Comma)
      output(", ");
    output(Str);
    Need_బిట్టు_విలువ_Comma = నిజం;
  }
  తిరిగి అబద్ధం;
}

శూన్య Output::చివర_బిట్టు_సమితి_Scalar() {
  output_పైకి_గమ్యం_చివర_యొక్క_పంక్తి(" ]");
}

శూన్య Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_పంక్తి_Check();
  ఒకవేళ (S.ఖాళీలు()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_పైకి_గమ్యం_చివర_యొక్క_పంక్తి("''");
    తిరిగి;
  }
  output(S, MustQuote);
  output_పైకి_గమ్యం_చివర_యొక్క_పంక్తి("");
}

శూన్య Output::ఖండం_Scalar_String(StringRef &S) {
  ఒకవేళ (!State_దొంతర.ఖాళీలు())
    new_పంక్తి_Check();
  output(" |");

  గుర్తులేని Indent = State_దొంతర.ఖాళీలు() ? 1 : State_దొంతర.కొలత_2();

  స్వయంచాలక బఫరు = జ్ఞాపకం_బఫరు::get_Mem_బఫరు(S, "", అబద్ధం);
  కొరకు (పంక్తి_iterator పంక్తులు(*బఫరు, అబద్ధం); !పంక్తులు.is_at_చివర(); ++పంక్తులు) {
    output_New_పంక్తి();
    కొరకు (గుర్తులేని I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*పంక్తులు);
  }
  output_పైకి_గమ్యం_చివర_యొక్క_పంక్తి("");
}

శూన్య Output::scalar_ట్యాగ్(std::string &ట్యాగ్) {
  ఒకవేళ (ట్యాగ్.ఖాళీలు())
    తిరిగి;
  new_పంక్తి_Check();
  output(ట్యాగ్);
  output(" ");
}

శూన్య Output::సమితి_Error(స్థిరరాశి Twine &సందేశం_2) {
}

std::error_సంకేతం Output::error() { తిరిగి {}; }

తార్కిక Output::can_Elide_ఖాళీలు_Sequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  ఒకవేళ (State_దొంతర.కొలత_2() < 2)
    తిరిగి నిజం;
  ఒకవేళ (State_దొంతర.వెనుక() != లోపల_Map_మొదటి_కీలకం)
    తిరిగి నిజం;
  తిరిగి !లోపల_Seq_Any_Element(State_దొంతర[State_దొంతర.కొలత_2() - 2]);
}

శూన్య Output::output(StringRef s) {
  స్తంభం += s.కొలత_2();
  బహిర్గతం << s;
}

శూన్య Output::output(StringRef S, QuotingType MustQuote) {
  ఒకవేళ (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    తిరిగి;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  ఒకవేళ (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ అబద్ధం));
    output(Quote);
    తిరిగి;
  }

  గుర్తులేని i = 0;
  గుర్తులేని j = 0;
  గుర్తులేని చివర_2 = S.కొలత_2();
  స్థిరరాశి అక్షరం *పునాది = S.దత్తాంశం();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  అయితే (j < చివర_2) {
    ఒకవేళ (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&పునాది[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&పునాది[i], j - i));
  output(Quote); // Ending quote.
}

శూన్య Output::output_పైకి_గమ్యం_చివర_యొక్క_పంక్తి(StringRef s) {
  output(s);
  ఒకవేళ (State_దొంతర.ఖాళీలు() || (!లోపల_Flow_Seq_Any_Element(State_దొంతర.వెనుక()) &&
                             !లోపల_Flow_Map_Any_కీలకం(State_దొంతర.వెనుక())))
    Padding = "\n";
}

శూన్య Output::output_New_పంక్తి() {
  బహిర్గతం << "\n";
  స్తంభం = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

శూన్య Output::new_పంక్తి_Check(తార్కిక ఖాళీలు_Sequence) {
  ఒకవేళ (Padding != "\n") {
    output(Padding);
    Padding = {};
    తిరిగి;
  }
  output_New_పంక్తి();
  Padding = {};

  ఒకవేళ (State_దొంతర.కొలత_2() == 0 || ఖాళీలు_Sequence)
    తిరిగి;

  గుర్తులేని Indent = State_దొంతర.కొలత_2() - 1;
  తార్కిక PossiblyNestedSeq = అబద్ధం;
  స్వయంచాలక I = State_దొంతర.rbegin(), E = State_దొంతర.rend();

  ఒకవేళ (లోపల_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = నిజం; // Not possibly but always.
    ++Indent;
  } లేకపోతే ఒకవేళ (*I == లోపల_Map_మొదటి_కీలకం || *I == లోపల_Flow_Map_మొదటి_కీలకం ||
             లోపల_Flow_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = నిజం;
    ++I; // Skip back().
  }

  గుర్తులేని Output_Dash_లెక్క = 0;
  ఒకవేళ (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    అయితే (I != E) {
      // Don't count the top of nested sequence.
      ఒకవేళ (!లోపల_Seq_Any_Element(*I))
        ఆపు;

      ++Output_Dash_లెక్క;

      // Stop counting if consecutive inSeqFirstElement ends.
      ఒకవేళ (*I++ != లోపల_Seq_మొదటి_Element)
        ఆపు;
    }
  }

  కొరకు (గుర్తులేని I = Output_Dash_లెక్క; I < Indent; ++I)
    output("  ");

  కొరకు (గుర్తులేని I = 0; I < Output_Dash_లెక్క; ++I)
    output("- ");
}

శూన్య Output::padded_కీలకం(StringRef కీలకం_2) {
  output(కీలకం_2, needsQuotes(కీలకం_2, అబద్ధం));
  output(":");
  స్థిరరాశి అక్షరం *spaces = "                ";
  ఒకవేళ (కీలకం_2.కొలత_2() < అక్షరశ్రేణి_పొడవు(spaces))
    Padding = &spaces[కీలకం_2.కొలత_2()];
  లేకపోతే
    Padding = " ";
}

శూన్య Output::flow_కీలకం(StringRef కీలకం) {
  ఒకవేళ (State_దొంతర.వెనుక() == లోపల_Flow_Map_Other_కీలకం)
    output(", ");
  ఒకవేళ (Wrap_స్తంభం && స్తంభం > Wrap_స్తంభం) {
    output("\n");
    కొరకు (పూర్ణాంకం I = 0; I < స్తంభం_At_Map_Flow_ప్రారంభం; ++I)
      output(" ");
    స్తంభం = స్తంభం_At_Map_Flow_ప్రారంభం;
    output("  ");
  }
  output(కీలకం, needsQuotes(కీలకం, అబద్ధం));
  output(": ");
}

కణుపు_రకం Output::get_కణుపు_రకం() { report_fatal_error("invalid call"); }

తార్కిక Output::లోపల_Seq_Any_Element(లోపల_State State) {
  తిరిగి State == లోపల_Seq_మొదటి_Element || State == లోపల_Seq_Other_Element;
}

తార్కిక Output::లోపల_Flow_Seq_Any_Element(లోపల_State State) {
  తిరిగి State == లోపల_Flow_Seq_మొదటి_Element || State == లోపల_Flow_Seq_Other_Element;
}

తార్కిక Output::లోపల_Map_Any_కీలకం(లోపల_State State) {
  తిరిగి State == లోపల_Map_మొదటి_కీలకం || State == లోపల_Map_Other_కీలకం;
}

తార్కిక Output::లోపల_Flow_Map_Any_కీలకం(లోపల_State State) {
  తిరిగి State == లోపల_Flow_Map_మొదటి_కీలకం || State == లోపల_Flow_Map_Other_కీలకం;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

శూన్య ScalarTraits<తార్కిక>::output(స్థిరరాశి తార్కిక &Val, శూన్య *, raw_ostream &బహిర్గతం) {
  బహిర్గతం << (Val ? "true" : "false");
}

StringRef ScalarTraits<తార్కిక>::ఆదానం_2(StringRef Scalar, శూన్య *, తార్కిక &Val) {
  ఒకవేళ (std::optional<తార్కిక> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    తిరిగి StringRef();
  }
  తిరిగి "invalid boolean";
}

శూన్య ScalarTraits<StringRef>::output(స్థిరరాశి StringRef &Val, శూన్య *,
                                     raw_ostream &బహిర్గతం) {
  బహిర్గతం << Val;
}

StringRef ScalarTraits<StringRef>::ఆదానం_2(StringRef Scalar, శూన్య *,
                                         StringRef &Val) {
  Val = Scalar;
  తిరిగి StringRef();
}

శూన్య ScalarTraits<std::string>::output(స్థిరరాశి std::string &Val, శూన్య *,
                                       raw_ostream &బహిర్గతం) {
  బహిర్గతం << Val;
}

StringRef ScalarTraits<std::string>::ఆదానం_2(StringRef Scalar, శూన్య *,
                                           std::string &Val) {
  Val = Scalar.str();
  తిరిగి StringRef();
}

శూన్య ScalarTraits<uint8_t>::output(స్థిరరాశి uint8_t &Val, శూన్య *,
                                   raw_ostream &బహిర్గతం) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t అంకె = Val;
  బహిర్గతం << అంకె;
}

StringRef ScalarTraits<uint8_t>::ఆదానం_2(StringRef Scalar, శూన్య *, uint8_t &Val) {
  గుర్తులేని దీర్ఘ దీర్ఘ n;
  ఒకవేళ (getAsUnsignedInteger(Scalar, 0, n))
    తిరిగి "invalid number";
  ఒకవేళ (n > 0xFF)
    తిరిగి "out of range number";
  Val = n;
  తిరిగి StringRef();
}

శూన్య ScalarTraits<uint16_t>::output(స్థిరరాశి uint16_t &Val, శూన్య *,
                                    raw_ostream &బహిర్గతం) {
  బహిర్గతం << Val;
}

StringRef ScalarTraits<uint16_t>::ఆదానం_2(StringRef Scalar, శూన్య *,
                                        uint16_t &Val) {
  గుర్తులేని దీర్ఘ దీర్ఘ n;
  ఒకవేళ (getAsUnsignedInteger(Scalar, 0, n))
    తిరిగి "invalid number";
  ఒకవేళ (n > 0xFFFF)
    తిరిగి "out of range number";
  Val = n;
  తిరిగి StringRef();
}

శూన్య ScalarTraits<uint32_t>::output(స్థిరరాశి uint32_t &Val, శూన్య *,
                                    raw_ostream &బహిర్గతం) {
  బహిర్గతం << Val;
}

StringRef ScalarTraits<uint32_t>::ఆదానం_2(StringRef Scalar, శూన్య *,
                                        uint32_t &Val) {
  గుర్తులేని దీర్ఘ దీర్ఘ n;
  ఒకవేళ (getAsUnsignedInteger(Scalar, 0, n))
    తిరిగి "invalid number";
  ఒకవేళ (n > 0xFFFFFFFFUL)
    తిరిగి "out of range number";
  Val = n;
  తిరిగి StringRef();
}

శూన్య ScalarTraits<uint64_t>::output(స్థిరరాశి uint64_t &Val, శూన్య *,
                                    raw_ostream &బహిర్గతం) {
  బహిర్గతం << Val;
}

StringRef ScalarTraits<uint64_t>::ఆదానం_2(StringRef Scalar, శూన్య *,
                                        uint64_t &Val) {
  గుర్తులేని దీర్ఘ దీర్ఘ N;
  ఒకవేళ (getAsUnsignedInteger(Scalar, 0, N))
    తిరిగి "invalid number";
  Val = N;
  తిరిగి StringRef();
}

శూన్య ScalarTraits<int8_t>::output(స్థిరరాశి int8_t &Val, శూన్య *, raw_ostream &బహిర్గతం) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t అంకె = Val;
  బహిర్గతం << అంకె;
}

StringRef ScalarTraits<int8_t>::ఆదానం_2(StringRef Scalar, శూన్య *, int8_t &Val) {
  దీర్ఘ దీర్ఘ N;
  ఒకవేళ (getAsSignedInteger(Scalar, 0, N))
    తిరిగి "invalid number";
  ఒకవేళ ((N > 127) || (N < -128))
    తిరిగి "out of range number";
  Val = N;
  తిరిగి StringRef();
}

శూన్య ScalarTraits<int16_t>::output(స్థిరరాశి int16_t &Val, శూన్య *,
                                   raw_ostream &బహిర్గతం) {
  బహిర్గతం << Val;
}

StringRef ScalarTraits<int16_t>::ఆదానం_2(StringRef Scalar, శూన్య *, int16_t &Val) {
  దీర్ఘ దీర్ఘ N;
  ఒకవేళ (getAsSignedInteger(Scalar, 0, N))
    తిరిగి "invalid number";
  ఒకవేళ ((N > INT16_MAX) || (N < INT16_MIN))
    తిరిగి "out of range number";
  Val = N;
  తిరిగి StringRef();
}

శూన్య ScalarTraits<int32_t>::output(స్థిరరాశి int32_t &Val, శూన్య *,
                                   raw_ostream &బహిర్గతం) {
  బహిర్గతం << Val;
}

StringRef ScalarTraits<int32_t>::ఆదానం_2(StringRef Scalar, శూన్య *, int32_t &Val) {
  దీర్ఘ దీర్ఘ N;
  ఒకవేళ (getAsSignedInteger(Scalar, 0, N))
    తిరిగి "invalid number";
  ఒకవేళ ((N > INT32_MAX) || (N < INT32_MIN))
    తిరిగి "out of range number";
  Val = N;
  తిరిగి StringRef();
}

శూన్య ScalarTraits<int64_t>::output(స్థిరరాశి int64_t &Val, శూన్య *,
                                   raw_ostream &బహిర్గతం) {
  బహిర్గతం << Val;
}

StringRef ScalarTraits<int64_t>::ఆదానం_2(StringRef Scalar, శూన్య *, int64_t &Val) {
  దీర్ఘ దీర్ఘ N;
  ఒకవేళ (getAsSignedInteger(Scalar, 0, N))
    తిరిగి "invalid number";
  Val = N;
  తిరిగి StringRef();
}

శూన్య ScalarTraits<ద్విగుణ>::output(స్థిరరాశి ద్విగుణ &Val, శూన్య *, raw_ostream &బహిర్గతం) {
  బహిర్గతం << format("%g", Val);
}

StringRef ScalarTraits<ద్విగుణ>::ఆదానం_2(StringRef Scalar, శూన్య *, ద్విగుణ &Val) {
  ఒకవేళ (గమ్యం_float(Scalar, Val))
    తిరిగి StringRef();
  తిరిగి "invalid floating point number";
}

శూన్య ScalarTraits<దశాంశం>::output(స్థిరరాశి దశాంశం &Val, శూన్య *, raw_ostream &బహిర్గతం) {
  బహిర్గతం << format("%g", Val);
}

StringRef ScalarTraits<దశాంశం>::ఆదానం_2(StringRef Scalar, శూన్య *, దశాంశం &Val) {
  ఒకవేళ (గమ్యం_float(Scalar, Val))
    తిరిగి StringRef();
  తిరిగి "invalid floating point number";
}

శూన్య ScalarTraits<Hex8>::output(స్థిరరాశి Hex8 &Val, శూన్య *, raw_ostream &బహిర్గతం) {
  బహిర్గతం << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::ఆదానం_2(StringRef Scalar, శూన్య *, Hex8 &Val) {
  గుర్తులేని దీర్ఘ దీర్ఘ n;
  ఒకవేళ (getAsUnsignedInteger(Scalar, 0, n))
    తిరిగి "invalid hex8 number";
  ఒకవేళ (n > 0xFF)
    తిరిగి "out of range hex8 number";
  Val = n;
  తిరిగి StringRef();
}

శూన్య ScalarTraits<Hex16>::output(స్థిరరాశి Hex16 &Val, శూన్య *, raw_ostream &బహిర్గతం) {
  బహిర్గతం << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::ఆదానం_2(StringRef Scalar, శూన్య *, Hex16 &Val) {
  గుర్తులేని దీర్ఘ దీర్ఘ n;
  ఒకవేళ (getAsUnsignedInteger(Scalar, 0, n))
    తిరిగి "invalid hex16 number";
  ఒకవేళ (n > 0xFFFF)
    తిరిగి "out of range hex16 number";
  Val = n;
  తిరిగి StringRef();
}

శూన్య ScalarTraits<Hex32>::output(స్థిరరాశి Hex32 &Val, శూన్య *, raw_ostream &బహిర్గతం) {
  బహిర్గతం << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::ఆదానం_2(StringRef Scalar, శూన్య *, Hex32 &Val) {
  గుర్తులేని దీర్ఘ దీర్ఘ n;
  ఒకవేళ (getAsUnsignedInteger(Scalar, 0, n))
    తిరిగి "invalid hex32 number";
  ఒకవేళ (n > 0xFFFFFFFFUL)
    తిరిగి "out of range hex32 number";
  Val = n;
  తిరిగి StringRef();
}

శూన్య ScalarTraits<Hex64>::output(స్థిరరాశి Hex64 &Val, శూన్య *, raw_ostream &బహిర్గతం) {
  బహిర్గతం << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::ఆదానం_2(StringRef Scalar, శూన్య *, Hex64 &Val) {
  గుర్తులేని దీర్ఘ దీర్ఘ అంకె;
  ఒకవేళ (getAsUnsignedInteger(Scalar, 0, అంకె))
    తిరిగి "invalid hex64 number";
  Val = అంకె;
  తిరిగి StringRef();
}

శూన్య ScalarTraits<సంస్కరణ_Tuple>::output(స్థిరరాశి సంస్కరణ_Tuple &Val, శూన్య *,
                                        llvm::raw_ostream &బహిర్గతం) {
  బహిర్గతం << Val.getAsString();
}

StringRef ScalarTraits<సంస్కరణ_Tuple>::ఆదానం_2(StringRef Scalar, శూన్య *,
                                            సంస్కరణ_Tuple &Val) {
  ఒకవేళ (Val.tryParse(Scalar))
    తిరిగి "invalid version format";
  తిరిగి StringRef();
}
