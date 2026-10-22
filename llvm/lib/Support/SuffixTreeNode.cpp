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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SuffixTreeNode.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਬਿਨਾਨਿਸ਼ਾਨੀ Suffix_ਰੁੱਖ_ਗੰਢ::get_ਸ਼ੁਰੂ_Idx() ਅਚਲ { ਵਾਪਸ ਸ਼ੁਰੂ_Idx; }
ਖਾਲੀ Suffix_ਰੁੱਖ_ਗੰਢ::increment_ਸ਼ੁਰੂ_Idx(ਬਿਨਾਨਿਸ਼ਾਨੀ Inc) { ਸ਼ੁਰੂ_Idx += Inc; }
ਖਾਲੀ Suffix_ਰੁੱਖ_ਗੰਢ::ਸੈੱਟ_Concat_Len(ਬਿਨਾਨਿਸ਼ਾਨੀ Len) { ConcatLen = Len; }
ਬਿਨਾਨਿਸ਼ਾਨੀ Suffix_ਰੁੱਖ_ਗੰਢ::getConcatLen() ਅਚਲ { ਵਾਪਸ ConcatLen; }

ਤਾਰਕਿਕ Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ::is_ਜੜ੍ਹ() ਅਚਲ {
  ਵਾਪਸ get_ਸ਼ੁਰੂ_Idx() == ਖਾਲੀ_ਥਾਵਾਂ_Idx;
}
ਬਿਨਾਨਿਸ਼ਾਨੀ Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ::get_ਸਿਰਾ_Idx() ਅਚਲ { ਵਾਪਸ ਸਿਰਾ_Idx; }
ਖਾਲੀ Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ::ਸੈੱਟ_Link(Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ *L) {
  ਪ੍ਰਤਿਜ੍ਞਾ(L && "Cannot set a null link?");
  Link = L;
}
Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ *Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ::getLink() ਅਚਲ { ਵਾਪਸ Link; }

ਬਿਨਾਨਿਸ਼ਾਨੀ Suffix_ਰੁੱਖ_ਪੱਤੀ_ਗੰਢ::get_ਸਿਰਾ_Idx() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਸਿਰਾ_Idx && "EndIdx is empty?");
  ਵਾਪਸ *ਸਿਰਾ_Idx;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ Suffix_ਰੁੱਖ_ਪੱਤੀ_ਗੰਢ::getSuffixIdx() ਅਚਲ { ਵਾਪਸ SuffixIdx; }
ਖਾਲੀ Suffix_ਰੁੱਖ_ਪੱਤੀ_ਗੰਢ::ਸੈੱਟ_Suffix_Idx(ਬਿਨਾਨਿਸ਼ਾਨੀ Idx) { SuffixIdx = Idx; }

ਬਿਨਾਨਿਸ਼ਾਨੀ Suffix_ਰੁੱਖ_ਗੰਢ::get_ਖੱਬਾ_ਪੱਤੀ_Idx() ਅਚਲ { ਵਾਪਸ ਖੱਬਾ_ਪੱਤੀ_Idx; }
ਬਿਨਾਨਿਸ਼ਾਨੀ Suffix_ਰੁੱਖ_ਗੰਢ::get_ਸੱਜਾ_ਪੱਤੀ_Idx() ਅਚਲ { ਵਾਪਸ ਸੱਜਾ_ਪੱਤੀ_Idx; }
ਖਾਲੀ Suffix_ਰੁੱਖ_ਗੰਢ::ਸੈੱਟ_ਖੱਬਾ_ਪੱਤੀ_Idx(ਬਿਨਾਨਿਸ਼ਾਨੀ Idx) { ਖੱਬਾ_ਪੱਤੀ_Idx = Idx; }
ਖਾਲੀ Suffix_ਰੁੱਖ_ਗੰਢ::ਸੈੱਟ_ਸੱਜਾ_ਪੱਤੀ_Idx(ਬਿਨਾਨਿਸ਼ਾਨੀ Idx) { ਸੱਜਾ_ਪੱਤੀ_Idx = Idx; }
