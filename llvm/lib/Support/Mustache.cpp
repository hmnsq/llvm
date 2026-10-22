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

#define DEBUG_TYPE "mustache"

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::mustache;

ನಾಮಾಕಾಶ {

using Accessor = ArrayRef<StringRef>;

ಸ್ಥಿರ ತಾರ್ಕಿಕ isFalsey(ಧ್ರುವ json::ಮೌಲ್ಯ &V) {
  ಪ್ರತ್ಯಾಗಮ V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().ಮೌಲ್ಯ_2()) ||
         (V.getAsArray() && V.getAsArray()->empty());
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isContextFalsey(ಧ್ರುವ json::ಮೌಲ್ಯ *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  ಯದಿ (!V)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ isFalsey(*V);
}

ಸ್ಥಿರ ಶೂನ್ಯ splitAndTrim(StringRef Str, SmallVectorImpl<StringRef> &Tokens) {
  size_t CurrentPos = 0;
  ಯಾವತ್ (CurrentPos < Str.size()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    ಯದಿ (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.size();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.slice(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t Start = Part.find_first_not_of(" \t\r\n");
    ಯದಿ (Start != StringRef::npos) {
      size_t End = Part.find_last_not_of(" \t\r\n");
      Tokens.ತಳ್ಳು_back(Part.slice(Start, End + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

ಸ್ಥಿರ Accessor splitMustacheString(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  SmallVector<StringRef> Tokens;
  ಯದಿ (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    Tokens.ತಳ್ಳು_back(".");
  } ಅನ್ಯಥಾ {
    splitAndTrim(Str, Tokens);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.size());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  ಪ್ರತ್ಯಾಗಮ ArrayRef<StringRef>(ArenaTokens, Tokens.size());
}
} // namespace

ನಾಮಾಕಾಶ llvm::mustache {

ವರ್ಗ MustacheOutputStream : ಸಾರ್ವಜನಿಕ raw_ostream {
ಸಾರ್ವಜನಿಕ:
  MustacheOutputStream() = ಸ್ವಭಾವ;
  ~MustacheOutputStream() override = ಸ್ವಭಾವ;

  ಆಭಾಸ ಶೂನ್ಯ suspendIndentation() {}
  ಆಭಾಸ ಶೂನ್ಯ resumeIndentation() {}

ಗುಪ್ತ:
  ಶೂನ್ಯ anchor() override;
};

ಶೂನ್ಯ MustacheOutputStream::anchor() {}

ವರ್ಗ RawMustacheOutputStream : ಸಾರ್ವಜನಿಕ MustacheOutputStream {
ಸಾರ್ವಜನಿಕ:
  RawMustacheOutputStream(raw_ostream &OS) : OS(OS) { SetUnbuffered(); }

ಗುಪ್ತ:
  raw_ostream &OS;

  ಶೂನ್ಯ write_impl(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size) override {
    OS.ಲಿಖ(Ptr, Size);
  }
  uint64_t current_pos() ಧ್ರುವ override { ಪ್ರತ್ಯಾಗಮ OS.tell(); }
};

ವರ್ಗ Token {
ಸಾರ್ವಜನಿಕ:
  ಗಣನಾ ವರ್ಗ Type {
    Text,
    Variable,
    Partial,
    SectionOpen,
    SectionClose,
    InvertSectionOpen,
    UnescapeVariable,
    Comment,
    SetDelimiter,
  };

  Token(StringRef Str)
      : TokenType(Type::Text), RawBody(Str), TokenBody(RawBody),
        Accessor_ಮೌಲ್ಯ({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, ವರ್ಣ Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    ಯದಿ (TokenType == Type::Comment)
      ಪ್ರತ್ಯಾಗಮ;
    StringRef AccessorStr(ಏತತ್->TokenBody);
    ಯದಿ (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_ಮೌಲ್ಯ = splitMustacheString(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Accessor_ಮೌಲ್ಯ; }

  Type getType() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ TokenType; }

  ಶೂನ್ಯ setIndentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Indentation; }

  ಸ್ಥಿರ Type getTokenType(ವರ್ಣ Identifier) {
    ವಿಕಲ್ಪ (Identifier) {
    ಪಕ್ಷ '#':
      ಪ್ರತ್ಯಾಗಮ Type::SectionOpen;
    ಪಕ್ಷ '/':
      ಪ್ರತ್ಯಾಗಮ Type::SectionClose;
    ಪಕ್ಷ '^':
      ಪ್ರತ್ಯಾಗಮ Type::InvertSectionOpen;
    ಪಕ್ಷ '!':
      ಪ್ರತ್ಯಾಗಮ Type::Comment;
    ಪಕ್ಷ '>':
      ಪ್ರತ್ಯಾಗಮ Type::Partial;
    ಪಕ್ಷ '&':
      ಪ್ರತ್ಯಾಗಮ Type::UnescapeVariable;
    ಪಕ್ಷ '=':
      ಪ್ರತ್ಯಾಗಮ Type::SetDelimiter;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ Type::Variable;
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

using EscapeMap = DenseMap<ವರ್ಣ, std::string>;

ವರ್ಗ ASTNode : ಸಾರ್ವಜನಿಕ ilist_ಗಂಟು<ASTNode> {
ಸಾರ್ವಜನಿಕ:
  ಗಣನಾ Type {
    ಬೇರು_2,
    Text,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::ಬೇರು_2), Parent(nullptr), ParentContext(nullptr) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *Parent)
      : Ctx(Ctx), Ty(Type::Text), Body(Body), Parent(Parent),
        ParentContext(nullptr) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *Parent)
      : Ctx(Ctx), Ty(Ty), Parent(Parent), Accessor_ಮೌಲ್ಯ(Accessor),
        ParentContext(nullptr) {}

  ಶೂನ್ಯ addChild(AstPtr Child) { Children.ತಳ್ಳು_back(Child); };

  ಶೂನ್ಯ setRawBody(StringRef NewBody) { RawBody = NewBody; };

  ಶೂನ್ಯ setIndentation(size_t NewIndentation) { Indentation = NewIndentation; };

  ಶೂನ್ಯ render(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &Data, MustacheOutputStream &OS);

ಗುಪ್ತ:
  ಶೂನ್ಯ renderLambdas(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &Contexts,
                     MustacheOutputStream &OS, Lambda &L);

  ಶೂನ್ಯ renderSectionLambdas(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &Contexts,
                            MustacheOutputStream &OS, SectionLambda &L);

  ಶೂನ್ಯ renderPartial(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &Contexts,
                     MustacheOutputStream &OS, ASTNode *Partial);

  ಶೂನ್ಯ renderChild(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &Context, MustacheOutputStream &OS);

  ಧ್ರುವ llvm::json::ಮೌಲ್ಯ *findContext();

  ಶೂನ್ಯ render_ಬೇರು(ಧ್ರುವ json::ಮೌಲ್ಯ &CurrentCtx, MustacheOutputStream &OS);
  ಶೂನ್ಯ renderText(MustacheOutputStream &OS);
  ಶೂನ್ಯ renderPartial(ಧ್ರುವ json::ಮೌಲ್ಯ &CurrentCtx, MustacheOutputStream &OS);
  ಶೂನ್ಯ renderVariable(ಧ್ರುವ json::ಮೌಲ್ಯ &CurrentCtx, MustacheOutputStream &OS);
  ಶೂನ್ಯ renderUnescapeVariable(ಧ್ರುವ json::ಮೌಲ್ಯ &CurrentCtx,
                              MustacheOutputStream &OS);
  ಶೂನ್ಯ renderSection(ಧ್ರುವ json::ಮೌಲ್ಯ &CurrentCtx, MustacheOutputStream &OS);
  ಶೂನ್ಯ renderInvertSection(ಧ್ರುವ json::ಮೌಲ್ಯ &CurrentCtx,
                           MustacheOutputStream &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  ಧ್ರುವ ArrayRef<StringRef> Accessor_ಮೌಲ್ಯ;
  ಧ್ರುವ llvm::json::ಮೌಲ್ಯ *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
ಸ್ಥಿರ AstPtr create_ಬೇರು_ಗಂಟು(MustacheContext &Ctx) {
  ಪ್ರತ್ಯಾಗಮ ನೂತನ (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

ಸ್ಥಿರ AstPtr create_ಗಂಟು(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  ಪ್ರತ್ಯಾಗಮ ನೂತನ (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

ಸ್ಥಿರ AstPtr create_Text_ಗಂಟು(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  ಪ್ರತ್ಯಾಗಮ ನೂತನ (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
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
ಸ್ಥಿರ ತಾರ್ಕಿಕ hasTextBehind(size_t Idx, ಧ್ರುವ ArrayRef<Token> &Tokens) {
  ಯದಿ (Idx == 0)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  size_t PrevIdx = Idx - 1;
  ಯದಿ (Tokens[PrevIdx].getType() != Token::Type::Text)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಧ್ರುವ Token &PrevToken = Tokens[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  ಪ್ರತ್ಯಾಗಮ !TokenBody.ends_with("\n") && !(TokenBody.empty() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
ಸ್ಥಿರ ತಾರ್ಕಿಕ hasTextAhead(size_t Idx, ಧ್ರುವ ArrayRef<Token> &Tokens) {
  ಯದಿ (Idx >= Tokens.size() - 1)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  size_t NextIdx = Idx + 1;
  ಯದಿ (Tokens[NextIdx].getType() != Token::Type::Text)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಧ್ರುವ Token &NextToken = Tokens[NextIdx];
  StringRef TokenBody = StringRef(NextToken.RawBody).ltrim(" ");
  ಪ್ರತ್ಯಾಗಮ !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ requiresCleanUp(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  ಪ್ರತ್ಯಾಗಮ T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::SetDelimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
ಸ್ಥಿರ ಶೂನ್ಯ stripTokenAhead(SmallVectorImpl<Token> &Tokens, size_t Idx) {
  Token &NextToken = Tokens[Idx + 1];
  StringRef NextTokenBody = NextToken.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  ಯದಿ (NextTokenBody.starts_with("\r\n"))
    NextToken.TokenBody = NextTokenBody.substr(2);
  ಅನ್ಯಥಾ ಯದಿ (NextTokenBody.starts_with("\n"))
    NextToken.TokenBody = NextTokenBody.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
ಶೂನ್ಯ stripTokenBefore(SmallVectorImpl<Token> &Tokens, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = Tokens[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.size() - Unindented.size();
  PrevToken.TokenBody = Unindented;
  CurrentToken.setIndentation(Indentation);
}

ಸಂರಚನಾ Tag {
  ಗಣನಾ ವರ್ಗ Kind {
    None,
    Normal, // {{...}}
    Triple, // {{{...}}}
  };

  Kind TagKind = Kind::None;
  StringRef Content;   // The content between the delimiters.
  StringRef FullMatch; // The entire tag, including delimiters.
  size_t StartPosition = StringRef::npos;
};

[[maybe_unused]] ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *tagKindToString(Tag::Kind K) {
  ವಿಕಲ್ಪ (K) {
  ಪಕ್ಷ Tag::Kind::None:
    ಪ್ರತ್ಯಾಗಮ "None";
  ಪಕ್ಷ Tag::Kind::Normal:
    ಪ್ರತ್ಯಾಗಮ "Normal";
  ಪಕ್ಷ Tag::Kind::Triple:
    ಪ್ರತ್ಯಾಗಮ "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_unused]] ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *jsonKindToString(json::ಮೌಲ್ಯ::Kind K) {
  ವಿಕಲ್ಪ (K) {
  ಪಕ್ಷ json::ಮೌಲ್ಯ::Kind::Null:
    ಪ್ರತ್ಯಾಗಮ "JSON_KIND_NULL";
  ಪಕ್ಷ json::ಮೌಲ್ಯ::Kind::Boolean:
    ಪ್ರತ್ಯಾಗಮ "JSON_KIND_BOOLEAN";
  ಪಕ್ಷ json::ಮೌಲ್ಯ::Kind::ಸಂಖ್ಯೆ:
    ಪ್ರತ್ಯಾಗಮ "JSON_KIND_NUMBER";
  ಪಕ್ಷ json::ಮೌಲ್ಯ::Kind::String:
    ಪ್ರತ್ಯಾಗಮ "JSON_KIND_STRING";
  ಪಕ್ಷ json::ಮೌಲ್ಯ::Kind::Array:
    ಪ್ರತ್ಯಾಗಮ "JSON_KIND_ARRAY";
  ಪಕ್ಷ json::ಮೌಲ್ಯ::Kind::Object:
    ಪ್ರತ್ಯಾಗಮ "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
ಸ್ಥಿರ SmallVector<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  SmallVector<Token> Tokens;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t TextStart = 0;

  ಧ್ರುವ StringLiteral TripleOpen("{{{");
  ಧ್ರುವ StringLiteral TripleClose("}}}");

  ಯಾವತ್ (Cursor < Template.size()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef TagOpen, TagClose;
    Tag::Kind Kind;

    // Determine which tag we've encountered.
    ಯದಿ (TemplateSuffix.starts_with(TripleOpen)) {
      Kind = Tag::Kind::Triple;
      TagOpen = TripleOpen;
      TagClose = TripleClose;
    } ಅನ್ಯಥಾ ಯದಿ (TemplateSuffix.starts_with(Open)) {
      Kind = Tag::Kind::Normal;
      TagOpen = Open;
      TagClose = Close;
    } ಅನ್ಯಥಾ {
      // Not at a tag, continue scanning.
      ++Cursor;
      ಅನುವರ್ತ;
    }

    // Found a tag, first add the preceding text.
    ಯದಿ (Cursor > TextStart)
      Tokens.emplace_back(Template.slice(TextStart, Cursor));

    // Find the closing tag.
    size_t EndPos = Template.find(TagClose, Cursor + TagOpen.size());
    ಯದಿ (EndPos == StringRef::npos) {
      // No closing tag, the rest is text.
      Tokens.emplace_back(Template.substr(Cursor));
      TextStart = Cursor = Template.size();
      ಭಙ್ಗ;
    }

    // Extract tag content and full match.
    size_t ContentStart = Cursor + TagOpen.size();
    StringRef Content = Template.substr(ContentStart, EndPos - ContentStart);
    StringRef FullMatch =
        Template.substr(Cursor, (EndPos + TagClose.size()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << FullMatch << ", Content: " << Content
                      << ", Kind: " << tagKindToString(Kind) << "\n");
    ಯದಿ (Kind == Tag::Kind::Triple) {
      Tokens.emplace_back(FullMatch, Ctx.Saver.save("&" + Content), '&', Ctx);
    } ಅನ್ಯಥಾ { // Normal Tag
      StringRef Interpolated = Content;
      ಯದಿ (!Interpolated.trim().starts_with("=")) {
        ವರ್ಣ Front = Interpolated.empty() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_back(FullMatch, Interpolated, Front, Ctx);
      } ಅನ್ಯಥಾ { // Set Delimiter
        Tokens.emplace_back(FullMatch, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](ವರ್ಣ C) { ಪ್ರತ್ಯಾಗಮ C == '='; });
        DelimSpec = DelimSpec.trim();

        auto [NewOpen, NewClose] = DelimSpec.split(' ');
        LLVM_DEBUG(dbgs() << "[Set Delimiter] NewOpen: " << NewOpen
                          << ", NewClose: " << NewClose << "\n");
        Open = NewOpen;
        Close = NewClose;
      }
    }

    // Move past the tag for the next iteration.
    Cursor += FullMatch.size();
    TextStart = Cursor;
  }

  // Add any remaining text after the last tag.
  ಯದಿ (TextStart < Template.size())
    Tokens.emplace_back(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  size_t LastIdx = Tokens.size() - 1;
  ಪ್ರತಿ (size_t Idx = 0, End = Tokens.size(); Idx < End; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    ಯದಿ (!requiresCleanUp(CurrentType))
      ಅನುವರ್ತ;

    ತಾರ್ಕಿಕ HasTextBehind = hasTextBehind(Idx, Tokens);
    ತಾರ್ಕಿಕ HasTextAhead = hasTextAhead(Idx, Tokens);

    ಯದಿ ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    ಯದಿ ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == LastIdx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  ಪ್ರತ್ಯಾಗಮ Tokens;
}

// Custom stream to escape strings.
ವರ್ಗ EscapeStringStream : ಸಾರ್ವಜನಿಕ MustacheOutputStream {
ಸಾರ್ವಜನಿಕ:
  explicit EscapeStringStream(llvm::raw_ostream &WrappedStream,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.ಕೀಲಿಗಳು().begin(), Escape.ಕೀಲಿಗಳು().end()),
        WrappedStream(WrappedStream) {
    SetUnbuffered();
  }

ರಕ್ಷಿತ:
  ಶೂನ್ಯ write_impl(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size) override {
    StringRef Data(Ptr, Size);
    size_t Start = 0;
    ಯಾವತ್ (Start < Size) {
      // Find the next character that needs to be escaped.
      size_t Next = Data.find_first_of(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      ಯದಿ (Next == StringRef::npos) {
        WrappedStream << Data.substr(Start);
        ಪ್ರತ್ಯಾಗಮ;
      }

      // Write the chunk of text before the escapable character.
      ಯದಿ (Next > Start)
        WrappedStream << Data.substr(Start, Next - Start);

      // Look up and write the escaped version of the character.
      WrappedStream << Escape[Data[Next]];
      Start = Next + 1;
    }
  }

  uint64_t current_pos() ಧ್ರುವ override { ಪ್ರತ್ಯಾಗಮ WrappedStream.tell(); }

ಗುಪ್ತ:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &WrappedStream;
};

// Custom stream to add indentation used to for rendering partials.
ವರ್ಗ AddIndentationStringStream : ಸಾರ್ವಜನಿಕ MustacheOutputStream {
ಸಾರ್ವಜನಿಕ:
  explicit AddIndentationStringStream(raw_ostream &WrappedStream,
                                      size_t Indentation)
      : Indentation(Indentation), WrappedStream(WrappedStream),
        NeedsIndent(ಸತ್ಯ), IsSuspended(ಅಸತ್ಯ) {
    SetUnbuffered();
  }

  ಶೂನ್ಯ suspendIndentation() override { IsSuspended = ಸತ್ಯ; }
  ಶೂನ್ಯ resumeIndentation() override { IsSuspended = ಅಸತ್ಯ; }

ರಕ್ಷಿತ:
  ಶೂನ್ಯ write_impl(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size) override {
    llvm::StringRef Data(Ptr, Size);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    ಪ್ರತಿ (ವರ್ಣ C : Data) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      ಯದಿ (NeedsIndent && C != '\n') {
        WrappedStream << Indent;
        NeedsIndent = ಅಸತ್ಯ;
      }
      WrappedStream << C;
      ಯದಿ (C == '\n' && !IsSuspended)
        NeedsIndent = ಸತ್ಯ;
    }
  }

  uint64_t current_pos() ಧ್ರುವ override { ಪ್ರತ್ಯಾಗಮ WrappedStream.tell(); }

ಗುಪ್ತ:
  size_t Indentation;
  raw_ostream &WrappedStream;
  ತಾರ್ಕಿಕ NeedsIndent;
  ತಾರ್ಕಿಕ IsSuspended;
};

ವರ್ಗ Parser {
ಸಾರ್ವಜನಿಕ:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

ಗುಪ್ತ:
  ಶೂನ್ಯ parseMustache(ASTNode *Parent);
  ಶೂನ್ಯ parseSection(ASTNode *Parent, ASTNode::Type Ty, ಧ್ರುವ Accessor &A);

  MustacheContext &Ctx;
  SmallVector<Token> Tokens;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

ಶೂನ್ಯ Parser::parseSection(ASTNode *Parent, ASTNode::Type Ty,
                          ಧ್ರುವ Accessor &A) {
  AstPtr Current_ಗಂಟು = create_ಗಂಟು(Ctx, Ty, A, Parent);
  size_t Start = CurrentPtr;
  parseMustache(Current_ಗಂಟು);
  ಧ್ರುವ size_t End = CurrentPtr - 1;

  size_t RawBodySize = 0;
  ಪ್ರತಿ (size_t I = Start; I < End; ++I)
    RawBodySize += Tokens[I].RawBody.size();

  SmallString<128> RawBody;
  RawBody.reserve(RawBodySize);
  ಪ್ರತಿ (std::size_t I = Start; I < End; ++I)
    RawBody += Tokens[I].RawBody;

  Current_ಗಂಟು->setRawBody(Ctx.Saver.save(StringRef(RawBody)));
  Parent->addChild(Current_ಗಂಟು);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr ಬೇರು_ಗಂಟು = create_ಬೇರು_ಗಂಟು(Ctx);
  parseMustache(ಬೇರು_ಗಂಟು);
  ಪ್ರತ್ಯಾಗಮ ಬೇರು_ಗಂಟು;
}

ಶೂನ್ಯ Parser::parseMustache(ASTNode *Parent) {

  ಯಾವತ್ (CurrentPtr < Tokens.size()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_ಗಂಟು;

    ವಿಕಲ್ಪ (CurrentToken.getType()) {
    ಪಕ್ಷ Token::Type::Text: {
      Current_ಗಂಟು = create_Text_ಗಂಟು(Ctx, CurrentToken.TokenBody, Parent);
      Parent->addChild(Current_ಗಂಟು);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ Token::Type::Variable: {
      Current_ಗಂಟು = create_ಗಂಟು(Ctx, ASTNode::Variable, A, Parent);
      Parent->addChild(Current_ಗಂಟು);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ Token::Type::UnescapeVariable: {
      Current_ಗಂಟು = create_ಗಂಟು(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->addChild(Current_ಗಂಟು);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ Token::Type::Partial: {
      Current_ಗಂಟು = create_ಗಂಟು(Ctx, ASTNode::Partial, A, Parent);
      Current_ಗಂಟು->setIndentation(CurrentToken.getIndentation());
      Parent->addChild(Current_ಗಂಟು);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ Token::Type::SectionOpen: {
      parseSection(Parent, ASTNode::Section, A);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ Token::Type::InvertSectionOpen: {
      parseSection(Parent, ASTNode::InvertSection, A);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ Token::Type::Comment:
    ಪಕ್ಷ Token::Type::SetDelimiter:
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::Type::SectionClose:
      ಪ್ರತ್ಯಾಗಮ;
    }
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ toMustacheString(ಧ್ರುವ json::ಮೌಲ್ಯ &Data, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << jsonKindToString(Data.kind()) << ", Data: " << Data
                    << "\n");
  ವಿಕಲ್ಪ (Data.kind()) {
  ಪಕ್ಷ json::ಮೌಲ್ಯ::Null:
    ಪ್ರತ್ಯಾಗಮ;
  ಪಕ್ಷ json::ಮೌಲ್ಯ::ಸಂಖ್ಯೆ: {
    auto ಅಂಕೆ = *Data.get_As_ಸಂಖ್ಯೆ();
    std::ostringstream SS;
    SS << ಅಂಕೆ;
    OS << SS.str();
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಪಕ್ಷ json::ಮೌಲ್ಯ::String: {
    OS << *Data.getAsString();
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಪಕ್ಷ json::ಮೌಲ್ಯ::Array: {
    auto Arr = *Data.getAsArray();
    ಯದಿ (Arr.empty())
      ಪ್ರತ್ಯಾಗಮ;
    [[fallthrough]];
  }
  ಪಕ್ಷ json::ಮೌಲ್ಯ::Object:
  ಪಕ್ಷ json::ಮೌಲ್ಯ::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.ಮೌಲ್ಯ_2(Data);
    ಭಙ್ಗ;
  }
  }
}

ಶೂನ್ಯ ASTNode::render_ಬೇರು(ಧ್ರುವ json::ಮೌಲ್ಯ &CurrentCtx,
                         MustacheOutputStream &OS) {
  renderChild(CurrentCtx, OS);
}

ಶೂನ್ಯ ASTNode::renderText(MustacheOutputStream &OS) { OS << Body; }

ಶೂನ್ಯ ASTNode::renderPartial(ಧ್ರುವ json::ಮೌಲ್ಯ &CurrentCtx,
                            MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_ಮೌಲ್ಯ[0]
                    << ", Indentation:" << Indentation << "\n");
  auto Partial = Ctx.Partials.find(Accessor_ಮೌಲ್ಯ[0]);
  ಯದಿ (Partial != Ctx.Partials.end())
    renderPartial(CurrentCtx, OS, Partial->get_ಮೌಲ್ಯ());
}

ಶೂನ್ಯ ASTNode::renderVariable(ಧ್ರುವ json::ಮೌಲ್ಯ &CurrentCtx,
                             MustacheOutputStream &OS) {
  auto Lambda = Ctx.Lambdas.find(Accessor_ಮೌಲ್ಯ[0]);
  ಯದಿ (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_ಮೌಲ್ಯ());
  } ಅನ್ಯಥಾ ಯದಿ (ಧ್ರುವ json::ಮೌಲ್ಯ *ContextPtr = findContext()) {
    EscapeStringStream ES(OS, Ctx.Escapes);
    toMustacheString(*ContextPtr, ES);
  }
}

ಶೂನ್ಯ ASTNode::renderUnescapeVariable(ಧ್ರುವ json::ಮೌಲ್ಯ &CurrentCtx,
                                     MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_ಮೌಲ್ಯ[0]
                    << "\n");
  auto Lambda = Ctx.Lambdas.find(Accessor_ಮೌಲ್ಯ[0]);
  ಯದಿ (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_ಮೌಲ್ಯ());
  } ಅನ್ಯಥಾ ಯದಿ (ಧ್ರುವ json::ಮೌಲ್ಯ *ContextPtr = findContext()) {
    OS.suspendIndentation();
    toMustacheString(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

ಶೂನ್ಯ ASTNode::renderSection(ಧ್ರುವ json::ಮೌಲ್ಯ &CurrentCtx,
                            MustacheOutputStream &OS) {
  auto SectionLambda = Ctx.SectionLambdas.find(Accessor_ಮೌಲ್ಯ[0]);
  ಯದಿ (SectionLambda != Ctx.SectionLambdas.end()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_ಮೌಲ್ಯ());
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಧ್ರುವ json::ಮೌಲ್ಯ *ContextPtr = findContext();
  ಯದಿ (isContextFalsey(ContextPtr))
    ಪ್ರತ್ಯಾಗಮ;

  ಯದಿ (ಧ್ರುವ json::Array *Arr = ContextPtr->getAsArray()) {
    ಪ್ರತಿ (ಧ್ರುವ json::ಮೌಲ್ಯ &V : *Arr)
      renderChild(V, OS);
    ಪ್ರತ್ಯಾಗಮ;
  }
  renderChild(*ContextPtr, OS);
}

ಶೂನ್ಯ ASTNode::renderInvertSection(ಧ್ರುವ json::ಮೌಲ್ಯ &CurrentCtx,
                                  MustacheOutputStream &OS) {
  ತಾರ್ಕಿಕ IsLambda = Ctx.SectionLambdas.contains(Accessor_ಮೌಲ್ಯ[0]);
  ಧ್ರುವ json::ಮೌಲ್ಯ *ContextPtr = findContext();
  ಯದಿ (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

ಶೂನ್ಯ ASTNode::render(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &Data, MustacheOutputStream &OS) {
  ಯದಿ (Ty != ಬೇರು_2 && Ty != Text && Accessor_ಮೌಲ್ಯ.empty())
    ಪ್ರತ್ಯಾಗಮ;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &Data;

  ವಿಕಲ್ಪ (Ty) {
  ಪಕ್ಷ ಬೇರು_2:
    render_ಬೇರು(Data, OS);
    ಪ್ರತ್ಯಾಗಮ;
  ಪಕ್ಷ Text:
    renderText(OS);
    ಪ್ರತ್ಯಾಗಮ;
  ಪಕ್ಷ Partial:
    renderPartial(Data, OS);
    ಪ್ರತ್ಯಾಗಮ;
  ಪಕ್ಷ Variable:
    renderVariable(Data, OS);
    ಪ್ರತ್ಯಾಗಮ;
  ಪಕ್ಷ UnescapeVariable:
    renderUnescapeVariable(Data, OS);
    ಪ್ರತ್ಯಾಗಮ;
  ಪಕ್ಷ Section:
    renderSection(Data, OS);
    ಪ್ರತ್ಯಾಗಮ;
  ಪಕ್ಷ InvertSection:
    renderInvertSection(Data, OS);
    ಪ್ರತ್ಯಾಗಮ;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ಧ್ರುವ json::ಮೌಲ್ಯ *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  ಯದಿ (Accessor_ಮೌಲ್ಯ.empty())
    ಪ್ರತ್ಯಾಗಮ nullptr;
  ಯದಿ (Accessor_ಮೌಲ್ಯ[0] == ".")
    ಪ್ರತ್ಯಾಗಮ ParentContext;

  ಧ್ರುವ json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = Accessor_ಮೌಲ್ಯ[0];
  ASTNode *CurrentParent = Parent;

  ಯಾವತ್ (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    ಯದಿ (CurrentParent->Ty != ಬೇರು_2) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      ಅನುವರ್ತ;
    }
    ಪ್ರತ್ಯಾಗಮ nullptr;
  }
  ಧ್ರುವ json::ಮೌಲ್ಯ *Context = nullptr;
  ಪ್ರತಿ (auto [Idx, Acc] : enumerate(Accessor_ಮೌಲ್ಯ)) {
    ಧ್ರುವ json::ಮೌಲ್ಯ *Current_ಮೌಲ್ಯ = CurrentContext->get(Acc);
    ಯದಿ (!Current_ಮೌಲ್ಯ)
      ಪ್ರತ್ಯಾಗಮ nullptr;
    ಯದಿ (Idx < Accessor_ಮೌಲ್ಯ.size() - 1) {
      CurrentContext = Current_ಮೌಲ್ಯ->getAsObject();
      ಯದಿ (!CurrentContext)
        ಪ್ರತ್ಯಾಗಮ nullptr;
    } ಅನ್ಯಥಾ {
      Context = Current_ಮೌಲ್ಯ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ Context;
}

ಶೂನ್ಯ ASTNode::renderChild(ಧ್ರುವ json::ಮೌಲ್ಯ &Contexts,
                          MustacheOutputStream &OS) {
  ಪ್ರತಿ (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

ಶೂನ್ಯ ASTNode::renderPartial(ಧ್ರುವ json::ಮೌಲ್ಯ &Contexts,
                            MustacheOutputStream &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  AddIndentationStringStream IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

ಶೂನ್ಯ ASTNode::renderLambdas(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &Contexts,
                            MustacheOutputStream &OS, Lambda &L) {
  json::ಮೌಲ್ಯ LambdaResult = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(LambdaResult, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ಗಂಟು = P.parse();

  EscapeStringStream ES(OS, Ctx.Escapes);
  ಯದಿ (Ty == Variable) {
    Lambda_ಗಂಟು->render(Contexts, ES);
    ಪ್ರತ್ಯಾಗಮ;
  }
  Lambda_ಗಂಟು->render(Contexts, OS);
}

ಶೂನ್ಯ ASTNode::renderSectionLambdas(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &Contexts,
                                   MustacheOutputStream &OS, SectionLambda &L) {
  json::ಮೌಲ್ಯ Return = L(RawBody.str());
  ಯದಿ (isFalsey(Return))
    ಪ್ರತ್ಯಾಗಮ;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ಗಂಟು = P.parse();
  Lambda_ಗಂಟು->render(Contexts, OS);
}

ಶೂನ್ಯ Template::render(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &Data, llvm::raw_ostream &OS) {
  RawMustacheOutputStream MOS(OS);
  ವೃಕ್ಷ->render(Data, MOS);
}

ಶೂನ್ಯ Template::registerPartial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_ವೃಕ್ಷ = P.parse();
  Ctx.Partials.ಸೇರಿಸು(std::make_pair(Name, Partial_ವೃಕ್ಷ));
}

ಶೂನ್ಯ Template::registerLambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

ಶೂನ್ಯ Template::registerLambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

ಶೂನ್ಯ Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  ವೃಕ್ಷ = P.parse();
  // The default behavior is to escape html entities.
  ಧ್ರುವ EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), ವೃಕ್ಷ(Other.ವೃಕ್ಷ) {
  Other.ವೃಕ್ಷ = nullptr;
}

Template::~Template() = ಸ್ವಭಾವ;

} // namespace llvm::mustache

#undef DEBUG_TYPE
