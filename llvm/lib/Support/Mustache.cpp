//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Mustache.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallVector.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <sstream>

#define DEBUG_TYPE "mustache"

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::mustache;

ନାମାକାଶ {

using Accessor = ArrayRef<StringRef>;

ସ୍ଥିର ତାର୍କିକ isFalsey(ଅଚଳ json::ମୂଲ୍ୟ &V) {
  ଫେରନ୍ତୁ V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().ମୂଲ୍ୟ_2()) ||
         (V.getAsArray() && V.getAsArray()->empty());
}

ସ୍ଥିର ତାର୍କିକ isContextFalsey(ଅଚଳ json::ମୂଲ୍ୟ *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  ଯଦି (!V)
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଫେରନ୍ତୁ isFalsey(*V);
}

ସ୍ଥିର ଶୂନ୍ୟ splitAndTrim(StringRef Str, SmallVectorImpl<StringRef> &Tokens) {
  size_t CurrentPos = 0;
  ଯେତେବେଳେ (CurrentPos < Str.size()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    ଯଦି (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.size();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.slice(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t Start = Part.find_first_not_of(" \t\r\n");
    ଯଦି (Start != StringRef::npos) {
      size_t End = Part.find_last_not_of(" \t\r\n");
      Tokens.ଠେଲ_back(Part.slice(Start, End + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

ସ୍ଥିର Accessor splitMustacheString(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  SmallVector<StringRef> Tokens;
  ଯଦି (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    Tokens.ଠେଲ_back(".");
  } ଅନ୍ୟଥା {
    splitAndTrim(Str, Tokens);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.size());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  ଫେରନ୍ତୁ ArrayRef<StringRef>(ArenaTokens, Tokens.size());
}
} // namespace

ନାମାକାଶ llvm::mustache {

ଶ୍ରେଣୀ MustacheOutputStream : ସର୍ବସାଧାରଣ raw_ostream {
ସର୍ବସାଧାରଣ:
  MustacheOutputStream() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;
  ~MustacheOutputStream() override = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

  ଆଭାସୀ ଶୂନ୍ୟ suspendIndentation() {}
  ଆଭାସୀ ଶୂନ୍ୟ resumeIndentation() {}

ବ୍ୟକ୍ତିଗତ:
  ଶୂନ୍ୟ anchor() override;
};

ଶୂନ୍ୟ MustacheOutputStream::anchor() {}

ଶ୍ରେଣୀ RawMustacheOutputStream : ସର୍ବସାଧାରଣ MustacheOutputStream {
ସର୍ବସାଧାରଣ:
  RawMustacheOutputStream(raw_ostream &OS) : OS(OS) { SetUnbuffered(); }

ବ୍ୟକ୍ତିଗତ:
  raw_ostream &OS;

  ଶୂନ୍ୟ write_impl(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size) override {
    OS.ଲିଖ(Ptr, Size);
  }
  uint64_t current_pos() ଅଚଳ override { ଫେରନ୍ତୁ OS.tell(); }
};

ଶ୍ରେଣୀ Token {
ସର୍ବସାଧାରଣ:
  ଗଣନା ଶ୍ରେଣୀ Type {
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
        Accessor_ମୂଲ୍ୟ({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, ଅକ୍ଷର Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    ଯଦି (TokenType == Type::Comment)
      ଫେରନ୍ତୁ;
    StringRef AccessorStr(ଏହି->TokenBody);
    ଯଦି (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_ମୂଲ୍ୟ = splitMustacheString(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ଅଚଳ { ଫେରନ୍ତୁ Accessor_ମୂଲ୍ୟ; }

  Type getType() ଅଚଳ { ଫେରନ୍ତୁ TokenType; }

  ଶୂନ୍ୟ setIndentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() ଅଚଳ { ଫେରନ୍ତୁ Indentation; }

  ସ୍ଥିର Type getTokenType(ଅକ୍ଷର Identifier) {
    ବାଛ (Identifier) {
    ସ୍ଥିତି '#':
      ଫେରନ୍ତୁ Type::SectionOpen;
    ସ୍ଥିତି '/':
      ଫେରନ୍ତୁ Type::SectionClose;
    ସ୍ଥିତି '^':
      ଫେରନ୍ତୁ Type::InvertSectionOpen;
    ସ୍ଥିତି '!':
      ଫେରନ୍ତୁ Type::Comment;
    ସ୍ଥିତି '>':
      ଫେରନ୍ତୁ Type::Partial;
    ସ୍ଥିତି '&':
      ଫେରନ୍ତୁ Type::UnescapeVariable;
    ସ୍ଥିତି '=':
      ଫେରନ୍ତୁ Type::SetDelimiter;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_ମୂଲ୍ୟ;
  size_t Indentation;
};

using EscapeMap = DenseMap<ଅକ୍ଷର, std::string>;

ଶ୍ରେଣୀ ASTNode : ସର୍ବସାଧାରଣ ilist_ଗଣ୍ଠି<ASTNode> {
ସର୍ବସାଧାରଣ:
  ଗଣନା Type {
    ମୂଳ_2,
    Text,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::ମୂଳ_2), Parent(nullptr), ParentContext(nullptr) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *Parent)
      : Ctx(Ctx), Ty(Type::Text), Body(Body), Parent(Parent),
        ParentContext(nullptr) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *Parent)
      : Ctx(Ctx), Ty(Ty), Parent(Parent), Accessor_ମୂଲ୍ୟ(Accessor),
        ParentContext(nullptr) {}

  ଶୂନ୍ୟ addChild(AstPtr Child) { Children.ଠେଲ_back(Child); };

  ଶୂନ୍ୟ setRawBody(StringRef NewBody) { RawBody = NewBody; };

  ଶୂନ୍ୟ setIndentation(size_t NewIndentation) { Indentation = NewIndentation; };

  ଶୂନ୍ୟ render(ଅଚଳ llvm::json::ମୂଲ୍ୟ &Data, MustacheOutputStream &OS);

ବ୍ୟକ୍ତିଗତ:
  ଶୂନ୍ୟ renderLambdas(ଅଚଳ llvm::json::ମୂଲ୍ୟ &Contexts,
                     MustacheOutputStream &OS, Lambda &L);

  ଶୂନ୍ୟ renderSectionLambdas(ଅଚଳ llvm::json::ମୂଲ୍ୟ &Contexts,
                            MustacheOutputStream &OS, SectionLambda &L);

  ଶୂନ୍ୟ renderPartial(ଅଚଳ llvm::json::ମୂଲ୍ୟ &Contexts,
                     MustacheOutputStream &OS, ASTNode *Partial);

  ଶୂନ୍ୟ renderChild(ଅଚଳ llvm::json::ମୂଲ୍ୟ &Context, MustacheOutputStream &OS);

  ଅଚଳ llvm::json::ମୂଲ୍ୟ *findContext();

  ଶୂନ୍ୟ render_ମୂଳ(ଅଚଳ json::ମୂଲ୍ୟ &CurrentCtx, MustacheOutputStream &OS);
  ଶୂନ୍ୟ renderText(MustacheOutputStream &OS);
  ଶୂନ୍ୟ renderPartial(ଅଚଳ json::ମୂଲ୍ୟ &CurrentCtx, MustacheOutputStream &OS);
  ଶୂନ୍ୟ renderVariable(ଅଚଳ json::ମୂଲ୍ୟ &CurrentCtx, MustacheOutputStream &OS);
  ଶୂନ୍ୟ renderUnescapeVariable(ଅଚଳ json::ମୂଲ୍ୟ &CurrentCtx,
                              MustacheOutputStream &OS);
  ଶୂନ୍ୟ renderSection(ଅଚଳ json::ମୂଲ୍ୟ &CurrentCtx, MustacheOutputStream &OS);
  ଶୂନ୍ୟ renderInvertSection(ଅଚଳ json::ମୂଲ୍ୟ &CurrentCtx,
                           MustacheOutputStream &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  ଅଚଳ ArrayRef<StringRef> Accessor_ମୂଲ୍ୟ;
  ଅଚଳ llvm::json::ମୂଲ୍ୟ *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
ସ୍ଥିର AstPtr create_ମୂଳ_ଗଣ୍ଠି(MustacheContext &Ctx) {
  ଫେରନ୍ତୁ ନୂତନ (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

ସ୍ଥିର AstPtr create_ଗଣ୍ଠି(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  ଫେରନ୍ତୁ ନୂତନ (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

ସ୍ଥିର AstPtr create_Text_ଗଣ୍ଠି(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  ଫେରନ୍ତୁ ନୂତନ (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
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
ସ୍ଥିର ତାର୍କିକ hasTextBehind(size_t Idx, ଅଚଳ ArrayRef<Token> &Tokens) {
  ଯଦି (Idx == 0)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  size_t PrevIdx = Idx - 1;
  ଯଦି (Tokens[PrevIdx].getType() != Token::Type::Text)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଅଚଳ Token &PrevToken = Tokens[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  ଫେରନ୍ତୁ !TokenBody.ends_with("\n") && !(TokenBody.empty() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
ସ୍ଥିର ତାର୍କିକ hasTextAhead(size_t Idx, ଅଚଳ ArrayRef<Token> &Tokens) {
  ଯଦି (Idx >= Tokens.size() - 1)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  size_t NextIdx = Idx + 1;
  ଯଦି (Tokens[NextIdx].getType() != Token::Type::Text)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଅଚଳ Token &NextToken = Tokens[NextIdx];
  StringRef TokenBody = StringRef(NextToken.RawBody).ltrim(" ");
  ଫେରନ୍ତୁ !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

ସ୍ଥିର ତାର୍କିକ requiresCleanUp(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  ଫେରନ୍ତୁ T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::SetDelimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
ସ୍ଥିର ଶୂନ୍ୟ stripTokenAhead(SmallVectorImpl<Token> &Tokens, size_t Idx) {
  Token &NextToken = Tokens[Idx + 1];
  StringRef NextTokenBody = NextToken.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  ଯଦି (NextTokenBody.starts_with("\r\n"))
    NextToken.TokenBody = NextTokenBody.substr(2);
  ଅନ୍ୟଥା ଯଦି (NextTokenBody.starts_with("\n"))
    NextToken.TokenBody = NextTokenBody.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
ଶୂନ୍ୟ stripTokenBefore(SmallVectorImpl<Token> &Tokens, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = Tokens[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.size() - Unindented.size();
  PrevToken.TokenBody = Unindented;
  CurrentToken.setIndentation(Indentation);
}

ସଂରଚନା Tag {
  ଗଣନା ଶ୍ରେଣୀ Kind {
    None,
    Normal, // {{...}}
    Triple, // {{{...}}}
  };

  Kind TagKind = Kind::None;
  StringRef Content;   // The content between the delimiters.
  StringRef FullMatch; // The entire tag, including delimiters.
  size_t StartPosition = StringRef::npos;
};

[[maybe_unused]] ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *tagKindToString(Tag::Kind K) {
  ବାଛ (K) {
  ସ୍ଥିତି Tag::Kind::None:
    ଫେରନ୍ତୁ "None";
  ସ୍ଥିତି Tag::Kind::Normal:
    ଫେରନ୍ତୁ "Normal";
  ସ୍ଥିତି Tag::Kind::Triple:
    ଫେରନ୍ତୁ "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_unused]] ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *jsonKindToString(json::ମୂଲ୍ୟ::Kind K) {
  ବାଛ (K) {
  ସ୍ଥିତି json::ମୂଲ୍ୟ::Kind::Null:
    ଫେରନ୍ତୁ "JSON_KIND_NULL";
  ସ୍ଥିତି json::ମୂଲ୍ୟ::Kind::Boolean:
    ଫେରନ୍ତୁ "JSON_KIND_BOOLEAN";
  ସ୍ଥିତି json::ମୂଲ୍ୟ::Kind::ସଂଖ୍ୟା:
    ଫେରନ୍ତୁ "JSON_KIND_NUMBER";
  ସ୍ଥିତି json::ମୂଲ୍ୟ::Kind::String:
    ଫେରନ୍ତୁ "JSON_KIND_STRING";
  ସ୍ଥିତି json::ମୂଲ୍ୟ::Kind::Array:
    ଫେରନ୍ତୁ "JSON_KIND_ARRAY";
  ସ୍ଥିତି json::ମୂଲ୍ୟ::Kind::Object:
    ଫେରନ୍ତୁ "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
ସ୍ଥିର SmallVector<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  SmallVector<Token> Tokens;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t TextStart = 0;

  ଅଚଳ StringLiteral TripleOpen("{{{");
  ଅଚଳ StringLiteral TripleClose("}}}");

  ଯେତେବେଳେ (Cursor < Template.size()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef TagOpen, TagClose;
    Tag::Kind Kind;

    // Determine which tag we've encountered.
    ଯଦି (TemplateSuffix.starts_with(TripleOpen)) {
      Kind = Tag::Kind::Triple;
      TagOpen = TripleOpen;
      TagClose = TripleClose;
    } ଅନ୍ୟଥା ଯଦି (TemplateSuffix.starts_with(Open)) {
      Kind = Tag::Kind::Normal;
      TagOpen = Open;
      TagClose = Close;
    } ଅନ୍ୟଥା {
      // Not at a tag, continue scanning.
      ++Cursor;
      ଜାରି;
    }

    // Found a tag, first add the preceding text.
    ଯଦି (Cursor > TextStart)
      Tokens.emplace_back(Template.slice(TextStart, Cursor));

    // Find the closing tag.
    size_t EndPos = Template.find(TagClose, Cursor + TagOpen.size());
    ଯଦି (EndPos == StringRef::npos) {
      // No closing tag, the rest is text.
      Tokens.emplace_back(Template.substr(Cursor));
      TextStart = Cursor = Template.size();
      ଭାଙ୍ଗ;
    }

    // Extract tag content and full match.
    size_t ContentStart = Cursor + TagOpen.size();
    StringRef Content = Template.substr(ContentStart, EndPos - ContentStart);
    StringRef FullMatch =
        Template.substr(Cursor, (EndPos + TagClose.size()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << FullMatch << ", Content: " << Content
                      << ", Kind: " << tagKindToString(Kind) << "\n");
    ଯଦି (Kind == Tag::Kind::Triple) {
      Tokens.emplace_back(FullMatch, Ctx.Saver.save("&" + Content), '&', Ctx);
    } ଅନ୍ୟଥା { // Normal Tag
      StringRef Interpolated = Content;
      ଯଦି (!Interpolated.trim().starts_with("=")) {
        ଅକ୍ଷର Front = Interpolated.empty() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_back(FullMatch, Interpolated, Front, Ctx);
      } ଅନ୍ୟଥା { // Set Delimiter
        Tokens.emplace_back(FullMatch, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](ଅକ୍ଷର C) { ଫେରନ୍ତୁ C == '='; });
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
  ଯଦି (TextStart < Template.size())
    Tokens.emplace_back(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  size_t LastIdx = Tokens.size() - 1;
  ପାଇଁ (size_t Idx = 0, End = Tokens.size(); Idx < End; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    ଯଦି (!requiresCleanUp(CurrentType))
      ଜାରି;

    ତାର୍କିକ HasTextBehind = hasTextBehind(Idx, Tokens);
    ତାର୍କିକ HasTextAhead = hasTextAhead(Idx, Tokens);

    ଯଦି ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    ଯଦି ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == LastIdx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  ଫେରନ୍ତୁ Tokens;
}

// Custom stream to escape strings.
ଶ୍ରେଣୀ EscapeStringStream : ସର୍ବସାଧାରଣ MustacheOutputStream {
ସର୍ବସାଧାରଣ:
  explicit EscapeStringStream(llvm::raw_ostream &WrappedStream,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.ଚାବିଗୁଡ଼ିକ().begin(), Escape.ଚାବିଗୁଡ଼ିକ().end()),
        WrappedStream(WrappedStream) {
    SetUnbuffered();
  }

ସୁରକ୍ଷିତ:
  ଶୂନ୍ୟ write_impl(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size) override {
    StringRef Data(Ptr, Size);
    size_t Start = 0;
    ଯେତେବେଳେ (Start < Size) {
      // Find the next character that needs to be escaped.
      size_t Next = Data.find_first_of(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      ଯଦି (Next == StringRef::npos) {
        WrappedStream << Data.substr(Start);
        ଫେରନ୍ତୁ;
      }

      // Write the chunk of text before the escapable character.
      ଯଦି (Next > Start)
        WrappedStream << Data.substr(Start, Next - Start);

      // Look up and write the escaped version of the character.
      WrappedStream << Escape[Data[Next]];
      Start = Next + 1;
    }
  }

  uint64_t current_pos() ଅଚଳ override { ଫେରନ୍ତୁ WrappedStream.tell(); }

ବ୍ୟକ୍ତିଗତ:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &WrappedStream;
};

// Custom stream to add indentation used to for rendering partials.
ଶ୍ରେଣୀ AddIndentationStringStream : ସର୍ବସାଧାରଣ MustacheOutputStream {
ସର୍ବସାଧାରଣ:
  explicit AddIndentationStringStream(raw_ostream &WrappedStream,
                                      size_t Indentation)
      : Indentation(Indentation), WrappedStream(WrappedStream),
        NeedsIndent(ସତ୍ୟ), IsSuspended(ମିଥ୍ୟା) {
    SetUnbuffered();
  }

  ଶୂନ୍ୟ suspendIndentation() override { IsSuspended = ସତ୍ୟ; }
  ଶୂନ୍ୟ resumeIndentation() override { IsSuspended = ମିଥ୍ୟା; }

ସୁରକ୍ଷିତ:
  ଶୂନ୍ୟ write_impl(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size) override {
    llvm::StringRef Data(Ptr, Size);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    ପାଇଁ (ଅକ୍ଷର C : Data) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      ଯଦି (NeedsIndent && C != '\n') {
        WrappedStream << Indent;
        NeedsIndent = ମିଥ୍ୟା;
      }
      WrappedStream << C;
      ଯଦି (C == '\n' && !IsSuspended)
        NeedsIndent = ସତ୍ୟ;
    }
  }

  uint64_t current_pos() ଅଚଳ override { ଫେରନ୍ତୁ WrappedStream.tell(); }

ବ୍ୟକ୍ତିଗତ:
  size_t Indentation;
  raw_ostream &WrappedStream;
  ତାର୍କିକ NeedsIndent;
  ତାର୍କିକ IsSuspended;
};

ଶ୍ରେଣୀ Parser {
ସର୍ବସାଧାରଣ:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

ବ୍ୟକ୍ତିଗତ:
  ଶୂନ୍ୟ parseMustache(ASTNode *Parent);
  ଶୂନ୍ୟ parseSection(ASTNode *Parent, ASTNode::Type Ty, ଅଚଳ Accessor &A);

  MustacheContext &Ctx;
  SmallVector<Token> Tokens;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

ଶୂନ୍ୟ Parser::parseSection(ASTNode *Parent, ASTNode::Type Ty,
                          ଅଚଳ Accessor &A) {
  AstPtr Current_ଗଣ୍ଠି = create_ଗଣ୍ଠି(Ctx, Ty, A, Parent);
  size_t Start = CurrentPtr;
  parseMustache(Current_ଗଣ୍ଠି);
  ଅଚଳ size_t End = CurrentPtr - 1;

  size_t RawBodySize = 0;
  ପାଇଁ (size_t I = Start; I < End; ++I)
    RawBodySize += Tokens[I].RawBody.size();

  SmallString<128> RawBody;
  RawBody.reserve(RawBodySize);
  ପାଇଁ (std::size_t I = Start; I < End; ++I)
    RawBody += Tokens[I].RawBody;

  Current_ଗଣ୍ଠି->setRawBody(Ctx.Saver.save(StringRef(RawBody)));
  Parent->addChild(Current_ଗଣ୍ଠି);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr ମୂଳ_ଗଣ୍ଠି = create_ମୂଳ_ଗଣ୍ଠି(Ctx);
  parseMustache(ମୂଳ_ଗଣ୍ଠି);
  ଫେରନ୍ତୁ ମୂଳ_ଗଣ୍ଠି;
}

ଶୂନ୍ୟ Parser::parseMustache(ASTNode *Parent) {

  ଯେତେବେଳେ (CurrentPtr < Tokens.size()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_ଗଣ୍ଠି;

    ବାଛ (CurrentToken.getType()) {
    ସ୍ଥିତି Token::Type::Text: {
      Current_ଗଣ୍ଠି = create_Text_ଗଣ୍ଠି(Ctx, CurrentToken.TokenBody, Parent);
      Parent->addChild(Current_ଗଣ୍ଠି);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି Token::Type::Variable: {
      Current_ଗଣ୍ଠି = create_ଗଣ୍ଠି(Ctx, ASTNode::Variable, A, Parent);
      Parent->addChild(Current_ଗଣ୍ଠି);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି Token::Type::UnescapeVariable: {
      Current_ଗଣ୍ଠି = create_ଗଣ୍ଠି(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->addChild(Current_ଗଣ୍ଠି);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି Token::Type::Partial: {
      Current_ଗଣ୍ଠି = create_ଗଣ୍ଠି(Ctx, ASTNode::Partial, A, Parent);
      Current_ଗଣ୍ଠି->setIndentation(CurrentToken.getIndentation());
      Parent->addChild(Current_ଗଣ୍ଠି);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି Token::Type::SectionOpen: {
      parseSection(Parent, ASTNode::Section, A);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି Token::Type::InvertSectionOpen: {
      parseSection(Parent, ASTNode::InvertSection, A);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି Token::Type::Comment:
    ସ୍ଥିତି Token::Type::SetDelimiter:
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::Type::SectionClose:
      ଫେରନ୍ତୁ;
    }
  }
}
ସ୍ଥିର ଶୂନ୍ୟ toMustacheString(ଅଚଳ json::ମୂଲ୍ୟ &Data, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << jsonKindToString(Data.kind()) << ", Data: " << Data
                    << "\n");
  ବାଛ (Data.kind()) {
  ସ୍ଥିତି json::ମୂଲ୍ୟ::Null:
    ଫେରନ୍ତୁ;
  ସ୍ଥିତି json::ମୂଲ୍ୟ::ସଂଖ୍ୟା: {
    auto ଅଙ୍କ = *Data.get_As_ସଂଖ୍ୟା();
    std::ostringstream SS;
    SS << ଅଙ୍କ;
    OS << SS.str();
    ଫେରନ୍ତୁ;
  }
  ସ୍ଥିତି json::ମୂଲ୍ୟ::String: {
    OS << *Data.getAsString();
    ଫେରନ୍ତୁ;
  }

  ସ୍ଥିତି json::ମୂଲ୍ୟ::Array: {
    auto Arr = *Data.getAsArray();
    ଯଦି (Arr.empty())
      ଫେରନ୍ତୁ;
    [[fallthrough]];
  }
  ସ୍ଥିତି json::ମୂଲ୍ୟ::Object:
  ସ୍ଥିତି json::ମୂଲ୍ୟ::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.ମୂଲ୍ୟ_2(Data);
    ଭାଙ୍ଗ;
  }
  }
}

ଶୂନ୍ୟ ASTNode::render_ମୂଳ(ଅଚଳ json::ମୂଲ୍ୟ &CurrentCtx,
                         MustacheOutputStream &OS) {
  renderChild(CurrentCtx, OS);
}

ଶୂନ୍ୟ ASTNode::renderText(MustacheOutputStream &OS) { OS << Body; }

ଶୂନ୍ୟ ASTNode::renderPartial(ଅଚଳ json::ମୂଲ୍ୟ &CurrentCtx,
                            MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_ମୂଲ୍ୟ[0]
                    << ", Indentation:" << Indentation << "\n");
  auto Partial = Ctx.Partials.find(Accessor_ମୂଲ୍ୟ[0]);
  ଯଦି (Partial != Ctx.Partials.end())
    renderPartial(CurrentCtx, OS, Partial->get_ମୂଲ୍ୟ());
}

ଶୂନ୍ୟ ASTNode::renderVariable(ଅଚଳ json::ମୂଲ୍ୟ &CurrentCtx,
                             MustacheOutputStream &OS) {
  auto Lambda = Ctx.Lambdas.find(Accessor_ମୂଲ୍ୟ[0]);
  ଯଦି (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_ମୂଲ୍ୟ());
  } ଅନ୍ୟଥା ଯଦି (ଅଚଳ json::ମୂଲ୍ୟ *ContextPtr = findContext()) {
    EscapeStringStream ES(OS, Ctx.Escapes);
    toMustacheString(*ContextPtr, ES);
  }
}

ଶୂନ୍ୟ ASTNode::renderUnescapeVariable(ଅଚଳ json::ମୂଲ୍ୟ &CurrentCtx,
                                     MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_ମୂଲ୍ୟ[0]
                    << "\n");
  auto Lambda = Ctx.Lambdas.find(Accessor_ମୂଲ୍ୟ[0]);
  ଯଦି (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_ମୂଲ୍ୟ());
  } ଅନ୍ୟଥା ଯଦି (ଅଚଳ json::ମୂଲ୍ୟ *ContextPtr = findContext()) {
    OS.suspendIndentation();
    toMustacheString(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

ଶୂନ୍ୟ ASTNode::renderSection(ଅଚଳ json::ମୂଲ୍ୟ &CurrentCtx,
                            MustacheOutputStream &OS) {
  auto SectionLambda = Ctx.SectionLambdas.find(Accessor_ମୂଲ୍ୟ[0]);
  ଯଦି (SectionLambda != Ctx.SectionLambdas.end()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_ମୂଲ୍ୟ());
    ଫେରନ୍ତୁ;
  }

  ଅଚଳ json::ମୂଲ୍ୟ *ContextPtr = findContext();
  ଯଦି (isContextFalsey(ContextPtr))
    ଫେରନ୍ତୁ;

  ଯଦି (ଅଚଳ json::Array *Arr = ContextPtr->getAsArray()) {
    ପାଇଁ (ଅଚଳ json::ମୂଲ୍ୟ &V : *Arr)
      renderChild(V, OS);
    ଫେରନ୍ତୁ;
  }
  renderChild(*ContextPtr, OS);
}

ଶୂନ୍ୟ ASTNode::renderInvertSection(ଅଚଳ json::ମୂଲ୍ୟ &CurrentCtx,
                                  MustacheOutputStream &OS) {
  ତାର୍କିକ IsLambda = Ctx.SectionLambdas.contains(Accessor_ମୂଲ୍ୟ[0]);
  ଅଚଳ json::ମୂଲ୍ୟ *ContextPtr = findContext();
  ଯଦି (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

ଶୂନ୍ୟ ASTNode::render(ଅଚଳ llvm::json::ମୂଲ୍ୟ &Data, MustacheOutputStream &OS) {
  ଯଦି (Ty != ମୂଳ_2 && Ty != Text && Accessor_ମୂଲ୍ୟ.empty())
    ଫେରନ୍ତୁ;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &Data;

  ବାଛ (Ty) {
  ସ୍ଥିତି ମୂଳ_2:
    render_ମୂଳ(Data, OS);
    ଫେରନ୍ତୁ;
  ସ୍ଥିତି Text:
    renderText(OS);
    ଫେରନ୍ତୁ;
  ସ୍ଥିତି Partial:
    renderPartial(Data, OS);
    ଫେରନ୍ତୁ;
  ସ୍ଥିତି Variable:
    renderVariable(Data, OS);
    ଫେରନ୍ତୁ;
  ସ୍ଥିତି UnescapeVariable:
    renderUnescapeVariable(Data, OS);
    ଫେରନ୍ତୁ;
  ସ୍ଥିତି Section:
    renderSection(Data, OS);
    ଫେରନ୍ତୁ;
  ସ୍ଥିତି InvertSection:
    renderInvertSection(Data, OS);
    ଫେରନ୍ତୁ;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ଅଚଳ json::ମୂଲ୍ୟ *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  ଯଦି (Accessor_ମୂଲ୍ୟ.empty())
    ଫେରନ୍ତୁ nullptr;
  ଯଦି (Accessor_ମୂଲ୍ୟ[0] == ".")
    ଫେରନ୍ତୁ ParentContext;

  ଅଚଳ json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = Accessor_ମୂଲ୍ୟ[0];
  ASTNode *CurrentParent = Parent;

  ଯେତେବେଳେ (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    ଯଦି (CurrentParent->Ty != ମୂଳ_2) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      ଜାରି;
    }
    ଫେରନ୍ତୁ nullptr;
  }
  ଅଚଳ json::ମୂଲ୍ୟ *Context = nullptr;
  ପାଇଁ (auto [Idx, Acc] : enumerate(Accessor_ମୂଲ୍ୟ)) {
    ଅଚଳ json::ମୂଲ୍ୟ *Current_ମୂଲ୍ୟ = CurrentContext->get(Acc);
    ଯଦି (!Current_ମୂଲ୍ୟ)
      ଫେରନ୍ତୁ nullptr;
    ଯଦି (Idx < Accessor_ମୂଲ୍ୟ.size() - 1) {
      CurrentContext = Current_ମୂଲ୍ୟ->getAsObject();
      ଯଦି (!CurrentContext)
        ଫେରନ୍ତୁ nullptr;
    } ଅନ୍ୟଥା {
      Context = Current_ମୂଲ୍ୟ;
    }
  }
  ଫେରନ୍ତୁ Context;
}

ଶୂନ୍ୟ ASTNode::renderChild(ଅଚଳ json::ମୂଲ୍ୟ &Contexts,
                          MustacheOutputStream &OS) {
  ପାଇଁ (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

ଶୂନ୍ୟ ASTNode::renderPartial(ଅଚଳ json::ମୂଲ୍ୟ &Contexts,
                            MustacheOutputStream &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  AddIndentationStringStream IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

ଶୂନ୍ୟ ASTNode::renderLambdas(ଅଚଳ llvm::json::ମୂଲ୍ୟ &Contexts,
                            MustacheOutputStream &OS, Lambda &L) {
  json::ମୂଲ୍ୟ LambdaResult = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(LambdaResult, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ଗଣ୍ଠି = P.parse();

  EscapeStringStream ES(OS, Ctx.Escapes);
  ଯଦି (Ty == Variable) {
    Lambda_ଗଣ୍ଠି->render(Contexts, ES);
    ଫେରନ୍ତୁ;
  }
  Lambda_ଗଣ୍ଠି->render(Contexts, OS);
}

ଶୂନ୍ୟ ASTNode::renderSectionLambdas(ଅଚଳ llvm::json::ମୂଲ୍ୟ &Contexts,
                                   MustacheOutputStream &OS, SectionLambda &L) {
  json::ମୂଲ୍ୟ Return = L(RawBody.str());
  ଯଦି (isFalsey(Return))
    ଫେରନ୍ତୁ;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ଗଣ୍ଠି = P.parse();
  Lambda_ଗଣ୍ଠି->render(Contexts, OS);
}

ଶୂନ୍ୟ Template::render(ଅଚଳ llvm::json::ମୂଲ୍ୟ &Data, llvm::raw_ostream &OS) {
  RawMustacheOutputStream MOS(OS);
  ବୃକ୍ଷ->render(Data, MOS);
}

ଶୂନ୍ୟ Template::registerPartial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_ବୃକ୍ଷ = P.parse();
  Ctx.Partials.ଭର୍ତ୍ତି(std::make_pair(Name, Partial_ବୃକ୍ଷ));
}

ଶୂନ୍ୟ Template::registerLambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

ଶୂନ୍ୟ Template::registerLambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

ଶୂନ୍ୟ Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  ବୃକ୍ଷ = P.parse();
  // The default behavior is to escape html entities.
  ଅଚଳ EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), ବୃକ୍ଷ(Other.ବୃକ୍ଷ) {
  Other.ବୃକ୍ଷ = nullptr;
}

Template::~Template() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

} // namespace llvm::mustache

#undef DEBUG_TYPE
