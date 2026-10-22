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

#അന്തര്ഭാവ "llvm/Support/SuffixTreeNode.h"

using നാമാകാശ llvm;

unsigned Suffix_വൃക്ഷം_കണ്ണി::getStartIdx() സ്ഥിരം { തിരികെ StartIdx; }
ശൂന്യം Suffix_വൃക്ഷം_കണ്ണി::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
ശൂന്യം Suffix_വൃക്ഷം_കണ്ണി::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_വൃക്ഷം_കണ്ണി::getConcatLen() സ്ഥിരം { തിരികെ ConcatLen; }

തർക്കം Suffix_വൃക്ഷം_Internal_കണ്ണി::is_വേര്() സ്ഥിരം {
  തിരികെ getStartIdx() == EmptyIdx;
}
unsigned Suffix_വൃക്ഷം_Internal_കണ്ണി::getEndIdx() സ്ഥിരം { തിരികെ EndIdx; }
ശൂന്യം Suffix_വൃക്ഷം_Internal_കണ്ണി::setLink(Suffix_വൃക്ഷം_Internal_കണ്ണി *L) {
  പ്രതിജ്ഞാ(L && "Cannot set a null link?");
  Link = L;
}
Suffix_വൃക്ഷം_Internal_കണ്ണി *Suffix_വൃക്ഷം_Internal_കണ്ണി::getLink() സ്ഥിരം { തിരികെ Link; }

unsigned Suffix_വൃക്ഷം_Leaf_കണ്ണി::getEndIdx() സ്ഥിരം {
  പ്രതിജ്ഞാ(EndIdx && "EndIdx is empty?");
  തിരികെ *EndIdx;
}

unsigned Suffix_വൃക്ഷം_Leaf_കണ്ണി::getSuffixIdx() സ്ഥിരം { തിരികെ SuffixIdx; }
ശൂന്യം Suffix_വൃക്ഷം_Leaf_കണ്ണി::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_വൃക്ഷം_കണ്ണി::get_ഇടത്_Leaf_Idx() സ്ഥിരം { തിരികെ ഇടത്_Leaf_Idx; }
unsigned Suffix_വൃക്ഷം_കണ്ണി::get_വലത്_Leaf_Idx() സ്ഥിരം { തിരികെ വലത്_Leaf_Idx; }
ശൂന്യം Suffix_വൃക്ഷം_കണ്ണി::set_ഇടത്_Leaf_Idx(unsigned Idx) { ഇടത്_Leaf_Idx = Idx; }
ശൂന്യം Suffix_വൃക്ഷം_കണ്ണി::set_വലത്_Leaf_Idx(unsigned Idx) { വലത്_Leaf_Idx = Idx; }
