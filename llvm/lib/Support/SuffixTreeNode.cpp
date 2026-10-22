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

#ಅನ್ತರ್ಭಾವ "llvm/Support/SuffixTreeNode.h"

using ನಾಮಾಕಾಶ llvm;

unsigned Suffix_ವೃಕ್ಷ_ಗಂಟು::getStartIdx() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ StartIdx; }
ಶೂನ್ಯ Suffix_ವೃಕ್ಷ_ಗಂಟು::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
ಶೂನ್ಯ Suffix_ವೃಕ್ಷ_ಗಂಟು::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_ವೃಕ್ಷ_ಗಂಟು::getConcatLen() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ ConcatLen; }

ತಾರ್ಕಿಕ Suffix_ವೃಕ್ಷ_Internal_ಗಂಟು::is_ಬೇರು() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ getStartIdx() == EmptyIdx;
}
unsigned Suffix_ವೃಕ್ಷ_Internal_ಗಂಟು::getEndIdx() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ EndIdx; }
ಶೂನ್ಯ Suffix_ವೃಕ್ಷ_Internal_ಗಂಟು::setLink(Suffix_ವೃಕ್ಷ_Internal_ಗಂಟು *L) {
  ಪ್ರತಿಜ್ಞಾ(L && "Cannot set a null link?");
  Link = L;
}
Suffix_ವೃಕ್ಷ_Internal_ಗಂಟು *Suffix_ವೃಕ್ಷ_Internal_ಗಂಟು::getLink() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Link; }

unsigned Suffix_ವೃಕ್ಷ_Leaf_ಗಂಟು::getEndIdx() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(EndIdx && "EndIdx is empty?");
  ಪ್ರತ್ಯಾಗಮ *EndIdx;
}

unsigned Suffix_ವೃಕ್ಷ_Leaf_ಗಂಟು::getSuffixIdx() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ SuffixIdx; }
ಶೂನ್ಯ Suffix_ವೃಕ್ಷ_Leaf_ಗಂಟು::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_ವೃಕ್ಷ_ಗಂಟು::get_ಎಡ_Leaf_Idx() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ ಎಡ_Leaf_Idx; }
unsigned Suffix_ವೃಕ್ಷ_ಗಂಟು::get_ಬಲ_Leaf_Idx() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ ಬಲ_Leaf_Idx; }
ಶೂನ್ಯ Suffix_ವೃಕ್ಷ_ಗಂಟು::set_ಎಡ_Leaf_Idx(unsigned Idx) { ಎಡ_Leaf_Idx = Idx; }
ಶೂನ್ಯ Suffix_ವೃಕ್ಷ_ಗಂಟು::set_ಬಲ_Leaf_Idx(unsigned Idx) { ಬಲ_Leaf_Idx = Idx; }
