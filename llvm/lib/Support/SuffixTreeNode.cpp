//===- llvm/ADT/SuffixTreeNode.cpp - Nodes for SuffixTrees --------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines nodes for use within a SuffixTree.
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/SuffixTreeNode.h"

ఉపయోగించి నామాకాశ llvm;

గుర్తులేని Suffix_వృక్షం_కణుపు::get_ప్రారంభం_Idx() స్థిరరాశి { తిరిగి ప్రారంభం_Idx; }
శూన్య Suffix_వృక్షం_కణుపు::increment_ప్రారంభం_Idx(గుర్తులేని Inc) { ప్రారంభం_Idx += Inc; }
శూన్య Suffix_వృక్షం_కణుపు::సమితి_Concat_Len(గుర్తులేని Len) { ConcatLen = Len; }
గుర్తులేని Suffix_వృక్షం_కణుపు::getConcatLen() స్థిరరాశి { తిరిగి ConcatLen; }

తార్కిక Suffix_వృక్షం_అంతర్గత_కణుపు::is_మూలం() స్థిరరాశి {
  తిరిగి get_ప్రారంభం_Idx() == ఖాళీలు_Idx;
}
గుర్తులేని Suffix_వృక్షం_అంతర్గత_కణుపు::get_చివర_Idx() స్థిరరాశి { తిరిగి చివర_Idx; }
శూన్య Suffix_వృక్షం_అంతర్గత_కణుపు::సమితి_Link(Suffix_వృక్షం_అంతర్గత_కణుపు *L) {
  ప్రతిజ్ఞా(L && "Cannot set a null link?");
  Link = L;
}
Suffix_వృక్షం_అంతర్గత_కణుపు *Suffix_వృక్షం_అంతర్గత_కణుపు::getLink() స్థిరరాశి { తిరిగి Link; }

గుర్తులేని Suffix_వృక్షం_ఆకు_కణుపు::get_చివర_Idx() స్థిరరాశి {
  ప్రతిజ్ఞా(చివర_Idx && "EndIdx is empty?");
  తిరిగి *చివర_Idx;
}

గుర్తులేని Suffix_వృక్షం_ఆకు_కణుపు::getSuffixIdx() స్థిరరాశి { తిరిగి SuffixIdx; }
శూన్య Suffix_వృక్షం_ఆకు_కణుపు::సమితి_Suffix_Idx(గుర్తులేని Idx) { SuffixIdx = Idx; }

గుర్తులేని Suffix_వృక్షం_కణుపు::get_ఎడమ_ఆకు_Idx() స్థిరరాశి { తిరిగి ఎడమ_ఆకు_Idx; }
గుర్తులేని Suffix_వృక్షం_కణుపు::get_కుడి_ఆకు_Idx() స్థిరరాశి { తిరిగి కుడి_ఆకు_Idx; }
శూన్య Suffix_వృక్షం_కణుపు::సమితి_ఎడమ_ఆకు_Idx(గుర్తులేని Idx) { ఎడమ_ఆకు_Idx = Idx; }
శూన్య Suffix_వృక్షం_కణుపు::సమితి_కుడి_ఆకు_Idx(గుర్తులేని Idx) { కుడి_ఆకు_Idx = Idx; }
