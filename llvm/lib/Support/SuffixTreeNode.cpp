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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಚಿಹ್ನೆರಹಿತ Suffix_ವೃಕ್ಷ_ಗಂಟು::get_ಆರಂಭ_Idx() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಆರಂಭ_Idx; }
ಶೂನ್ಯ Suffix_ವೃಕ್ಷ_ಗಂಟು::increment_ಆರಂಭ_Idx(ಚಿಹ್ನೆರಹಿತ Inc) { ಆರಂಭ_Idx += Inc; }
ಶೂನ್ಯ Suffix_ವೃಕ್ಷ_ಗಂಟು::ಗಣ_Concat_Len(ಚಿಹ್ನೆರಹಿತ Len) { ConcatLen = Len; }
ಚಿಹ್ನೆರಹಿತ Suffix_ವೃಕ್ಷ_ಗಂಟು::getConcatLen() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ConcatLen; }

ತಾರ್ಕಿಕ Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು::is_ಬೇರು() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ get_ಆರಂಭ_Idx() == ಖಾಲಿಗಳು_Idx;
}
ಚಿಹ್ನೆರಹಿತ Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು::get_ಕೊನೆ_Idx() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಕೊನೆ_Idx; }
ಶೂನ್ಯ Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು::ಗಣ_Link(Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು *L) {
  ಪ್ರತಿಜ್ಞಾ(L && "Cannot set a null link?");
  Link = L;
}
Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು *Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು::getLink() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Link; }

ಚಿಹ್ನೆರಹಿತ Suffix_ವೃಕ್ಷ_ಎಲೆ_ಗಂಟು::get_ಕೊನೆ_Idx() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಕೊನೆ_Idx && "EndIdx is empty?");
  ಹಿಂತಿರುಗಿ *ಕೊನೆ_Idx;
}

ಚಿಹ್ನೆರಹಿತ Suffix_ವೃಕ್ಷ_ಎಲೆ_ಗಂಟು::getSuffixIdx() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ SuffixIdx; }
ಶೂನ್ಯ Suffix_ವೃಕ್ಷ_ಎಲೆ_ಗಂಟು::ಗಣ_Suffix_Idx(ಚಿಹ್ನೆರಹಿತ Idx) { SuffixIdx = Idx; }

ಚಿಹ್ನೆರಹಿತ Suffix_ವೃಕ್ಷ_ಗಂಟು::get_ಎಡ_ಎಲೆ_Idx() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಎಡ_ಎಲೆ_Idx; }
ಚಿಹ್ನೆರಹಿತ Suffix_ವೃಕ್ಷ_ಗಂಟು::get_ಬಲ_ಎಲೆ_Idx() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಬಲ_ಎಲೆ_Idx; }
ಶೂನ್ಯ Suffix_ವೃಕ್ಷ_ಗಂಟು::ಗಣ_ಎಡ_ಎಲೆ_Idx(ಚಿಹ್ನೆರಹಿತ Idx) { ಎಡ_ಎಲೆ_Idx = Idx; }
ಶೂನ್ಯ Suffix_ವೃಕ್ಷ_ಗಂಟು::ಗಣ_ಬಲ_ಎಲೆ_Idx(ಚಿಹ್ನೆರಹಿತ Idx) { ಬಲ_ಎಲೆ_Idx = Idx; }
