//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#অন্তর্ভাব "llvm/Support/Mustache.h"
#অন্তর্ভাব "llvm/ADT/SmallVector.h"
#অন্তর্ভাব "llvm/Support/Debug.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <sstream>

#define DEBUG_TYPE "mustache"

using নামাকাশ llvm;
using নামাকাশ llvm::mustache;

নামাকাশ {

using Accessor = ArrayRef<StringRef>;

স্থির যৌক্তিক isFalsey(ধ্রুবক json::মান &V) {
  ফেরত V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().মান_2()) ||
         (V.getAsArray() && V.getAsArray()->empty());
}

স্থির যৌক্তিক isContextFalsey(ধ্রুবক json::মান *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  যদি (!V)
    ফেরত সত্য;
  ফেরত isFalsey(*V);
}

স্থির শূন্য splitAndTrim(StringRef Str, SmallVectorImpl<StringRef> &Tokens) {
  size_t CurrentPos = 0;
  যখন (CurrentPos < Str.আকার_3()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    যদি (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.আকার_3();

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
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.আকার_3());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  ফেরত ArrayRef<StringRef>(ArenaTokens, Tokens.আকার_3());
}
} // namespace

নামাকাশ llvm::mustache {

শ্রেণী MustacheOutputStream : সর্বজনীন raw_ostream {
সর্বজনীন:
  MustacheOutputStream() = পূর্বনির্ধারিত;
  ~MustacheOutputStream() override = পূর্বনির্ধারিত;

  আভাসী শূন্য suspendIndentation() {}
  আভাসী শূন্য resumeIndentation() {}

ব্যক্তিগত:
  শূন্য anchor() override;
};

শূন্য MustacheOutputStream::anchor() {}

শ্রেণী RawMustacheOutputStream : সর্বজনীন MustacheOutputStream {
সর্বজনীন:
  RawMustacheOutputStream(raw_ostream &OS) : OS(OS) { SetUnbuffered(); }

ব্যক্তিগত:
  raw_ostream &OS;

  শূন্য write_impl(ধ্রুবক অক্ষর *Ptr, size_t আকার_2) override {
    OS.লিখ(Ptr, আকার_2);
  }
  uint64_t current_pos() ধ্রুবক override { ফেরত OS.tell(); }
};

শ্রেণী Token {
সর্বজনীন:
  গণনা শ্রেণী Type {
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

  Token(StringRef RawBody, StringRef TokenBody, অক্ষর Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    যদি (TokenType == Type::Comment)
      ফেরত;
    StringRef AccessorStr(এই->TokenBody);
    যদি (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_মান = splitMustacheString(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ধ্রুবক { ফেরত Accessor_মান; }

  Type getType() ধ্রুবক { ফেরত TokenType; }

  শূন্য setIndentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() ধ্রুবক { ফেরত Indentation; }

  স্থির Type getTokenType(অক্ষর Identifier) {
    নির্বাচন (Identifier) {
    ক্ষেত্র '#':
      ফেরত Type::SectionOpen;
    ক্ষেত্র '/':
      ফেরত Type::SectionClose;
    ক্ষেত্র '^':
      ফেরত Type::InvertSectionOpen;
    ক্ষেত্র '!':
      ফেরত Type::Comment;
    ক্ষেত্র '>':
      ফেরত Type::Partial;
    ক্ষেত্র '&':
      ফেরত Type::UnescapeVariable;
    ক্ষেত্র '=':
      ফেরত Type::SetDelimiter;
    পূর্বনির্ধারিত:
      ফেরত Type::Variable;
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

using EscapeMap = DenseMap<অক্ষর, std::string>;

শ্রেণী ASTNode : সর্বজনীন ilist_গ্রন্থি<ASTNode> {
সর্বজনীন:
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

  শূন্য render(ধ্রুবক llvm::json::মান &তথ্য_2, MustacheOutputStream &OS);

ব্যক্তিগত:
  শূন্য renderLambdas(ধ্রুবক llvm::json::মান &Contexts,
                     MustacheOutputStream &OS, Lambda &L);

  শূন্য renderSectionLambdas(ধ্রুবক llvm::json::মান &Contexts,
                            MustacheOutputStream &OS, SectionLambda &L);

  শূন্য renderPartial(ধ্রুবক llvm::json::মান &Contexts,
                     MustacheOutputStream &OS, ASTNode *Partial);

  শূন্য renderChild(ধ্রুবক llvm::json::মান &Context, MustacheOutputStream &OS);

  ধ্রুবক llvm::json::মান *findContext();

  শূন্য render_মূল(ধ্রুবক json::মান &CurrentCtx, MustacheOutputStream &OS);
  শূন্য renderText(MustacheOutputStream &OS);
  শূন্য renderPartial(ধ্রুবক json::মান &CurrentCtx, MustacheOutputStream &OS);
  শূন্য renderVariable(ধ্রুবক json::মান &CurrentCtx, MustacheOutputStream &OS);
  শূন্য renderUnescapeVariable(ধ্রুবক json::মান &CurrentCtx,
                              MustacheOutputStream &OS);
  শূন্য renderSection(ধ্রুবক json::মান &CurrentCtx, MustacheOutputStream &OS);
  শূন্য renderInvertSection(ধ্রুবক json::মান &CurrentCtx,
                           MustacheOutputStream &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  ধ্রুবক ArrayRef<StringRef> Accessor_মান;
  ধ্রুবক llvm::json::মান *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
স্থির AstPtr create_মূল_গ্রন্থি(MustacheContext &Ctx) {
  ফেরত নতুন (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

স্থির AstPtr create_গ্রন্থি(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  ফেরত নতুন (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

স্থির AstPtr create_Text_গ্রন্থি(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  ফেরত নতুন (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
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
স্থির যৌক্তিক hasTextBehind(size_t Idx, ধ্রুবক ArrayRef<Token> &Tokens) {
  যদি (Idx == 0)
    ফেরত সত্য;

  size_t পূর্ব_Idx = Idx - 1;
  যদি (Tokens[পূর্ব_Idx].getType() != Token::Type::Text)
    ফেরত সত্য;

  ধ্রুবক Token &পূর্ব_Token = Tokens[পূর্ব_Idx];
  StringRef TokenBody = StringRef(পূর্ব_Token.RawBody).rtrim(" \r\t\v");
  ফেরত !TokenBody.ends_with("\n") && !(TokenBody.empty() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
স্থির যৌক্তিক hasTextAhead(size_t Idx, ধ্রুবক ArrayRef<Token> &Tokens) {
  যদি (Idx >= Tokens.আকার_3() - 1)
    ফেরত সত্য;

  size_t পরবর্তী_Idx = Idx + 1;
  যদি (Tokens[পরবর্তী_Idx].getType() != Token::Type::Text)
    ফেরত সত্য;

  ধ্রুবক Token &পরবর্তী_Token = Tokens[পরবর্তী_Idx];
  StringRef TokenBody = StringRef(পরবর্তী_Token.RawBody).ltrim(" ");
  ফেরত !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

স্থির যৌক্তিক requiresCleanUp(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  ফেরত T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
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
  Token &পরবর্তী_Token = Tokens[Idx + 1];
  StringRef পরবর্তী_Token_Body = পরবর্তী_Token.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  যদি (পরবর্তী_Token_Body.starts_with("\r\n"))
    পরবর্তী_Token.TokenBody = পরবর্তী_Token_Body.substr(2);
  অন্যথা যদি (পরবর্তী_Token_Body.starts_with("\n"))
    পরবর্তী_Token.TokenBody = পরবর্তী_Token_Body.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
শূন্য stripTokenBefore(SmallVectorImpl<Token> &Tokens, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &পূর্ব_Token = Tokens[Idx - 1];
  StringRef পূর্ব_Token_Body = পূর্ব_Token.TokenBody;
  StringRef Unindented = পূর্ব_Token_Body.rtrim(" \r\t\v");
  size_t Indentation = পূর্ব_Token_Body.আকার_3() - Unindented.আকার_3();
  পূর্ব_Token.TokenBody = Unindented;
  CurrentToken.setIndentation(Indentation);
}

কাঠামো Tag {
  গণনা শ্রেণী Kind {
    None,
    Normal, // {{...}}
    Triple, // {{{...}}}
  };

  Kind TagKind = Kind::None;
  StringRef Content;   // The content between the delimiters.
  StringRef FullMatch; // The entire tag, including delimiters.
  size_t StartPosition = StringRef::npos;
};

[[maybe_unused]] স্থির ধ্রুবক অক্ষর *tagKindToString(Tag::Kind K) {
  নির্বাচন (K) {
  ক্ষেত্র Tag::Kind::None:
    ফেরত "None";
  ক্ষেত্র Tag::Kind::Normal:
    ফেরত "Normal";
  ক্ষেত্র Tag::Kind::Triple:
    ফেরত "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_unused]] স্থির ধ্রুবক অক্ষর *jsonKindToString(json::মান::Kind K) {
  নির্বাচন (K) {
  ক্ষেত্র json::মান::Kind::Null:
    ফেরত "JSON_KIND_NULL";
  ক্ষেত্র json::মান::Kind::Boolean:
    ফেরত "JSON_KIND_BOOLEAN";
  ক্ষেত্র json::মান::Kind::সংখ্যা:
    ফেরত "JSON_KIND_NUMBER";
  ক্ষেত্র json::মান::Kind::String:
    ফেরত "JSON_KIND_STRING";
  ক্ষেত্র json::মান::Kind::Array:
    ফেরত "JSON_KIND_ARRAY";
  ক্ষেত্র json::মান::Kind::Object:
    ফেরত "JSON_KIND_OBJECT";
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

  ধ্রুবক StringLiteral TripleOpen("{{{");
  ধ্রুবক StringLiteral TripleClose("}}}");

  যখন (Cursor < Template.আকার_3()) {
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
      চালু;
    }

    // Found a tag, first add the preceding text.
    যদি (Cursor > TextStart)
      Tokens.emplace_back(Template.slice(TextStart, Cursor));

    // Find the closing tag.
    size_t EndPos = Template.find(TagClose, Cursor + TagOpen.আকার_3());
    যদি (EndPos == StringRef::npos) {
      // No closing tag, the rest is text.
      Tokens.emplace_back(Template.substr(Cursor));
      TextStart = Cursor = Template.আকার_3();
      বিরতি;
    }

    // Extract tag content and full match.
    size_t ContentStart = Cursor + TagOpen.আকার_3();
    StringRef Content = Template.substr(ContentStart, EndPos - ContentStart);
    StringRef FullMatch =
        Template.substr(Cursor, (EndPos + TagClose.আকার_3()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << FullMatch << ", Content: " << Content
                      << ", Kind: " << tagKindToString(Kind) << "\n");
    যদি (Kind == Tag::Kind::Triple) {
      Tokens.emplace_back(FullMatch, Ctx.Saver.save("&" + Content), '&', Ctx);
    } অন্যথা { // Normal Tag
      StringRef Interpolated = Content;
      যদি (!Interpolated.trim().starts_with("=")) {
        অক্ষর Front = Interpolated.empty() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_back(FullMatch, Interpolated, Front, Ctx);
      } অন্যথা { // Set Delimiter
        Tokens.emplace_back(FullMatch, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](অক্ষর C) { ফেরত C == '='; });
        DelimSpec = DelimSpec.trim();

        auto [NewOpen, NewClose] = DelimSpec.split(' ');
        LLVM_DEBUG(dbgs() << "[Set Delimiter] NewOpen: " << NewOpen
                          << ", NewClose: " << NewClose << "\n");
        Open = NewOpen;
        Close = NewClose;
      }
    }

    // Move past the tag for the next iteration.
    Cursor += FullMatch.আকার_3();
    TextStart = Cursor;
  }

  // Add any remaining text after the last tag.
  যদি (TextStart < Template.আকার_3())
    Tokens.emplace_back(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  size_t LastIdx = Tokens.আকার_3() - 1;
  জন্য (size_t Idx = 0, End = Tokens.আকার_3(); Idx < End; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    যদি (!requiresCleanUp(CurrentType))
      চালু;

    যৌক্তিক HasTextBehind = hasTextBehind(Idx, Tokens);
    যৌক্তিক HasTextAhead = hasTextAhead(Idx, Tokens);

    যদি ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    যদি ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == LastIdx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  ফেরত Tokens;
}

// Custom stream to escape strings.
শ্রেণী EscapeStringStream : সর্বজনীন MustacheOutputStream {
সর্বজনীন:
  explicit EscapeStringStream(llvm::raw_ostream &WrappedStream,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.চাবিগুলি().begin(), Escape.চাবিগুলি().end()),
        WrappedStream(WrappedStream) {
    SetUnbuffered();
  }

সুরক্ষিত:
  শূন্য write_impl(ধ্রুবক অক্ষর *Ptr, size_t আকার_2) override {
    StringRef তথ্য_2(Ptr, আকার_2);
    size_t Start = 0;
    যখন (Start < আকার_2) {
      // Find the next character that needs to be escaped.
      size_t পরবর্তী_2 = তথ্য_2.find_first_of(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      যদি (পরবর্তী_2 == StringRef::npos) {
        WrappedStream << তথ্য_2.substr(Start);
        ফেরত;
      }

      // Write the chunk of text before the escapable character.
      যদি (পরবর্তী_2 > Start)
        WrappedStream << তথ্য_2.substr(Start, পরবর্তী_2 - Start);

      // Look up and write the escaped version of the character.
      WrappedStream << Escape[তথ্য_2[পরবর্তী_2]];
      Start = পরবর্তী_2 + 1;
    }
  }

  uint64_t current_pos() ধ্রুবক override { ফেরত WrappedStream.tell(); }

ব্যক্তিগত:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &WrappedStream;
};

// Custom stream to add indentation used to for rendering partials.
শ্রেণী AddIndentationStringStream : সর্বজনীন MustacheOutputStream {
সর্বজনীন:
  explicit AddIndentationStringStream(raw_ostream &WrappedStream,
                                      size_t Indentation)
      : Indentation(Indentation), WrappedStream(WrappedStream),
        NeedsIndent(সত্য), IsSuspended(মিথ্যা) {
    SetUnbuffered();
  }

  শূন্য suspendIndentation() override { IsSuspended = সত্য; }
  শূন্য resumeIndentation() override { IsSuspended = মিথ্যা; }

সুরক্ষিত:
  শূন্য write_impl(ধ্রুবক অক্ষর *Ptr, size_t আকার_2) override {
    llvm::StringRef তথ্য_2(Ptr, আকার_2);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    জন্য (অক্ষর C : তথ্য_2) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      যদি (NeedsIndent && C != '\n') {
        WrappedStream << Indent;
        NeedsIndent = মিথ্যা;
      }
      WrappedStream << C;
      যদি (C == '\n' && !IsSuspended)
        NeedsIndent = সত্য;
    }
  }

  uint64_t current_pos() ধ্রুবক override { ফেরত WrappedStream.tell(); }

ব্যক্তিগত:
  size_t Indentation;
  raw_ostream &WrappedStream;
  যৌক্তিক NeedsIndent;
  যৌক্তিক IsSuspended;
};

শ্রেণী Parser {
সর্বজনীন:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

ব্যক্তিগত:
  শূন্য parseMustache(ASTNode *Parent);
  শূন্য parseSection(ASTNode *Parent, ASTNode::Type Ty, ধ্রুবক Accessor &A);

  MustacheContext &Ctx;
  SmallVector<Token> Tokens;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

শূন্য Parser::parseSection(ASTNode *Parent, ASTNode::Type Ty,
                          ধ্রুবক Accessor &A) {
  AstPtr Current_গ্রন্থি = create_গ্রন্থি(Ctx, Ty, A, Parent);
  size_t Start = CurrentPtr;
  parseMustache(Current_গ্রন্থি);
  ধ্রুবক size_t End = CurrentPtr - 1;

  size_t Raw_Body_আকার = 0;
  জন্য (size_t I = Start; I < End; ++I)
    Raw_Body_আকার += Tokens[I].RawBody.আকার_3();

  SmallString<128> RawBody;
  RawBody.reserve(Raw_Body_আকার);
  জন্য (std::size_t I = Start; I < End; ++I)
    RawBody += Tokens[I].RawBody;

  Current_গ্রন্থি->setRawBody(Ctx.Saver.save(StringRef(RawBody)));
  Parent->addChild(Current_গ্রন্থি);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr মূল_গ্রন্থি = create_মূল_গ্রন্থি(Ctx);
  parseMustache(মূল_গ্রন্থি);
  ফেরত মূল_গ্রন্থি;
}

শূন্য Parser::parseMustache(ASTNode *Parent) {

  যখন (CurrentPtr < Tokens.আকার_3()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_গ্রন্থি;

    নির্বাচন (CurrentToken.getType()) {
    ক্ষেত্র Token::Type::Text: {
      Current_গ্রন্থি = create_Text_গ্রন্থি(Ctx, CurrentToken.TokenBody, Parent);
      Parent->addChild(Current_গ্রন্থি);
      বিরতি;
    }
    ক্ষেত্র Token::Type::Variable: {
      Current_গ্রন্থি = create_গ্রন্থি(Ctx, ASTNode::Variable, A, Parent);
      Parent->addChild(Current_গ্রন্থি);
      বিরতি;
    }
    ক্ষেত্র Token::Type::UnescapeVariable: {
      Current_গ্রন্থি = create_গ্রন্থি(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->addChild(Current_গ্রন্থি);
      বিরতি;
    }
    ক্ষেত্র Token::Type::Partial: {
      Current_গ্রন্থি = create_গ্রন্থি(Ctx, ASTNode::Partial, A, Parent);
      Current_গ্রন্থি->setIndentation(CurrentToken.getIndentation());
      Parent->addChild(Current_গ্রন্থি);
      বিরতি;
    }
    ক্ষেত্র Token::Type::SectionOpen: {
      parseSection(Parent, ASTNode::Section, A);
      বিরতি;
    }
    ক্ষেত্র Token::Type::InvertSectionOpen: {
      parseSection(Parent, ASTNode::InvertSection, A);
      বিরতি;
    }
    ক্ষেত্র Token::Type::Comment:
    ক্ষেত্র Token::Type::SetDelimiter:
      বিরতি;
    ক্ষেত্র Token::Type::SectionClose:
      ফেরত;
    }
  }
}
স্থির শূন্য toMustacheString(ধ্রুবক json::মান &তথ্য_2, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << jsonKindToString(তথ্য_2.kind()) << ", Data: " << তথ্য_2
                    << "\n");
  নির্বাচন (তথ্য_2.kind()) {
  ক্ষেত্র json::মান::Null:
    ফেরত;
  ক্ষেত্র json::মান::সংখ্যা: {
    auto অঙ্ক = *তথ্য_2.get_As_সংখ্যা();
    std::ostringstream SS;
    SS << অঙ্ক;
    OS << SS.str();
    ফেরত;
  }
  ক্ষেত্র json::মান::String: {
    OS << *তথ্য_2.getAsString();
    ফেরত;
  }

  ক্ষেত্র json::মান::Array: {
    auto Arr = *তথ্য_2.getAsArray();
    যদি (Arr.empty())
      ফেরত;
    [[fallthrough]];
  }
  ক্ষেত্র json::মান::Object:
  ক্ষেত্র json::মান::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.মান_2(তথ্য_2);
    বিরতি;
  }
  }
}

শূন্য ASTNode::render_মূল(ধ্রুবক json::মান &CurrentCtx,
                         MustacheOutputStream &OS) {
  renderChild(CurrentCtx, OS);
}

শূন্য ASTNode::renderText(MustacheOutputStream &OS) { OS << Body; }

শূন্য ASTNode::renderPartial(ধ্রুবক json::মান &CurrentCtx,
                            MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_মান[0]
                    << ", Indentation:" << Indentation << "\n");
  auto Partial = Ctx.Partials.find(Accessor_মান[0]);
  যদি (Partial != Ctx.Partials.end())
    renderPartial(CurrentCtx, OS, Partial->get_মান());
}

শূন্য ASTNode::renderVariable(ধ্রুবক json::মান &CurrentCtx,
                             MustacheOutputStream &OS) {
  auto Lambda = Ctx.Lambdas.find(Accessor_মান[0]);
  যদি (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_মান());
  } অন্যথা যদি (ধ্রুবক json::মান *ContextPtr = findContext()) {
    EscapeStringStream ES(OS, Ctx.Escapes);
    toMustacheString(*ContextPtr, ES);
  }
}

শূন্য ASTNode::renderUnescapeVariable(ধ্রুবক json::মান &CurrentCtx,
                                     MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_মান[0]
                    << "\n");
  auto Lambda = Ctx.Lambdas.find(Accessor_মান[0]);
  যদি (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_মান());
  } অন্যথা যদি (ধ্রুবক json::মান *ContextPtr = findContext()) {
    OS.suspendIndentation();
    toMustacheString(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

শূন্য ASTNode::renderSection(ধ্রুবক json::মান &CurrentCtx,
                            MustacheOutputStream &OS) {
  auto SectionLambda = Ctx.SectionLambdas.find(Accessor_মান[0]);
  যদি (SectionLambda != Ctx.SectionLambdas.end()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_মান());
    ফেরত;
  }

  ধ্রুবক json::মান *ContextPtr = findContext();
  যদি (isContextFalsey(ContextPtr))
    ফেরত;

  যদি (ধ্রুবক json::Array *Arr = ContextPtr->getAsArray()) {
    জন্য (ধ্রুবক json::মান &V : *Arr)
      renderChild(V, OS);
    ফেরত;
  }
  renderChild(*ContextPtr, OS);
}

শূন্য ASTNode::renderInvertSection(ধ্রুবক json::মান &CurrentCtx,
                                  MustacheOutputStream &OS) {
  যৌক্তিক IsLambda = Ctx.SectionLambdas.contains(Accessor_মান[0]);
  ধ্রুবক json::মান *ContextPtr = findContext();
  যদি (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

শূন্য ASTNode::render(ধ্রুবক llvm::json::মান &তথ্য_2, MustacheOutputStream &OS) {
  যদি (Ty != মূল_2 && Ty != Text && Accessor_মান.empty())
    ফেরত;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &তথ্য_2;

  নির্বাচন (Ty) {
  ক্ষেত্র মূল_2:
    render_মূল(তথ্য_2, OS);
    ফেরত;
  ক্ষেত্র Text:
    renderText(OS);
    ফেরত;
  ক্ষেত্র Partial:
    renderPartial(তথ্য_2, OS);
    ফেরত;
  ক্ষেত্র Variable:
    renderVariable(তথ্য_2, OS);
    ফেরত;
  ক্ষেত্র UnescapeVariable:
    renderUnescapeVariable(তথ্য_2, OS);
    ফেরত;
  ক্ষেত্র Section:
    renderSection(তথ্য_2, OS);
    ফেরত;
  ক্ষেত্র InvertSection:
    renderInvertSection(তথ্য_2, OS);
    ফেরত;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ধ্রুবক json::মান *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  যদি (Accessor_মান.empty())
    ফেরত nullptr;
  যদি (Accessor_মান[0] == ".")
    ফেরত ParentContext;

  ধ্রুবক json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = Accessor_মান[0];
  ASTNode *CurrentParent = Parent;

  যখন (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    যদি (CurrentParent->Ty != মূল_2) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      চালু;
    }
    ফেরত nullptr;
  }
  ধ্রুবক json::মান *Context = nullptr;
  জন্য (auto [Idx, Acc] : enumerate(Accessor_মান)) {
    ধ্রুবক json::মান *Current_মান = CurrentContext->get(Acc);
    যদি (!Current_মান)
      ফেরত nullptr;
    যদি (Idx < Accessor_মান.আকার_3() - 1) {
      CurrentContext = Current_মান->getAsObject();
      যদি (!CurrentContext)
        ফেরত nullptr;
    } অন্যথা {
      Context = Current_মান;
    }
  }
  ফেরত Context;
}

শূন্য ASTNode::renderChild(ধ্রুবক json::মান &Contexts,
                          MustacheOutputStream &OS) {
  জন্য (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

শূন্য ASTNode::renderPartial(ধ্রুবক json::মান &Contexts,
                            MustacheOutputStream &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  AddIndentationStringStream IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

শূন্য ASTNode::renderLambdas(ধ্রুবক llvm::json::মান &Contexts,
                            MustacheOutputStream &OS, Lambda &L) {
  json::মান Lambda_ফলাফল = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Lambda_ফলাফল, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_গ্রন্থি = P.parse();

  EscapeStringStream ES(OS, Ctx.Escapes);
  যদি (Ty == Variable) {
    Lambda_গ্রন্থি->render(Contexts, ES);
    ফেরত;
  }
  Lambda_গ্রন্থি->render(Contexts, OS);
}

শূন্য ASTNode::renderSectionLambdas(ধ্রুবক llvm::json::মান &Contexts,
                                   MustacheOutputStream &OS, SectionLambda &L) {
  json::মান Return = L(RawBody.str());
  যদি (isFalsey(Return))
    ফেরত;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_গ্রন্থি = P.parse();
  Lambda_গ্রন্থি->render(Contexts, OS);
}

শূন্য Template::render(ধ্রুবক llvm::json::মান &তথ্য_2, llvm::raw_ostream &OS) {
  RawMustacheOutputStream MOS(OS);
  বৃক্ষ->render(তথ্য_2, MOS);
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
  ধ্রুবক EscapeMap HtmlEntities = {{'&', "&amp;"},
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

Template::~Template() = পূর্বনির্ধারিত;

} // namespace llvm::mustache

#undef DEBUG_TYPE
