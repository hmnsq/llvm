//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#অন্তর্ভাৱ "llvm/Support/Mustache.h"
#অন্তর্ভাৱ "llvm/ADT/SmallVector.h"
#অন্তর্ভাৱ "llvm/Support/Debug.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"
#অন্তর্ভাৱ <sstream>

#define DEBUG_TYPE "mustache"

using নামাকাশ llvm;
using নামাকাশ llvm::mustache;

নামাকাশ {

using Accessor = ArrayRef<StringRef>;

স্থির তার্কিক isFalsey(ধ্রুৱ json::মান &V) {
  ঘূৰাই V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().মান_2()) ||
         (V.getAsArray() && V.getAsArray()->empty());
}

স্থির তার্কিক isContextFalsey(ধ্রুৱ json::মান *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  যদি (!V)
    ঘূৰাই সঁচা;
  ঘূৰাই isFalsey(*V);
}

স্থির শূন্য splitAndTrim(StringRef Str, SmallVectorImpl<StringRef> &Tokens) {
  size_t CurrentPos = 0;
  যেতিয়ালৈ (CurrentPos < Str.size()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    যদি (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.size();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.slice(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t Start = Part.find_first_not_of(" \t\r\n");
    যদি (Start != StringRef::npos) {
      size_t End = Part.find_last_not_of(" \t\r\n");
      Tokens.ঠেলো_back(Part.slice(Start, End + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

স্থির Accessor splitMustacheString(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  SmallVector<StringRef> Tokens;
  যদি (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    Tokens.ঠেলো_back(".");
  } অন্যথা {
    splitAndTrim(Str, Tokens);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.size());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  ঘূৰাই ArrayRef<StringRef>(ArenaTokens, Tokens.size());
}
} // namespace

নামাকাশ llvm::mustache {

শ্ৰেণী MustacheOutputStream : সার্ৱজনিক raw_ostream {
সার্ৱজনিক:
  MustacheOutputStream() = স্ৱভাৱ;
  ~MustacheOutputStream() override = স্ৱভাৱ;

  আভাস শূন্য suspendIndentation() {}
  আভাস শূন্য resumeIndentation() {}

গুপ্ত:
  শূন্য anchor() override;
};

শূন্য MustacheOutputStream::anchor() {}

শ্ৰেণী RawMustacheOutputStream : সার্ৱজনিক MustacheOutputStream {
সার্ৱজনিক:
  RawMustacheOutputStream(raw_ostream &OS) : OS(OS) { SetUnbuffered(); }

গুপ্ত:
  raw_ostream &OS;

  শূন্য write_impl(ধ্রুৱ ৱর্ণ *Ptr, size_t Size) override {
    OS.লিখ(Ptr, Size);
  }
  uint64_t current_pos() ধ্রুৱ override { ঘূৰাই OS.tell(); }
};

শ্ৰেণী Token {
সার্ৱজনিক:
  গণনা শ্ৰেণী Type {
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
        Accessor_মান({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, ৱর্ণ Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    যদি (TokenType == Type::Comment)
      ঘূৰাই;
    StringRef AccessorStr(এতত্->TokenBody);
    যদি (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_মান = splitMustacheString(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ধ্রুৱ { ঘূৰাই Accessor_মান; }

  Type getType() ধ্রুৱ { ঘূৰাই TokenType; }

  শূন্য setIndentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() ধ্রুৱ { ঘূৰাই Indentation; }

  স্থির Type getTokenType(ৱর্ণ Identifier) {
    ৱিকল্প (Identifier) {
    পক্ষ '#':
      ঘূৰাই Type::SectionOpen;
    পক্ষ '/':
      ঘূৰাই Type::SectionClose;
    পক্ষ '^':
      ঘূৰাই Type::InvertSectionOpen;
    পক্ষ '!':
      ঘূৰাই Type::Comment;
    পক্ষ '>':
      ঘূৰাই Type::Partial;
    পক্ষ '&':
      ঘূৰাই Type::UnescapeVariable;
    পক্ষ '=':
      ঘূৰাই Type::SetDelimiter;
    স্ৱভাৱ:
      ঘূৰাই Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_মান;
  size_t Indentation;
};

using EscapeMap = DenseMap<ৱর্ণ, std::string>;

শ্ৰেণী ASTNode : সার্ৱজনিক ilist_গ্ৰন্থি<ASTNode> {
সার্ৱজনিক:
  গণনা Type {
    মূল_2,
    Text,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::মূল_2), Parent(nullptr), ParentContext(nullptr) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *Parent)
      : Ctx(Ctx), Ty(Type::Text), Body(Body), Parent(Parent),
        ParentContext(nullptr) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *Parent)
      : Ctx(Ctx), Ty(Ty), Parent(Parent), Accessor_মান(Accessor),
        ParentContext(nullptr) {}

  শূন্য addChild(AstPtr Child) { Children.ঠেলো_back(Child); };

  শূন্য setRawBody(StringRef NewBody) { RawBody = NewBody; };

  শূন্য setIndentation(size_t NewIndentation) { Indentation = NewIndentation; };

  শূন্য render(ধ্রুৱ llvm::json::মান &Data, MustacheOutputStream &OS);

গুপ্ত:
  শূন্য renderLambdas(ধ্রুৱ llvm::json::মান &Contexts,
                     MustacheOutputStream &OS, Lambda &L);

  শূন্য renderSectionLambdas(ধ্রুৱ llvm::json::মান &Contexts,
                            MustacheOutputStream &OS, SectionLambda &L);

  শূন্য renderPartial(ধ্রুৱ llvm::json::মান &Contexts,
                     MustacheOutputStream &OS, ASTNode *Partial);

  শূন্য renderChild(ধ্রুৱ llvm::json::মান &Context, MustacheOutputStream &OS);

  ধ্রুৱ llvm::json::মান *findContext();

  শূন্য render_মূল(ধ্রুৱ json::মান &CurrentCtx, MustacheOutputStream &OS);
  শূন্য renderText(MustacheOutputStream &OS);
  শূন্য renderPartial(ধ্রুৱ json::মান &CurrentCtx, MustacheOutputStream &OS);
  শূন্য renderVariable(ধ্রুৱ json::মান &CurrentCtx, MustacheOutputStream &OS);
  শূন্য renderUnescapeVariable(ধ্রুৱ json::মান &CurrentCtx,
                              MustacheOutputStream &OS);
  শূন্য renderSection(ধ্রুৱ json::মান &CurrentCtx, MustacheOutputStream &OS);
  শূন্য renderInvertSection(ধ্রুৱ json::মান &CurrentCtx,
                           MustacheOutputStream &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  ধ্রুৱ ArrayRef<StringRef> Accessor_মান;
  ধ্রুৱ llvm::json::মান *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
স্থির AstPtr create_মূল_গ্ৰন্থি(MustacheContext &Ctx) {
  ঘূৰাই নূতন (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

স্থির AstPtr create_গ্ৰন্থি(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  ঘূৰাই নূতন (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

স্থির AstPtr create_Text_গ্ৰন্থি(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  ঘূৰাই নূতন (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
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
স্থির তার্কিক hasTextBehind(size_t Idx, ধ্রুৱ ArrayRef<Token> &Tokens) {
  যদি (Idx == 0)
    ঘূৰাই সঁচা;

  size_t PrevIdx = Idx - 1;
  যদি (Tokens[PrevIdx].getType() != Token::Type::Text)
    ঘূৰাই সঁচা;

  ধ্রুৱ Token &PrevToken = Tokens[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  ঘূৰাই !TokenBody.ends_with("\n") && !(TokenBody.empty() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
স্থির তার্কিক hasTextAhead(size_t Idx, ধ্রুৱ ArrayRef<Token> &Tokens) {
  যদি (Idx >= Tokens.size() - 1)
    ঘূৰাই সঁচা;

  size_t NextIdx = Idx + 1;
  যদি (Tokens[NextIdx].getType() != Token::Type::Text)
    ঘূৰাই সঁচা;

  ধ্রুৱ Token &NextToken = Tokens[NextIdx];
  StringRef TokenBody = StringRef(NextToken.RawBody).ltrim(" ");
  ঘূৰাই !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

স্থির তার্কিক requiresCleanUp(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  ঘূৰাই T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::SetDelimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
স্থির শূন্য stripTokenAhead(SmallVectorImpl<Token> &Tokens, size_t Idx) {
  Token &NextToken = Tokens[Idx + 1];
  StringRef NextTokenBody = NextToken.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  যদি (NextTokenBody.starts_with("\r\n"))
    NextToken.TokenBody = NextTokenBody.substr(2);
  অন্যথা যদি (NextTokenBody.starts_with("\n"))
    NextToken.TokenBody = NextTokenBody.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
শূন্য stripTokenBefore(SmallVectorImpl<Token> &Tokens, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = Tokens[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.size() - Unindented.size();
  PrevToken.TokenBody = Unindented;
  CurrentToken.setIndentation(Indentation);
}

সংরচনা Tag {
  গণনা শ্ৰেণী Kind {
    None,
    Normal, // {{...}}
    Triple, // {{{...}}}
  };

  Kind TagKind = Kind::None;
  StringRef Content;   // The content between the delimiters.
  StringRef FullMatch; // The entire tag, including delimiters.
  size_t StartPosition = StringRef::npos;
};

[[maybe_unused]] স্থির ধ্রুৱ ৱর্ণ *tagKindToString(Tag::Kind K) {
  ৱিকল্প (K) {
  পক্ষ Tag::Kind::None:
    ঘূৰাই "None";
  পক্ষ Tag::Kind::Normal:
    ঘূৰাই "Normal";
  পক্ষ Tag::Kind::Triple:
    ঘূৰাই "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_unused]] স্থির ধ্রুৱ ৱর্ণ *jsonKindToString(json::মান::Kind K) {
  ৱিকল্প (K) {
  পক্ষ json::মান::Kind::Null:
    ঘূৰাই "JSON_KIND_NULL";
  পক্ষ json::মান::Kind::Boolean:
    ঘূৰাই "JSON_KIND_BOOLEAN";
  পক্ষ json::মান::Kind::সংখ্যা:
    ঘূৰাই "JSON_KIND_NUMBER";
  পক্ষ json::মান::Kind::String:
    ঘূৰাই "JSON_KIND_STRING";
  পক্ষ json::মান::Kind::Array:
    ঘূৰাই "JSON_KIND_ARRAY";
  পক্ষ json::মান::Kind::Object:
    ঘূৰাই "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
স্থির SmallVector<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  SmallVector<Token> Tokens;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t TextStart = 0;

  ধ্রুৱ StringLiteral TripleOpen("{{{");
  ধ্রুৱ StringLiteral TripleClose("}}}");

  যেতিয়ালৈ (Cursor < Template.size()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef TagOpen, TagClose;
    Tag::Kind Kind;

    // Determine which tag we've encountered.
    যদি (TemplateSuffix.starts_with(TripleOpen)) {
      Kind = Tag::Kind::Triple;
      TagOpen = TripleOpen;
      TagClose = TripleClose;
    } অন্যথা যদি (TemplateSuffix.starts_with(Open)) {
      Kind = Tag::Kind::Normal;
      TagOpen = Open;
      TagClose = Close;
    } অন্যথা {
      // Not at a tag, continue scanning.
      ++Cursor;
      অনুৱর্ত;
    }

    // Found a tag, first add the preceding text.
    যদি (Cursor > TextStart)
      Tokens.emplace_back(Template.slice(TextStart, Cursor));

    // Find the closing tag.
    size_t EndPos = Template.find(TagClose, Cursor + TagOpen.size());
    যদি (EndPos == StringRef::npos) {
      // No closing tag, the rest is text.
      Tokens.emplace_back(Template.substr(Cursor));
      TextStart = Cursor = Template.size();
      ভঙ্গ;
    }

    // Extract tag content and full match.
    size_t ContentStart = Cursor + TagOpen.size();
    StringRef Content = Template.substr(ContentStart, EndPos - ContentStart);
    StringRef FullMatch =
        Template.substr(Cursor, (EndPos + TagClose.size()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << FullMatch << ", Content: " << Content
                      << ", Kind: " << tagKindToString(Kind) << "\n");
    যদি (Kind == Tag::Kind::Triple) {
      Tokens.emplace_back(FullMatch, Ctx.Saver.save("&" + Content), '&', Ctx);
    } অন্যথা { // Normal Tag
      StringRef Interpolated = Content;
      যদি (!Interpolated.trim().starts_with("=")) {
        ৱর্ণ Front = Interpolated.empty() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_back(FullMatch, Interpolated, Front, Ctx);
      } অন্যথা { // Set Delimiter
        Tokens.emplace_back(FullMatch, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](ৱর্ণ C) { ঘূৰাই C == '='; });
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
  যদি (TextStart < Template.size())
    Tokens.emplace_back(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  size_t LastIdx = Tokens.size() - 1;
  বাবে (size_t Idx = 0, End = Tokens.size(); Idx < End; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    যদি (!requiresCleanUp(CurrentType))
      অনুৱর্ত;

    তার্কিক HasTextBehind = hasTextBehind(Idx, Tokens);
    তার্কিক HasTextAhead = hasTextAhead(Idx, Tokens);

    যদি ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    যদি ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == LastIdx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  ঘূৰাই Tokens;
}

// Custom stream to escape strings.
শ্ৰেণী EscapeStringStream : সার্ৱজনিক MustacheOutputStream {
সার্ৱজনিক:
  explicit EscapeStringStream(llvm::raw_ostream &WrappedStream,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.চাবিগুলি().begin(), Escape.চাবিগুলি().end()),
        WrappedStream(WrappedStream) {
    SetUnbuffered();
  }

রক্ষিত:
  শূন্য write_impl(ধ্রুৱ ৱর্ণ *Ptr, size_t Size) override {
    StringRef Data(Ptr, Size);
    size_t Start = 0;
    যেতিয়ালৈ (Start < Size) {
      // Find the next character that needs to be escaped.
      size_t Next = Data.find_first_of(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      যদি (Next == StringRef::npos) {
        WrappedStream << Data.substr(Start);
        ঘূৰাই;
      }

      // Write the chunk of text before the escapable character.
      যদি (Next > Start)
        WrappedStream << Data.substr(Start, Next - Start);

      // Look up and write the escaped version of the character.
      WrappedStream << Escape[Data[Next]];
      Start = Next + 1;
    }
  }

  uint64_t current_pos() ধ্রুৱ override { ঘূৰাই WrappedStream.tell(); }

গুপ্ত:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &WrappedStream;
};

// Custom stream to add indentation used to for rendering partials.
শ্ৰেণী AddIndentationStringStream : সার্ৱজনিক MustacheOutputStream {
সার্ৱজনিক:
  explicit AddIndentationStringStream(raw_ostream &WrappedStream,
                                      size_t Indentation)
      : Indentation(Indentation), WrappedStream(WrappedStream),
        NeedsIndent(সঁচা), IsSuspended(মিছা) {
    SetUnbuffered();
  }

  শূন্য suspendIndentation() override { IsSuspended = সঁচা; }
  শূন্য resumeIndentation() override { IsSuspended = মিছা; }

রক্ষিত:
  শূন্য write_impl(ধ্রুৱ ৱর্ণ *Ptr, size_t Size) override {
    llvm::StringRef Data(Ptr, Size);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    বাবে (ৱর্ণ C : Data) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      যদি (NeedsIndent && C != '\n') {
        WrappedStream << Indent;
        NeedsIndent = মিছা;
      }
      WrappedStream << C;
      যদি (C == '\n' && !IsSuspended)
        NeedsIndent = সঁচা;
    }
  }

  uint64_t current_pos() ধ্রুৱ override { ঘূৰাই WrappedStream.tell(); }

গুপ্ত:
  size_t Indentation;
  raw_ostream &WrappedStream;
  তার্কিক NeedsIndent;
  তার্কিক IsSuspended;
};

শ্ৰেণী Parser {
সার্ৱজনিক:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

গুপ্ত:
  শূন্য parseMustache(ASTNode *Parent);
  শূন্য parseSection(ASTNode *Parent, ASTNode::Type Ty, ধ্রুৱ Accessor &A);

  MustacheContext &Ctx;
  SmallVector<Token> Tokens;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

শূন্য Parser::parseSection(ASTNode *Parent, ASTNode::Type Ty,
                          ধ্রুৱ Accessor &A) {
  AstPtr Current_গ্ৰন্থি = create_গ্ৰন্থি(Ctx, Ty, A, Parent);
  size_t Start = CurrentPtr;
  parseMustache(Current_গ্ৰন্থি);
  ধ্রুৱ size_t End = CurrentPtr - 1;

  size_t RawBodySize = 0;
  বাবে (size_t I = Start; I < End; ++I)
    RawBodySize += Tokens[I].RawBody.size();

  SmallString<128> RawBody;
  RawBody.reserve(RawBodySize);
  বাবে (std::size_t I = Start; I < End; ++I)
    RawBody += Tokens[I].RawBody;

  Current_গ্ৰন্থি->setRawBody(Ctx.Saver.save(StringRef(RawBody)));
  Parent->addChild(Current_গ্ৰন্থি);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr মূল_গ্ৰন্থি = create_মূল_গ্ৰন্থি(Ctx);
  parseMustache(মূল_গ্ৰন্থি);
  ঘূৰাই মূল_গ্ৰন্থি;
}

শূন্য Parser::parseMustache(ASTNode *Parent) {

  যেতিয়ালৈ (CurrentPtr < Tokens.size()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_গ্ৰন্থি;

    ৱিকল্প (CurrentToken.getType()) {
    পক্ষ Token::Type::Text: {
      Current_গ্ৰন্থি = create_Text_গ্ৰন্থি(Ctx, CurrentToken.TokenBody, Parent);
      Parent->addChild(Current_গ্ৰন্থি);
      ভঙ্গ;
    }
    পক্ষ Token::Type::Variable: {
      Current_গ্ৰন্থি = create_গ্ৰন্থি(Ctx, ASTNode::Variable, A, Parent);
      Parent->addChild(Current_গ্ৰন্থি);
      ভঙ্গ;
    }
    পক্ষ Token::Type::UnescapeVariable: {
      Current_গ্ৰন্থি = create_গ্ৰন্থি(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->addChild(Current_গ্ৰন্থি);
      ভঙ্গ;
    }
    পক্ষ Token::Type::Partial: {
      Current_গ্ৰন্থি = create_গ্ৰন্থি(Ctx, ASTNode::Partial, A, Parent);
      Current_গ্ৰন্থি->setIndentation(CurrentToken.getIndentation());
      Parent->addChild(Current_গ্ৰন্থি);
      ভঙ্গ;
    }
    পক্ষ Token::Type::SectionOpen: {
      parseSection(Parent, ASTNode::Section, A);
      ভঙ্গ;
    }
    পক্ষ Token::Type::InvertSectionOpen: {
      parseSection(Parent, ASTNode::InvertSection, A);
      ভঙ্গ;
    }
    পক্ষ Token::Type::Comment:
    পক্ষ Token::Type::SetDelimiter:
      ভঙ্গ;
    পক্ষ Token::Type::SectionClose:
      ঘূৰাই;
    }
  }
}
স্থির শূন্য toMustacheString(ধ্রুৱ json::মান &Data, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << jsonKindToString(Data.kind()) << ", Data: " << Data
                    << "\n");
  ৱিকল্প (Data.kind()) {
  পক্ষ json::মান::Null:
    ঘূৰাই;
  পক্ষ json::মান::সংখ্যা: {
    auto অঙ্ক = *Data.get_As_সংখ্যা();
    std::ostringstream SS;
    SS << অঙ্ক;
    OS << SS.str();
    ঘূৰাই;
  }
  পক্ষ json::মান::String: {
    OS << *Data.getAsString();
    ঘূৰাই;
  }

  পক্ষ json::মান::Array: {
    auto Arr = *Data.getAsArray();
    যদি (Arr.empty())
      ঘূৰাই;
    [[fallthrough]];
  }
  পক্ষ json::মান::Object:
  পক্ষ json::মান::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.মান_2(Data);
    ভঙ্গ;
  }
  }
}

শূন্য ASTNode::render_মূল(ধ্রুৱ json::মান &CurrentCtx,
                         MustacheOutputStream &OS) {
  renderChild(CurrentCtx, OS);
}

শূন্য ASTNode::renderText(MustacheOutputStream &OS) { OS << Body; }

শূন্য ASTNode::renderPartial(ধ্রুৱ json::মান &CurrentCtx,
                            MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_মান[0]
                    << ", Indentation:" << Indentation << "\n");
  auto Partial = Ctx.Partials.find(Accessor_মান[0]);
  যদি (Partial != Ctx.Partials.end())
    renderPartial(CurrentCtx, OS, Partial->get_মান());
}

শূন্য ASTNode::renderVariable(ধ্রুৱ json::মান &CurrentCtx,
                             MustacheOutputStream &OS) {
  auto Lambda = Ctx.Lambdas.find(Accessor_মান[0]);
  যদি (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_মান());
  } অন্যথা যদি (ধ্রুৱ json::মান *ContextPtr = findContext()) {
    EscapeStringStream ES(OS, Ctx.Escapes);
    toMustacheString(*ContextPtr, ES);
  }
}

শূন্য ASTNode::renderUnescapeVariable(ধ্রুৱ json::মান &CurrentCtx,
                                     MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_মান[0]
                    << "\n");
  auto Lambda = Ctx.Lambdas.find(Accessor_মান[0]);
  যদি (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_মান());
  } অন্যথা যদি (ধ্রুৱ json::মান *ContextPtr = findContext()) {
    OS.suspendIndentation();
    toMustacheString(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

শূন্য ASTNode::renderSection(ধ্রুৱ json::মান &CurrentCtx,
                            MustacheOutputStream &OS) {
  auto SectionLambda = Ctx.SectionLambdas.find(Accessor_মান[0]);
  যদি (SectionLambda != Ctx.SectionLambdas.end()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_মান());
    ঘূৰাই;
  }

  ধ্রুৱ json::মান *ContextPtr = findContext();
  যদি (isContextFalsey(ContextPtr))
    ঘূৰাই;

  যদি (ধ্রুৱ json::Array *Arr = ContextPtr->getAsArray()) {
    বাবে (ধ্রুৱ json::মান &V : *Arr)
      renderChild(V, OS);
    ঘূৰাই;
  }
  renderChild(*ContextPtr, OS);
}

শূন্য ASTNode::renderInvertSection(ধ্রুৱ json::মান &CurrentCtx,
                                  MustacheOutputStream &OS) {
  তার্কিক IsLambda = Ctx.SectionLambdas.contains(Accessor_মান[0]);
  ধ্রুৱ json::মান *ContextPtr = findContext();
  যদি (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

শূন্য ASTNode::render(ধ্রুৱ llvm::json::মান &Data, MustacheOutputStream &OS) {
  যদি (Ty != মূল_2 && Ty != Text && Accessor_মান.empty())
    ঘূৰাই;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &Data;

  ৱিকল্প (Ty) {
  পক্ষ মূল_2:
    render_মূল(Data, OS);
    ঘূৰাই;
  পক্ষ Text:
    renderText(OS);
    ঘূৰাই;
  পক্ষ Partial:
    renderPartial(Data, OS);
    ঘূৰাই;
  পক্ষ Variable:
    renderVariable(Data, OS);
    ঘূৰাই;
  পক্ষ UnescapeVariable:
    renderUnescapeVariable(Data, OS);
    ঘূৰাই;
  পক্ষ Section:
    renderSection(Data, OS);
    ঘূৰাই;
  পক্ষ InvertSection:
    renderInvertSection(Data, OS);
    ঘূৰাই;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ধ্রুৱ json::মান *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  যদি (Accessor_মান.empty())
    ঘূৰাই nullptr;
  যদি (Accessor_মান[0] == ".")
    ঘূৰাই ParentContext;

  ধ্রুৱ json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = Accessor_মান[0];
  ASTNode *CurrentParent = Parent;

  যেতিয়ালৈ (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    যদি (CurrentParent->Ty != মূল_2) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      অনুৱর্ত;
    }
    ঘূৰাই nullptr;
  }
  ধ্রুৱ json::মান *Context = nullptr;
  বাবে (auto [Idx, Acc] : enumerate(Accessor_মান)) {
    ধ্রুৱ json::মান *Current_মান = CurrentContext->get(Acc);
    যদি (!Current_মান)
      ঘূৰাই nullptr;
    যদি (Idx < Accessor_মান.size() - 1) {
      CurrentContext = Current_মান->getAsObject();
      যদি (!CurrentContext)
        ঘূৰাই nullptr;
    } অন্যথা {
      Context = Current_মান;
    }
  }
  ঘূৰাই Context;
}

শূন্য ASTNode::renderChild(ধ্রুৱ json::মান &Contexts,
                          MustacheOutputStream &OS) {
  বাবে (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

শূন্য ASTNode::renderPartial(ধ্রুৱ json::মান &Contexts,
                            MustacheOutputStream &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  AddIndentationStringStream IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

শূন্য ASTNode::renderLambdas(ধ্রুৱ llvm::json::মান &Contexts,
                            MustacheOutputStream &OS, Lambda &L) {
  json::মান LambdaResult = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(LambdaResult, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_গ্ৰন্থি = P.parse();

  EscapeStringStream ES(OS, Ctx.Escapes);
  যদি (Ty == Variable) {
    Lambda_গ্ৰন্থি->render(Contexts, ES);
    ঘূৰাই;
  }
  Lambda_গ্ৰন্থি->render(Contexts, OS);
}

শূন্য ASTNode::renderSectionLambdas(ধ্রুৱ llvm::json::মান &Contexts,
                                   MustacheOutputStream &OS, SectionLambda &L) {
  json::মান Return = L(RawBody.str());
  যদি (isFalsey(Return))
    ঘূৰাই;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_গ্ৰন্থি = P.parse();
  Lambda_গ্ৰন্থি->render(Contexts, OS);
}

শূন্য Template::render(ধ্রুৱ llvm::json::মান &Data, llvm::raw_ostream &OS) {
  RawMustacheOutputStream MOS(OS);
  বৃক্ষ->render(Data, MOS);
}

শূন্য Template::registerPartial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_বৃক্ষ = P.parse();
  Ctx.Partials.ঢোকাও(std::make_pair(Name, Partial_বৃক্ষ));
}

শূন্য Template::registerLambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

শূন্য Template::registerLambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

শূন্য Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  বৃক্ষ = P.parse();
  // The default behavior is to escape html entities.
  ধ্রুৱ EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), বৃক্ষ(Other.বৃক্ষ) {
  Other.বৃক্ষ = nullptr;
}

Template::~Template() = স্ৱভাৱ;

} // namespace llvm::mustache

#undef DEBUG_TYPE
