//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ಅನ್ತರ್ಭಾವ "llvm/Support/Mustache.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <sstream>

#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_TYPE "mustache"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::mustache;

ನಾಮಾಕಾಶ {

ಬಳಸಿ Accessor = ArrayRef<StringRef>;

ಸ್ಥಿರ ತಾರ್ಕಿಕ isFalsey(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &V) {
  ಹಿಂತಿರುಗಿ V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().ಮೌಲ್ಯ_2()) ||
         (V.getAsArray() && V.getAsArray()->ಖಾಲಿಗಳು());
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isContextFalsey(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  ಆದರೆ (!V)
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಹಿಂತಿರುಗಿ isFalsey(*V);
}

ಸ್ಥಿರ ಶೂನ್ಯ ವಿಭಜಿಸು_And_Trim(StringRef Str, Small_ಸದಿಶ_Impl<StringRef> &ಟೋಕನುಗಳು) {
  size_t CurrentPos = 0;
  ಆಗ (CurrentPos < Str.ಗಾತ್ರ_3()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.ಕಂಡುಹಿಡಿ('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    ಆದರೆ (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.ಗಾತ್ರ_3();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.ತುಂಡು(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t ಆರಂಭ = Part.ಕಂಡುಹಿಡಿ_ಮೊದಲ_not_ದ(" \t\r\n");
    ಆದರೆ (ಆರಂಭ != StringRef::npos) {
      size_t ಕೊನೆ_2 = Part.ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_not_ದ(" \t\r\n");
      ಟೋಕನುಗಳು.ತಳ್ಳು_ಹಿಂಭಾಗ(Part.ತುಂಡು(ಆರಂಭ, ಕೊನೆ_2 + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

ಸ್ಥಿರ Accessor ವಿಭಜಿಸು_Mustache_String(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  Small_ಸದಿಶ<StringRef> ಟೋಕನುಗಳು;
  ಆದರೆ (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    ಟೋಕನುಗಳು.ತಳ್ಳು_ಹಿಂಭಾಗ(".");
  } ಇಲ್ಲದಿದ್ದರೆ {
    ವಿಭಜಿಸು_And_Trim(Str, ಟೋಕನುಗಳು);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *Arena_ಟೋಕನುಗಳು = Ctx.Allocator.Allocate<StringRef>(ಟೋಕನುಗಳು.ಗಾತ್ರ_3());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(ಟೋಕನುಗಳು, Arena_ಟೋಕನುಗಳು);
  // Return an ArrayRef pointing to the stable arena memory.
  ಹಿಂತಿರುಗಿ ArrayRef<StringRef>(Arena_ಟೋಕನುಗಳು, ಟೋಕನುಗಳು.ಗಾತ್ರ_3());
}
} // namespace

ನಾಮಾಕಾಶ llvm::mustache {

ವರ್ಗ Mustache_Output_ಹರಿವು : ಸಾರ್ವಜನಿಕ raw_ostream {
ಸಾರ್ವಜನಿಕ:
  Mustache_Output_ಹರಿವು() = ಪೂರ್ವನಿಯೋಜಿತ;
  ~Mustache_Output_ಹರಿವು() override = ಪೂರ್ವನಿಯೋಜಿತ;

  ವಾಸ್ತವಿಕ ಶೂನ್ಯ suspendIndentation() {}
  ವಾಸ್ತವಿಕ ಶೂನ್ಯ resumeIndentation() {}

ಖಾಸಗಿ:
  ಶೂನ್ಯ anchor() override;
};

ಶೂನ್ಯ Mustache_Output_ಹರಿವು::anchor() {}

ವರ್ಗ Raw_Mustache_Output_ಹರಿವು : ಸಾರ್ವಜನಿಕ Mustache_Output_ಹರಿವು {
ಸಾರ್ವಜನಿಕ:
  Raw_Mustache_Output_ಹರಿವು(raw_ostream &OS) : OS(OS) { ಗಣ_Unbuffered(); }

ಖಾಸಗಿ:
  raw_ostream &OS;

  ಶೂನ್ಯ write_impl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) override {
    OS.ಲಿಖ(Ptr, ಗಾತ್ರ_2);
  }
  uint64_t current_pos() ಸ್ಥಿರಾಂಕ override { ಹಿಂತಿರುಗಿ OS.tell(); }
};

ವರ್ಗ Token {
ಸಾರ್ವಜನಿಕ:
  ಎಣಿಕೆ ವರ್ಗ Type {
    ಪಠ್ಯ_2,
    Variable,
    Partial,
    SectionOpen,
    SectionClose,
    InvertSectionOpen,
    UnescapeVariable,
    Comment,
    ಗಣ_Delimiter,
  };

  Token(StringRef Str)
      : TokenType(Type::ಪಠ್ಯ_2), RawBody(Str), TokenBody(RawBody),
        Accessor_ಮೌಲ್ಯ({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, ಅಕ್ಷರ Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    ಆದರೆ (TokenType == Type::Comment)
      ಹಿಂತಿರುಗಿ;
    StringRef AccessorStr(ಇದು->TokenBody);
    ಆದರೆ (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_ಮೌಲ್ಯ = ವಿಭಜಿಸು_Mustache_String(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Accessor_ಮೌಲ್ಯ; }

  Type getType() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ TokenType; }

  ಶೂನ್ಯ ಗಣ_Indentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Indentation; }

  ಸ್ಥಿರ Type getTokenType(ಅಕ್ಷರ Identifier) {
    ಆಯ್ಕೆ (Identifier) {
    ಸ್ಥಿತಿ '#':
      ಹಿಂತಿರುಗಿ Type::SectionOpen;
    ಸ್ಥಿತಿ '/':
      ಹಿಂತಿರುಗಿ Type::SectionClose;
    ಸ್ಥಿತಿ '^':
      ಹಿಂತಿರುಗಿ Type::InvertSectionOpen;
    ಸ್ಥಿತಿ '!':
      ಹಿಂತಿರುಗಿ Type::Comment;
    ಸ್ಥಿತಿ '>':
      ಹಿಂತಿರುಗಿ Type::Partial;
    ಸ್ಥಿತಿ '&':
      ಹಿಂತಿರುಗಿ Type::UnescapeVariable;
    ಸ್ಥಿತಿ '=':
      ಹಿಂತಿರುಗಿ Type::ಗಣ_Delimiter;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_ಮೌಲ್ಯ;
  size_t Indentation;
};

ಬಳಸಿ EscapeMap = DenseMap<ಅಕ್ಷರ, std::string>;

ವರ್ಗ ASTNode : ಸಾರ್ವಜನಿಕ ilist_ಗಂಟು<ASTNode> {
ಸಾರ್ವಜನಿಕ:
  ಎಣಿಕೆ Type {
    ಬೇರು_2,
    ಪಠ್ಯ_2,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::ಬೇರು_2), ಪೋಷಕ_2(ಶೂನ್ಯಸೂಚಕ), ಪೋಷಕ_Context(ಶೂನ್ಯಸೂಚಕ) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *ಪೋಷಕ_2)
      : Ctx(Ctx), Ty(Type::ಪಠ್ಯ_2), Body(Body), ಪೋಷಕ_2(ಪೋಷಕ_2),
        ಪೋಷಕ_Context(ಶೂನ್ಯಸೂಚಕ) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *ಪೋಷಕ_2)
      : Ctx(Ctx), Ty(Ty), ಪೋಷಕ_2(ಪೋಷಕ_2), Accessor_ಮೌಲ್ಯ(Accessor),
        ಪೋಷಕ_Context(ಶೂನ್ಯಸೂಚಕ) {}

  ಶೂನ್ಯ ಕೂಡಿಸು_ಮಗು(AstPtr ಮಗು) { Children.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮಗು); };

  ಶೂನ್ಯ ಗಣ_Raw_Body(StringRef NewBody) { RawBody = NewBody; };

  ಶೂನ್ಯ ಗಣ_Indentation(size_t NewIndentation) { Indentation = NewIndentation; };

  ಶೂನ್ಯ render(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ದತ್ತಾಂಶ_2, Mustache_Output_ಹರಿವು &OS);

ಖಾಸಗಿ:
  ಶೂನ್ಯ renderLambdas(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &Contexts,
                     Mustache_Output_ಹರಿವು &OS, Lambda &L);

  ಶೂನ್ಯ renderSectionLambdas(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &Contexts,
                            Mustache_Output_ಹರಿವು &OS, SectionLambda &L);

  ಶೂನ್ಯ renderPartial(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &Contexts,
                     Mustache_Output_ಹರಿವು &OS, ASTNode *Partial);

  ಶೂನ್ಯ render_ಮಗು(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &Context, Mustache_Output_ಹರಿವು &OS);

  ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ *ಕಂಡುಹಿಡಿ_Context();

  ಶೂನ್ಯ render_ಬೇರು(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &CurrentCtx, Mustache_Output_ಹರಿವು &OS);
  ಶೂನ್ಯ render_ಪಠ್ಯ(Mustache_Output_ಹರಿವು &OS);
  ಶೂನ್ಯ renderPartial(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &CurrentCtx, Mustache_Output_ಹರಿವು &OS);
  ಶೂನ್ಯ renderVariable(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &CurrentCtx, Mustache_Output_ಹರಿವು &OS);
  ಶೂನ್ಯ renderUnescapeVariable(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &CurrentCtx,
                              Mustache_Output_ಹರಿವು &OS);
  ಶೂನ್ಯ renderSection(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &CurrentCtx, Mustache_Output_ಹರಿವು &OS);
  ಶೂನ್ಯ renderInvertSection(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &CurrentCtx,
                           Mustache_Output_ಹರಿವು &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *ಪೋಷಕ_2;
  ASTNodeList Children;
  ಸ್ಥಿರಾಂಕ ArrayRef<StringRef> Accessor_ಮೌಲ್ಯ;
  ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ *ಪೋಷಕ_Context;
};

// A wrapper for arena allocator for ASTNodes
ಸ್ಥಿರ AstPtr create_ಬೇರು_ಗಂಟು(MustacheContext &Ctx) {
  ಹಿಂತಿರುಗಿ ಹೊಸ (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

ಸ್ಥಿರ AstPtr create_ಗಂಟು(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *ಪೋಷಕ_2) {
  ಹಿಂತಿರುಗಿ ಹೊಸ (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, ಪೋಷಕ_2);
}

ಸ್ಥಿರ AstPtr create_ಪಠ್ಯ_ಗಂಟು(MustacheContext &Ctx, StringRef Body,
                             ASTNode *ಪೋಷಕ_2) {
  ಹಿಂತಿರುಗಿ ಹೊಸ (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, ಪೋಷಕ_2);
}

// Function to check if there is meaningful text behind.
// We determine if a token has meaningful text behind
// if the right of previous token contains anything that is
// not a newline.
// For example:
//  "Stuff {{#Section}}" (returns true)
//   vs
//  "{{#Section}} \n" (returns false)
// We make an exception for when previous token is empty
// and the current token is the second token.
// For example:
//  "{{#Section}}"
ಸ್ಥಿರ ತಾರ್ಕಿಕ has_ಪಠ್ಯ_Behind(size_t Idx, ಸ್ಥಿರಾಂಕ ArrayRef<Token> &ಟೋಕನುಗಳು) {
  ಆದರೆ (Idx == 0)
    ಹಿಂತಿರುಗಿ ನಿಜ;

  size_t PrevIdx = Idx - 1;
  ಆದರೆ (ಟೋಕನುಗಳು[PrevIdx].getType() != Token::Type::ಪಠ್ಯ_2)
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಸ್ಥಿರಾಂಕ Token &PrevToken = ಟೋಕನುಗಳು[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  ಹಿಂತಿರುಗಿ !TokenBody.ತುದಿಗಳು_with("\n") && !(TokenBody.ಖಾಲಿಗಳು() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
ಸ್ಥಿರ ತಾರ್ಕಿಕ has_ಪಠ್ಯ_Ahead(size_t Idx, ಸ್ಥಿರಾಂಕ ArrayRef<Token> &ಟೋಕನುಗಳು) {
  ಆದರೆ (Idx >= ಟೋಕನುಗಳು.ಗಾತ್ರ_3() - 1)
    ಹಿಂತಿರುಗಿ ನಿಜ;

  size_t ಮುಂದಿನ_Idx = Idx + 1;
  ಆದರೆ (ಟೋಕನುಗಳು[ಮುಂದಿನ_Idx].getType() != Token::Type::ಪಠ್ಯ_2)
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಸ್ಥಿರಾಂಕ Token &ಮುಂದಿನ_Token = ಟೋಕನುಗಳು[ಮುಂದಿನ_Idx];
  StringRef TokenBody = StringRef(ಮುಂದಿನ_Token.RawBody).ltrim(" ");
  ಹಿಂತಿರುಗಿ !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ requires_Clean_ಮೇಲೆ(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  ಹಿಂತಿರುಗಿ T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::ಗಣ_Delimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
ಸ್ಥಿರ ಶೂನ್ಯ stripTokenAhead(Small_ಸದಿಶ_Impl<Token> &ಟೋಕನುಗಳು, size_t Idx) {
  Token &ಮುಂದಿನ_Token = ಟೋಕನುಗಳು[Idx + 1];
  StringRef ಮುಂದಿನ_Token_Body = ಮುಂದಿನ_Token.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  ಆದರೆ (ಮುಂದಿನ_Token_Body.starts_with("\r\n"))
    ಮುಂದಿನ_Token.TokenBody = ಮುಂದಿನ_Token_Body.substr(2);
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮುಂದಿನ_Token_Body.starts_with("\n"))
    ಮುಂದಿನ_Token.TokenBody = ಮುಂದಿನ_Token_Body.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
ಶೂನ್ಯ stripTokenBefore(Small_ಸದಿಶ_Impl<Token> &ಟೋಕನುಗಳು, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = ಟೋಕನುಗಳು[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.ಗಾತ್ರ_3() - Unindented.ಗಾತ್ರ_3();
  PrevToken.TokenBody = Unindented;
  CurrentToken.ಗಣ_Indentation(Indentation);
}

ರಚನೆ ಟ್ಯಾಗ್ {
  ಎಣಿಕೆ ವರ್ಗ ಬಗೆ_2 {
    None,
    ಸಾಮಾನ್ಯ, // {{...}}
    Triple, // {{{...}}}
  };

  ಬಗೆ_2 ಟ್ಯಾಗ್_ಬಗೆ = ಬಗೆ_2::None;
  StringRef Content;   // The content between the delimiters.
  StringRef ತುಂಬಿದ_ಹೊಂದಿಕೆ; // The entire tag, including delimiters.
  size_t ಆರಂಭ_Position = StringRef::npos;
};

[[maybe_ಬಳಸದ]] ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಟ್ಯಾಗ್_ಬಗೆ_ಗಮ್ಯ_String(ಟ್ಯಾಗ್::ಬಗೆ_2 K) {
  ಆಯ್ಕೆ (K) {
  ಸ್ಥಿತಿ ಟ್ಯಾಗ್::ಬಗೆ_2::None:
    ಹಿಂತಿರುಗಿ "None";
  ಸ್ಥಿತಿ ಟ್ಯಾಗ್::ಬಗೆ_2::ಸಾಮಾನ್ಯ:
    ಹಿಂತಿರುಗಿ "Normal";
  ಸ್ಥಿತಿ ಟ್ಯಾಗ್::ಬಗೆ_2::Triple:
    ಹಿಂತಿರುಗಿ "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_ಬಳಸದ]] ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *json_ಬಗೆ_ಗಮ್ಯ_String(json::ಮೌಲ್ಯ::ಬಗೆ_2 K) {
  ಆಯ್ಕೆ (K) {
  ಸ್ಥಿತಿ json::ಮೌಲ್ಯ::ಬಗೆ_2::Null:
    ಹಿಂತಿರುಗಿ "JSON_KIND_NULL";
  ಸ್ಥಿತಿ json::ಮೌಲ್ಯ::ಬಗೆ_2::Boolean:
    ಹಿಂತಿರುಗಿ "JSON_KIND_BOOLEAN";
  ಸ್ಥಿತಿ json::ಮೌಲ್ಯ::ಬಗೆ_2::ಸಂಖ್ಯೆ:
    ಹಿಂತಿರುಗಿ "JSON_KIND_NUMBER";
  ಸ್ಥಿತಿ json::ಮೌಲ್ಯ::ಬಗೆ_2::String:
    ಹಿಂತಿರುಗಿ "JSON_KIND_STRING";
  ಸ್ಥಿತಿ json::ಮೌಲ್ಯ::ಬಗೆ_2::Array:
    ಹಿಂತಿರುಗಿ "JSON_KIND_ARRAY";
  ಸ್ಥಿತಿ json::ಮೌಲ್ಯ::ಬಗೆ_2::Object:
    ಹಿಂತಿರುಗಿ "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
ಸ್ಥಿರ Small_ಸದಿಶ<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  Small_ಸದಿಶ<Token> ಟೋಕನುಗಳು;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t ಪಠ್ಯ_ಆರಂಭ = 0;

  ಸ್ಥಿರಾಂಕ StringLiteral TripleOpen("{{{");
  ಸ್ಥಿರಾಂಕ StringLiteral TripleClose("}}}");

  ಆಗ (Cursor < Template.ಗಾತ್ರ_3()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef ಟ್ಯಾಗ್_Open, ಟ್ಯಾಗ್_Close;
    ಟ್ಯಾಗ್::ಬಗೆ_2 ಬಗೆ_2;

    // Determine which tag we've encountered.
    ಆದರೆ (TemplateSuffix.starts_with(TripleOpen)) {
      ಬಗೆ_2 = ಟ್ಯಾಗ್::ಬಗೆ_2::Triple;
      ಟ್ಯಾಗ್_Open = TripleOpen;
      ಟ್ಯಾಗ್_Close = TripleClose;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (TemplateSuffix.starts_with(Open)) {
      ಬಗೆ_2 = ಟ್ಯಾಗ್::ಬಗೆ_2::ಸಾಮಾನ್ಯ;
      ಟ್ಯಾಗ್_Open = Open;
      ಟ್ಯಾಗ್_Close = Close;
    } ಇಲ್ಲದಿದ್ದರೆ {
      // Not at a tag, continue scanning.
      ++Cursor;
      ಮುಂದುವರಿ;
    }

    // Found a tag, first add the preceding text.
    ಆದರೆ (Cursor > ಪಠ್ಯ_ಆರಂಭ)
      ಟೋಕನುಗಳು.emplace_ಹಿಂಭಾಗ(Template.ತುಂಡು(ಪಠ್ಯ_ಆರಂಭ, Cursor));

    // Find the closing tag.
    size_t ಕೊನೆ_Pos = Template.ಕಂಡುಹಿಡಿ(ಟ್ಯಾಗ್_Close, Cursor + ಟ್ಯಾಗ್_Open.ಗಾತ್ರ_3());
    ಆದರೆ (ಕೊನೆ_Pos == StringRef::npos) {
      // No closing tag, the rest is text.
      ಟೋಕನುಗಳು.emplace_ಹಿಂಭಾಗ(Template.substr(Cursor));
      ಪಠ್ಯ_ಆರಂಭ = Cursor = Template.ಗಾತ್ರ_3();
      ನಿಲ್ಲಿಸು;
    }

    // Extract tag content and full match.
    size_t Content_ಆರಂಭ = Cursor + ಟ್ಯಾಗ್_Open.ಗಾತ್ರ_3();
    StringRef Content = Template.substr(Content_ಆರಂಭ, ಕೊನೆ_Pos - Content_ಆರಂಭ);
    StringRef ತುಂಬಿದ_ಹೊಂದಿಕೆ =
        Template.substr(Cursor, (ಕೊನೆ_Pos + ಟ್ಯಾಗ್_Close.ಗಾತ್ರ_3()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << ತುಂಬಿದ_ಹೊಂದಿಕೆ << ", Content: " << Content
                      << ", Kind: " << ಟ್ಯಾಗ್_ಬಗೆ_ಗಮ್ಯ_String(ಬಗೆ_2) << "\n");
    ಆದರೆ (ಬಗೆ_2 == ಟ್ಯಾಗ್::ಬಗೆ_2::Triple) {
      ಟೋಕನುಗಳು.emplace_ಹಿಂಭಾಗ(ತುಂಬಿದ_ಹೊಂದಿಕೆ, Ctx.Saver.save("&" + Content), '&', Ctx);
    } ಇಲ್ಲದಿದ್ದರೆ { // Normal Tag
      StringRef Interpolated = Content;
      ಆದರೆ (!Interpolated.trim().starts_with("=")) {
        ಅಕ್ಷರ Front = Interpolated.ಖಾಲಿಗಳು() ? ' ' : Interpolated.trim().front();
        ಟೋಕನುಗಳು.emplace_ಹಿಂಭಾಗ(ತುಂಬಿದ_ಹೊಂದಿಕೆ, Interpolated, Front, Ctx);
      } ಇಲ್ಲದಿದ್ದರೆ { // Set Delimiter
        ಟೋಕನುಗಳು.emplace_ಹಿಂಭಾಗ(ತುಂಬಿದ_ಹೊಂದಿಕೆ, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](ಅಕ್ಷರ C) { ಹಿಂತಿರುಗಿ C == '='; });
        DelimSpec = DelimSpec.trim();

        ಸ್ವಯಂಚಾಲಿತ [NewOpen, NewClose] = DelimSpec.ವಿಭಜಿಸು(' ');
        LLVM_DEBUG(dbgs() << "[Set Delimiter] NewOpen: " << NewOpen
                          << ", NewClose: " << NewClose << "\n");
        Open = NewOpen;
        Close = NewClose;
      }
    }

    // Move past the tag for the next iteration.
    Cursor += ತುಂಬಿದ_ಹೊಂದಿಕೆ.ಗಾತ್ರ_3();
    ಪಠ್ಯ_ಆರಂಭ = Cursor;
  }

  // Add any remaining text after the last tag.
  ಆದರೆ (ಪಠ್ಯ_ಆರಂಭ < Template.ಗಾತ್ರ_3())
    ಟೋಕನುಗಳು.emplace_ಹಿಂಭಾಗ(Template.substr(ಪಠ್ಯ_ಆರಂಭ));

  // Fix up white spaces for standalone tags.
  size_t ಕೊನೆಯ_Idx = ಟೋಕನುಗಳು.ಗಾತ್ರ_3() - 1;
  ಗಾಗಿ (size_t Idx = 0, ಕೊನೆ_2 = ಟೋಕನುಗಳು.ಗಾತ್ರ_3(); Idx < ಕೊನೆ_2; ++Idx) {
    Token &CurrentToken = ಟೋಕನುಗಳು[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    ಆದರೆ (!requires_Clean_ಮೇಲೆ(CurrentType))
      ಮುಂದುವರಿ;

    ತಾರ್ಕಿಕ Has_ಪಠ್ಯ_Behind = has_ಪಠ್ಯ_Behind(Idx, ಟೋಕನುಗಳು);
    ತಾರ್ಕಿಕ Has_ಪಠ್ಯ_Ahead = has_ಪಠ್ಯ_Ahead(Idx, ಟೋಕನುಗಳು);

    ಆದರೆ ((!Has_ಪಠ್ಯ_Ahead && !Has_ಪಠ್ಯ_Behind) || (!Has_ಪಠ್ಯ_Ahead && Idx == 0))
      stripTokenAhead(ಟೋಕನುಗಳು, Idx);

    ಆದರೆ ((!Has_ಪಠ್ಯ_Behind && !Has_ಪಠ್ಯ_Ahead) || (!Has_ಪಠ್ಯ_Behind && Idx == ಕೊನೆಯ_Idx))
      stripTokenBefore(ಟೋಕನುಗಳು, Idx, CurrentToken, CurrentType);
  }
  ಹಿಂತಿರುಗಿ ಟೋಕನುಗಳು;
}

// Custom stream to escape strings.
ವರ್ಗ Escape_String_ಹರಿವು : ಸಾರ್ವಜನಿಕ Mustache_Output_ಹರಿವು {
ಸಾರ್ವಜನಿಕ:
  ಸ್ಪಷ್ಟ Escape_String_ಹರಿವು(llvm::raw_ostream &Wrapped_ಹರಿವು,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.ಕೀಲಿಗಳು().begin(), Escape.ಕೀಲಿಗಳು().ಕೊನೆ()),
        Wrapped_ಹರಿವು(Wrapped_ಹರಿವು) {
    ಗಣ_Unbuffered();
  }

ಸಂರಕ್ಷಿತ:
  ಶೂನ್ಯ write_impl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) override {
    StringRef ದತ್ತಾಂಶ_2(Ptr, ಗಾತ್ರ_2);
    size_t ಆರಂಭ = 0;
    ಆಗ (ಆರಂಭ < ಗಾತ್ರ_2) {
      // Find the next character that needs to be escaped.
      size_t ಮುಂದಿನ_2 = ದತ್ತಾಂಶ_2.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(EscapeChars.str(), ಆರಂಭ);

      // If no escapable characters are found, write the rest of the string.
      ಆದರೆ (ಮುಂದಿನ_2 == StringRef::npos) {
        Wrapped_ಹರಿವು << ದತ್ತಾಂಶ_2.substr(ಆರಂಭ);
        ಹಿಂತಿರುಗಿ;
      }

      // Write the chunk of text before the escapable character.
      ಆದರೆ (ಮುಂದಿನ_2 > ಆರಂಭ)
        Wrapped_ಹರಿವು << ದತ್ತಾಂಶ_2.substr(ಆರಂಭ, ಮುಂದಿನ_2 - ಆರಂಭ);

      // Look up and write the escaped version of the character.
      Wrapped_ಹರಿವು << Escape[ದತ್ತಾಂಶ_2[ಮುಂದಿನ_2]];
      ಆರಂಭ = ಮುಂದಿನ_2 + 1;
    }
  }

  uint64_t current_pos() ಸ್ಥಿರಾಂಕ override { ಹಿಂತಿರುಗಿ Wrapped_ಹರಿವು.tell(); }

ಖಾಸಗಿ:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &Wrapped_ಹರಿವು;
};

// Custom stream to add indentation used to for rendering partials.
ವರ್ಗ ಕೂಡಿಸು_Indentation_String_ಹರಿವು : ಸಾರ್ವಜನಿಕ Mustache_Output_ಹರಿವು {
ಸಾರ್ವಜನಿಕ:
  ಸ್ಪಷ್ಟ ಕೂಡಿಸು_Indentation_String_ಹರಿವು(raw_ostream &Wrapped_ಹರಿವು,
                                      size_t Indentation)
      : Indentation(Indentation), Wrapped_ಹರಿವು(Wrapped_ಹರಿವು),
        NeedsIndent(ನಿಜ), IsSuspended(ಸುಳ್ಳು) {
    ಗಣ_Unbuffered();
  }

  ಶೂನ್ಯ suspendIndentation() override { IsSuspended = ನಿಜ; }
  ಶೂನ್ಯ resumeIndentation() override { IsSuspended = ಸುಳ್ಳು; }

ಸಂರಕ್ಷಿತ:
  ಶೂನ್ಯ write_impl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) override {
    llvm::StringRef ದತ್ತಾಂಶ_2(Ptr, ಗಾತ್ರ_2);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    ಗಾಗಿ (ಅಕ್ಷರ C : ದತ್ತಾಂಶ_2) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      ಆದರೆ (NeedsIndent && C != '\n') {
        Wrapped_ಹರಿವು << Indent;
        NeedsIndent = ಸುಳ್ಳು;
      }
      Wrapped_ಹರಿವು << C;
      ಆದರೆ (C == '\n' && !IsSuspended)
        NeedsIndent = ನಿಜ;
    }
  }

  uint64_t current_pos() ಸ್ಥಿರಾಂಕ override { ಹಿಂತಿರುಗಿ Wrapped_ಹರಿವು.tell(); }

ಖಾಸಗಿ:
  size_t Indentation;
  raw_ostream &Wrapped_ಹರಿವು;
  ತಾರ್ಕಿಕ NeedsIndent;
  ತಾರ್ಕಿಕ IsSuspended;
};

ವರ್ಗ Parser {
ಸಾರ್ವಜನಿಕ:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

ಖಾಸಗಿ:
  ಶೂನ್ಯ parseMustache(ASTNode *ಪೋಷಕ_2);
  ಶೂನ್ಯ parseSection(ASTNode *ಪೋಷಕ_2, ASTNode::Type Ty, ಸ್ಥಿರಾಂಕ Accessor &A);

  MustacheContext &Ctx;
  Small_ಸದಿಶ<Token> ಟೋಕನುಗಳು;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

ಶೂನ್ಯ Parser::parseSection(ASTNode *ಪೋಷಕ_2, ASTNode::Type Ty,
                          ಸ್ಥಿರಾಂಕ Accessor &A) {
  AstPtr Current_ಗಂಟು = create_ಗಂಟು(Ctx, Ty, A, ಪೋಷಕ_2);
  size_t ಆರಂಭ = CurrentPtr;
  parseMustache(Current_ಗಂಟು);
  ಸ್ಥಿರಾಂಕ size_t ಕೊನೆ_2 = CurrentPtr - 1;

  size_t Raw_Body_ಗಾತ್ರ = 0;
  ಗಾಗಿ (size_t I = ಆರಂಭ; I < ಕೊನೆ_2; ++I)
    Raw_Body_ಗಾತ್ರ += ಟೋಕನುಗಳು[I].RawBody.ಗಾತ್ರ_3();

  SmallString<128> RawBody;
  RawBody.reserve(Raw_Body_ಗಾತ್ರ);
  ಗಾಗಿ (std::size_t I = ಆರಂಭ; I < ಕೊನೆ_2; ++I)
    RawBody += ಟೋಕನುಗಳು[I].RawBody;

  Current_ಗಂಟು->ಗಣ_Raw_Body(Ctx.Saver.save(StringRef(RawBody)));
  ಪೋಷಕ_2->ಕೂಡಿಸು_ಮಗು(Current_ಗಂಟು);
}

AstPtr Parser::parse() {
  ಟೋಕನುಗಳು = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr ಬೇರು_ಗಂಟು = create_ಬೇರು_ಗಂಟು(Ctx);
  parseMustache(ಬೇರು_ಗಂಟು);
  ಹಿಂತಿರುಗಿ ಬೇರು_ಗಂಟು;
}

ಶೂನ್ಯ Parser::parseMustache(ASTNode *ಪೋಷಕ_2) {

  ಆಗ (CurrentPtr < ಟೋಕನುಗಳು.ಗಾತ್ರ_3()) {
    Token CurrentToken = ಟೋಕನುಗಳು[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_ಗಂಟು;

    ಆಯ್ಕೆ (CurrentToken.getType()) {
    ಸ್ಥಿತಿ Token::Type::ಪಠ್ಯ_2: {
      Current_ಗಂಟು = create_ಪಠ್ಯ_ಗಂಟು(Ctx, CurrentToken.TokenBody, ಪೋಷಕ_2);
      ಪೋಷಕ_2->ಕೂಡಿಸು_ಮಗು(Current_ಗಂಟು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ Token::Type::Variable: {
      Current_ಗಂಟು = create_ಗಂಟು(Ctx, ASTNode::Variable, A, ಪೋಷಕ_2);
      ಪೋಷಕ_2->ಕೂಡಿಸು_ಮಗು(Current_ಗಂಟು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ Token::Type::UnescapeVariable: {
      Current_ಗಂಟು = create_ಗಂಟು(Ctx, ASTNode::UnescapeVariable, A, ಪೋಷಕ_2);
      ಪೋಷಕ_2->ಕೂಡಿಸು_ಮಗು(Current_ಗಂಟು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ Token::Type::Partial: {
      Current_ಗಂಟು = create_ಗಂಟು(Ctx, ASTNode::Partial, A, ಪೋಷಕ_2);
      Current_ಗಂಟು->ಗಣ_Indentation(CurrentToken.getIndentation());
      ಪೋಷಕ_2->ಕೂಡಿಸು_ಮಗು(Current_ಗಂಟು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ Token::Type::SectionOpen: {
      parseSection(ಪೋಷಕ_2, ASTNode::Section, A);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ Token::Type::InvertSectionOpen: {
      parseSection(ಪೋಷಕ_2, ASTNode::InvertSection, A);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ Token::Type::Comment:
    ಸ್ಥಿತಿ Token::Type::ಗಣ_Delimiter:
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::Type::SectionClose:
      ಹಿಂತಿರುಗಿ;
    }
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ ಗಮ್ಯ_Mustache_String(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &ದತ್ತಾಂಶ_2, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << json_ಬಗೆ_ಗಮ್ಯ_String(ದತ್ತಾಂಶ_2.ಬಗೆ()) << ", Data: " << ದತ್ತಾಂಶ_2
                    << "\n");
  ಆಯ್ಕೆ (ದತ್ತಾಂಶ_2.ಬಗೆ()) {
  ಸ್ಥಿತಿ json::ಮೌಲ್ಯ::Null:
    ಹಿಂತಿರುಗಿ;
  ಸ್ಥಿತಿ json::ಮೌಲ್ಯ::ಸಂಖ್ಯೆ: {
    ಸ್ವಯಂಚಾಲಿತ ಅಂಕೆ = *ದತ್ತಾಂಶ_2.get_As_ಸಂಖ್ಯೆ();
    std::ostringstream SS;
    SS << ಅಂಕೆ;
    OS << SS.str();
    ಹಿಂತಿರುಗಿ;
  }
  ಸ್ಥಿತಿ json::ಮೌಲ್ಯ::String: {
    OS << *ದತ್ತಾಂಶ_2.getAsString();
    ಹಿಂತಿರುಗಿ;
  }

  ಸ್ಥಿತಿ json::ಮೌಲ್ಯ::Array: {
    ಸ್ವಯಂಚಾಲಿತ Arr = *ದತ್ತಾಂಶ_2.getAsArray();
    ಆದರೆ (Arr.ಖಾಲಿಗಳು())
      ಹಿಂತಿರುಗಿ;
    [[fallthrough]];
  }
  ಸ್ಥಿತಿ json::ಮೌಲ್ಯ::Object:
  ಸ್ಥಿತಿ json::ಮೌಲ್ಯ::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.ಮೌಲ್ಯ_2(ದತ್ತಾಂಶ_2);
    ನಿಲ್ಲಿಸು;
  }
  }
}

ಶೂನ್ಯ ASTNode::render_ಬೇರು(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &CurrentCtx,
                         Mustache_Output_ಹರಿವು &OS) {
  render_ಮಗು(CurrentCtx, OS);
}

ಶೂನ್ಯ ASTNode::render_ಪಠ್ಯ(Mustache_Output_ಹರಿವು &OS) { OS << Body; }

ಶೂನ್ಯ ASTNode::renderPartial(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &CurrentCtx,
                            Mustache_Output_ಹರಿವು &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_ಮೌಲ್ಯ[0]
                    << ", Indentation:" << Indentation << "\n");
  ಸ್ವಯಂಚಾಲಿತ Partial = Ctx.Partials.ಕಂಡುಹಿಡಿ(Accessor_ಮೌಲ್ಯ[0]);
  ಆದರೆ (Partial != Ctx.Partials.ಕೊನೆ())
    renderPartial(CurrentCtx, OS, Partial->get_ಮೌಲ್ಯ());
}

ಶೂನ್ಯ ASTNode::renderVariable(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &CurrentCtx,
                             Mustache_Output_ಹರಿವು &OS) {
  ಸ್ವಯಂಚಾಲಿತ Lambda = Ctx.Lambdas.ಕಂಡುಹಿಡಿ(Accessor_ಮೌಲ್ಯ[0]);
  ಆದರೆ (Lambda != Ctx.Lambdas.ಕೊನೆ()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_ಮೌಲ್ಯ());
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ *ContextPtr = ಕಂಡುಹಿಡಿ_Context()) {
    Escape_String_ಹರಿವು ES(OS, Ctx.Escapes);
    ಗಮ್ಯ_Mustache_String(*ContextPtr, ES);
  }
}

ಶೂನ್ಯ ASTNode::renderUnescapeVariable(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &CurrentCtx,
                                     Mustache_Output_ಹರಿವು &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_ಮೌಲ್ಯ[0]
                    << "\n");
  ಸ್ವಯಂಚಾಲಿತ Lambda = Ctx.Lambdas.ಕಂಡುಹಿಡಿ(Accessor_ಮೌಲ್ಯ[0]);
  ಆದರೆ (Lambda != Ctx.Lambdas.ಕೊನೆ()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_ಮೌಲ್ಯ());
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ *ContextPtr = ಕಂಡುಹಿಡಿ_Context()) {
    OS.suspendIndentation();
    ಗಮ್ಯ_Mustache_String(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

ಶೂನ್ಯ ASTNode::renderSection(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &CurrentCtx,
                            Mustache_Output_ಹರಿವು &OS) {
  ಸ್ವಯಂಚಾಲಿತ SectionLambda = Ctx.SectionLambdas.ಕಂಡುಹಿಡಿ(Accessor_ಮೌಲ್ಯ[0]);
  ಆದರೆ (SectionLambda != Ctx.SectionLambdas.ಕೊನೆ()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_ಮೌಲ್ಯ());
    ಹಿಂತಿರುಗಿ;
  }

  ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ *ContextPtr = ಕಂಡುಹಿಡಿ_Context();
  ಆದರೆ (isContextFalsey(ContextPtr))
    ಹಿಂತಿರುಗಿ;

  ಆದರೆ (ಸ್ಥಿರಾಂಕ json::Array *Arr = ContextPtr->getAsArray()) {
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &V : *Arr)
      render_ಮಗು(V, OS);
    ಹಿಂತಿರುಗಿ;
  }
  render_ಮಗು(*ContextPtr, OS);
}

ಶೂನ್ಯ ASTNode::renderInvertSection(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &CurrentCtx,
                                  Mustache_Output_ಹರಿವು &OS) {
  ತಾರ್ಕಿಕ IsLambda = Ctx.SectionLambdas.ಒಳಗೊಂಡಿದೆ(Accessor_ಮೌಲ್ಯ[0]);
  ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ *ContextPtr = ಕಂಡುಹಿಡಿ_Context();
  ಆದರೆ (isContextFalsey(ContextPtr) && !IsLambda) {
    render_ಮಗು(CurrentCtx, OS);
  }
}

ಶೂನ್ಯ ASTNode::render(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ದತ್ತಾಂಶ_2, Mustache_Output_ಹರಿವು &OS) {
  ಆದರೆ (Ty != ಬೇರು_2 && Ty != ಪಠ್ಯ_2 && Accessor_ಮೌಲ್ಯ.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ಪೋಷಕ_Context = &ದತ್ತಾಂಶ_2;

  ಆಯ್ಕೆ (Ty) {
  ಸ್ಥಿತಿ ಬೇರು_2:
    render_ಬೇರು(ದತ್ತಾಂಶ_2, OS);
    ಹಿಂತಿರುಗಿ;
  ಸ್ಥಿತಿ ಪಠ್ಯ_2:
    render_ಪಠ್ಯ(OS);
    ಹಿಂತಿರುಗಿ;
  ಸ್ಥಿತಿ Partial:
    renderPartial(ದತ್ತಾಂಶ_2, OS);
    ಹಿಂತಿರುಗಿ;
  ಸ್ಥಿತಿ Variable:
    renderVariable(ದತ್ತಾಂಶ_2, OS);
    ಹಿಂತಿರುಗಿ;
  ಸ್ಥಿತಿ UnescapeVariable:
    renderUnescapeVariable(ದತ್ತಾಂಶ_2, OS);
    ಹಿಂತಿರುಗಿ;
  ಸ್ಥಿತಿ Section:
    renderSection(ದತ್ತಾಂಶ_2, OS);
    ಹಿಂತಿರುಗಿ;
  ಸ್ಥಿತಿ InvertSection:
    renderInvertSection(ದತ್ತಾಂಶ_2, OS);
    ಹಿಂತಿರುಗಿ;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ *ASTNode::ಕಂಡುಹಿಡಿ_Context() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  ಆದರೆ (Accessor_ಮೌಲ್ಯ.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  ಆದರೆ (Accessor_ಮೌಲ್ಯ[0] == ".")
    ಹಿಂತಿರುಗಿ ಪೋಷಕ_Context;

  ಸ್ಥಿರಾಂಕ json::Object *CurrentContext = ಪೋಷಕ_Context->getAsObject();
  StringRef CurrentAccessor = Accessor_ಮೌಲ್ಯ[0];
  ASTNode *Current_ಪೋಷಕ = ಪೋಷಕ_2;

  ಆಗ (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    ಆದರೆ (Current_ಪೋಷಕ->Ty != ಬೇರು_2) {
      CurrentContext = Current_ಪೋಷಕ->ಪೋಷಕ_Context->getAsObject();
      Current_ಪೋಷಕ = Current_ಪೋಷಕ->ಪೋಷಕ_2;
      ಮುಂದುವರಿ;
    }
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  }
  ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ *Context = ಶೂನ್ಯಸೂಚಕ;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ [Idx, Acc] : enumerate(Accessor_ಮೌಲ್ಯ)) {
    ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ *Current_ಮೌಲ್ಯ = CurrentContext->get(Acc);
    ಆದರೆ (!Current_ಮೌಲ್ಯ)
      ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
    ಆದರೆ (Idx < Accessor_ಮೌಲ್ಯ.ಗಾತ್ರ_3() - 1) {
      CurrentContext = Current_ಮೌಲ್ಯ->getAsObject();
      ಆದರೆ (!CurrentContext)
        ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
    } ಇಲ್ಲದಿದ್ದರೆ {
      Context = Current_ಮೌಲ್ಯ;
    }
  }
  ಹಿಂತಿರುಗಿ Context;
}

ಶೂನ್ಯ ASTNode::render_ಮಗು(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &Contexts,
                          Mustache_Output_ಹರಿವು &OS) {
  ಗಾಗಿ (ASTNode &ಮಗು : Children)
    ಮಗು.render(Contexts, OS);
}

ಶೂನ್ಯ ASTNode::renderPartial(ಸ್ಥಿರಾಂಕ json::ಮೌಲ್ಯ &Contexts,
                            Mustache_Output_ಹರಿವು &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  ಕೂಡಿಸು_Indentation_String_ಹರಿವು IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

ಶೂನ್ಯ ASTNode::renderLambdas(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &Contexts,
                            Mustache_Output_ಹರಿವು &OS, Lambda &L) {
  json::ಮೌಲ್ಯ Lambda_ಫಲಿತಾಂಶ = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  ಗಮ್ಯ_Mustache_String(Lambda_ಫಲಿತಾಂಶ, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ಗಂಟು = P.parse();

  Escape_String_ಹರಿವು ES(OS, Ctx.Escapes);
  ಆದರೆ (Ty == Variable) {
    Lambda_ಗಂಟು->render(Contexts, ES);
    ಹಿಂತಿರುಗಿ;
  }
  Lambda_ಗಂಟು->render(Contexts, OS);
}

ಶೂನ್ಯ ASTNode::renderSectionLambdas(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &Contexts,
                                   Mustache_Output_ಹರಿವು &OS, SectionLambda &L) {
  json::ಮೌಲ್ಯ Return = L(RawBody.str());
  ಆದರೆ (isFalsey(Return))
    ಹಿಂತಿರುಗಿ;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  ಗಮ್ಯ_Mustache_String(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ಗಂಟು = P.parse();
  Lambda_ಗಂಟು->render(Contexts, OS);
}

ಶೂನ್ಯ Template::render(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ದತ್ತಾಂಶ_2, llvm::raw_ostream &OS) {
  Raw_Mustache_Output_ಹರಿವು MOS(OS);
  ವೃಕ್ಷ->render(ದತ್ತಾಂಶ_2, MOS);
}

ಶೂನ್ಯ Template::ರಿಜಿಸ್ಟರ್_Partial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_ವೃಕ್ಷ = P.parse();
  Ctx.Partials.ಸೇರಿಸು(std::make_pair(Name, Partial_ವೃಕ್ಷ));
}

ಶೂನ್ಯ Template::ರಿಜಿಸ್ಟರ್_Lambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

ಶೂನ್ಯ Template::ರಿಜಿಸ್ಟರ್_Lambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

ಶೂನ್ಯ Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  ವೃಕ್ಷ = P.parse();
  // The default behavior is to escape html entities.
  ಸ್ಥಿರಾಂಕ EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), ವೃಕ್ಷ(Other.ವೃಕ್ಷ) {
  Other.ವೃಕ್ಷ = ಶೂನ್ಯಸೂಚಕ;
}

Template::~Template() = ಪೂರ್ವನಿಯೋಜಿತ;

} // namespace llvm::mustache

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು DEBUG_TYPE
