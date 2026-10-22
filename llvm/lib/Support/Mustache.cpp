//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#அந்தர்பாவ "llvm/Support/Mustache.h"
#அந்தர்பாவ "llvm/ADT/SmallVector.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <sstream>

#define DEBUG_TYPE "mustache"

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::mustache;

நாமாகாஶ {

using Accessor = ArrayRef<StringRef>;

நிலையான தருக்க isFalsey(மாறிலி json::மதிப்பு &V) {
  திரும்பு V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().மதிப்பு_2()) ||
         (V.getAsArray() && V.getAsArray()->empty());
}

நிலையான தருக்க isContextFalsey(மாறிலி json::மதிப்பு *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  எனில் (!V)
    திரும்பு உண்மை;
  திரும்பு isFalsey(*V);
}

நிலையான வெறுமை splitAndTrim(StringRef Str, SmallVectorImpl<StringRef> &Tokens) {
  size_t CurrentPos = 0;
  போது (CurrentPos < Str.size()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    எனில் (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.size();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.slice(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t Start = Part.find_first_not_of(" \t\r\n");
    எனில் (Start != StringRef::npos) {
      size_t End = Part.find_last_not_of(" \t\r\n");
      Tokens.தள்ளு_back(Part.slice(Start, End + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

நிலையான Accessor splitMustacheString(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  SmallVector<StringRef> Tokens;
  எனில் (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    Tokens.தள்ளு_back(".");
  } இல்லையெனில் {
    splitAndTrim(Str, Tokens);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.size());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  திரும்பு ArrayRef<StringRef>(ArenaTokens, Tokens.size());
}
} // namespace

நாமாகாஶ llvm::mustache {

வகுப்பு MustacheOutputStream : பொது raw_ostream {
பொது:
  MustacheOutputStream() = இயல்பு;
  ~MustacheOutputStream() override = இயல்பு;

  மெய்நிகர் வெறுமை suspendIndentation() {}
  மெய்நிகர் வெறுமை resumeIndentation() {}

தனிப்பட்ட:
  வெறுமை anchor() override;
};

வெறுமை MustacheOutputStream::anchor() {}

வகுப்பு RawMustacheOutputStream : பொது MustacheOutputStream {
பொது:
  RawMustacheOutputStream(raw_ostream &OS) : OS(OS) { SetUnbuffered(); }

தனிப்பட்ட:
  raw_ostream &OS;

  வெறுமை write_impl(மாறிலி எழுத்து *Ptr, size_t Size) override {
    OS.லிக(Ptr, Size);
  }
  uint64_t current_pos() மாறிலி override { திரும்பு OS.tell(); }
};

வகுப்பு Token {
பொது:
  எண்ணிக்கை வகுப்பு Type {
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
        Accessor_மதிப்பு({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, எழுத்து Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    எனில் (TokenType == Type::Comment)
      திரும்பு;
    StringRef AccessorStr(இது->TokenBody);
    எனில் (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_மதிப்பு = splitMustacheString(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() மாறிலி { திரும்பு Accessor_மதிப்பு; }

  Type getType() மாறிலி { திரும்பு TokenType; }

  வெறுமை setIndentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() மாறிலி { திரும்பு Indentation; }

  நிலையான Type getTokenType(எழுத்து Identifier) {
    தேர்வு (Identifier) {
    நிலை '#':
      திரும்பு Type::SectionOpen;
    நிலை '/':
      திரும்பு Type::SectionClose;
    நிலை '^':
      திரும்பு Type::InvertSectionOpen;
    நிலை '!':
      திரும்பு Type::Comment;
    நிலை '>':
      திரும்பு Type::Partial;
    நிலை '&':
      திரும்பு Type::UnescapeVariable;
    நிலை '=':
      திரும்பு Type::SetDelimiter;
    இயல்பு:
      திரும்பு Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_மதிப்பு;
  size_t Indentation;
};

using EscapeMap = DenseMap<எழுத்து, std::string>;

வகுப்பு ASTNode : பொது ilist_முனை<ASTNode> {
பொது:
  எண்ணிக்கை Type {
    வேர்_2,
    Text,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::வேர்_2), Parent(nullptr), ParentContext(nullptr) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *Parent)
      : Ctx(Ctx), Ty(Type::Text), Body(Body), Parent(Parent),
        ParentContext(nullptr) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *Parent)
      : Ctx(Ctx), Ty(Ty), Parent(Parent), Accessor_மதிப்பு(Accessor),
        ParentContext(nullptr) {}

  வெறுமை addChild(AstPtr Child) { Children.தள்ளு_back(Child); };

  வெறுமை setRawBody(StringRef NewBody) { RawBody = NewBody; };

  வெறுமை setIndentation(size_t NewIndentation) { Indentation = NewIndentation; };

  வெறுமை render(மாறிலி llvm::json::மதிப்பு &Data, MustacheOutputStream &OS);

தனிப்பட்ட:
  வெறுமை renderLambdas(மாறிலி llvm::json::மதிப்பு &Contexts,
                     MustacheOutputStream &OS, Lambda &L);

  வெறுமை renderSectionLambdas(மாறிலி llvm::json::மதிப்பு &Contexts,
                            MustacheOutputStream &OS, SectionLambda &L);

  வெறுமை renderPartial(மாறிலி llvm::json::மதிப்பு &Contexts,
                     MustacheOutputStream &OS, ASTNode *Partial);

  வெறுமை renderChild(மாறிலி llvm::json::மதிப்பு &Context, MustacheOutputStream &OS);

  மாறிலி llvm::json::மதிப்பு *findContext();

  வெறுமை render_வேர்(மாறிலி json::மதிப்பு &CurrentCtx, MustacheOutputStream &OS);
  வெறுமை renderText(MustacheOutputStream &OS);
  வெறுமை renderPartial(மாறிலி json::மதிப்பு &CurrentCtx, MustacheOutputStream &OS);
  வெறுமை renderVariable(மாறிலி json::மதிப்பு &CurrentCtx, MustacheOutputStream &OS);
  வெறுமை renderUnescapeVariable(மாறிலி json::மதிப்பு &CurrentCtx,
                              MustacheOutputStream &OS);
  வெறுமை renderSection(மாறிலி json::மதிப்பு &CurrentCtx, MustacheOutputStream &OS);
  வெறுமை renderInvertSection(மாறிலி json::மதிப்பு &CurrentCtx,
                           MustacheOutputStream &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  மாறிலி ArrayRef<StringRef> Accessor_மதிப்பு;
  மாறிலி llvm::json::மதிப்பு *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
நிலையான AstPtr create_வேர்_முனை(MustacheContext &Ctx) {
  திரும்பு புதிய (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

நிலையான AstPtr create_முனை(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  திரும்பு புதிய (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

நிலையான AstPtr create_Text_முனை(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  திரும்பு புதிய (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
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
நிலையான தருக்க hasTextBehind(size_t Idx, மாறிலி ArrayRef<Token> &Tokens) {
  எனில் (Idx == 0)
    திரும்பு உண்மை;

  size_t PrevIdx = Idx - 1;
  எனில் (Tokens[PrevIdx].getType() != Token::Type::Text)
    திரும்பு உண்மை;

  மாறிலி Token &PrevToken = Tokens[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  திரும்பு !TokenBody.ends_with("\n") && !(TokenBody.empty() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
நிலையான தருக்க hasTextAhead(size_t Idx, மாறிலி ArrayRef<Token> &Tokens) {
  எனில் (Idx >= Tokens.size() - 1)
    திரும்பு உண்மை;

  size_t NextIdx = Idx + 1;
  எனில் (Tokens[NextIdx].getType() != Token::Type::Text)
    திரும்பு உண்மை;

  மாறிலி Token &NextToken = Tokens[NextIdx];
  StringRef TokenBody = StringRef(NextToken.RawBody).ltrim(" ");
  திரும்பு !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

நிலையான தருக்க requiresCleanUp(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  திரும்பு T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::SetDelimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
நிலையான வெறுமை stripTokenAhead(SmallVectorImpl<Token> &Tokens, size_t Idx) {
  Token &NextToken = Tokens[Idx + 1];
  StringRef NextTokenBody = NextToken.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  எனில் (NextTokenBody.starts_with("\r\n"))
    NextToken.TokenBody = NextTokenBody.substr(2);
  இல்லையெனில் எனில் (NextTokenBody.starts_with("\n"))
    NextToken.TokenBody = NextTokenBody.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
வெறுமை stripTokenBefore(SmallVectorImpl<Token> &Tokens, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = Tokens[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.size() - Unindented.size();
  PrevToken.TokenBody = Unindented;
  CurrentToken.setIndentation(Indentation);
}

கட்டமைப்பு Tag {
  எண்ணிக்கை வகுப்பு Kind {
    None,
    Normal, // {{...}}
    Triple, // {{{...}}}
  };

  Kind TagKind = Kind::None;
  StringRef Content;   // The content between the delimiters.
  StringRef FullMatch; // The entire tag, including delimiters.
  size_t StartPosition = StringRef::npos;
};

[[maybe_unused]] நிலையான மாறிலி எழுத்து *tagKindToString(Tag::Kind K) {
  தேர்வு (K) {
  நிலை Tag::Kind::None:
    திரும்பு "None";
  நிலை Tag::Kind::Normal:
    திரும்பு "Normal";
  நிலை Tag::Kind::Triple:
    திரும்பு "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_unused]] நிலையான மாறிலி எழுத்து *jsonKindToString(json::மதிப்பு::Kind K) {
  தேர்வு (K) {
  நிலை json::மதிப்பு::Kind::Null:
    திரும்பு "JSON_KIND_NULL";
  நிலை json::மதிப்பு::Kind::Boolean:
    திரும்பு "JSON_KIND_BOOLEAN";
  நிலை json::மதிப்பு::Kind::எண்:
    திரும்பு "JSON_KIND_NUMBER";
  நிலை json::மதிப்பு::Kind::String:
    திரும்பு "JSON_KIND_STRING";
  நிலை json::மதிப்பு::Kind::Array:
    திரும்பு "JSON_KIND_ARRAY";
  நிலை json::மதிப்பு::Kind::Object:
    திரும்பு "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
நிலையான SmallVector<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  SmallVector<Token> Tokens;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t TextStart = 0;

  மாறிலி StringLiteral TripleOpen("{{{");
  மாறிலி StringLiteral TripleClose("}}}");

  போது (Cursor < Template.size()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef TagOpen, TagClose;
    Tag::Kind Kind;

    // Determine which tag we've encountered.
    எனில் (TemplateSuffix.starts_with(TripleOpen)) {
      Kind = Tag::Kind::Triple;
      TagOpen = TripleOpen;
      TagClose = TripleClose;
    } இல்லையெனில் எனில் (TemplateSuffix.starts_with(Open)) {
      Kind = Tag::Kind::Normal;
      TagOpen = Open;
      TagClose = Close;
    } இல்லையெனில் {
      // Not at a tag, continue scanning.
      ++Cursor;
      தொடர்;
    }

    // Found a tag, first add the preceding text.
    எனில் (Cursor > TextStart)
      Tokens.emplace_back(Template.slice(TextStart, Cursor));

    // Find the closing tag.
    size_t EndPos = Template.find(TagClose, Cursor + TagOpen.size());
    எனில் (EndPos == StringRef::npos) {
      // No closing tag, the rest is text.
      Tokens.emplace_back(Template.substr(Cursor));
      TextStart = Cursor = Template.size();
      நிறுத்து;
    }

    // Extract tag content and full match.
    size_t ContentStart = Cursor + TagOpen.size();
    StringRef Content = Template.substr(ContentStart, EndPos - ContentStart);
    StringRef FullMatch =
        Template.substr(Cursor, (EndPos + TagClose.size()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << FullMatch << ", Content: " << Content
                      << ", Kind: " << tagKindToString(Kind) << "\n");
    எனில் (Kind == Tag::Kind::Triple) {
      Tokens.emplace_back(FullMatch, Ctx.Saver.save("&" + Content), '&', Ctx);
    } இல்லையெனில் { // Normal Tag
      StringRef Interpolated = Content;
      எனில் (!Interpolated.trim().starts_with("=")) {
        எழுத்து Front = Interpolated.empty() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_back(FullMatch, Interpolated, Front, Ctx);
      } இல்லையெனில் { // Set Delimiter
        Tokens.emplace_back(FullMatch, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](எழுத்து C) { திரும்பு C == '='; });
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
  எனில் (TextStart < Template.size())
    Tokens.emplace_back(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  size_t LastIdx = Tokens.size() - 1;
  க்கு (size_t Idx = 0, End = Tokens.size(); Idx < End; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    எனில் (!requiresCleanUp(CurrentType))
      தொடர்;

    தருக்க HasTextBehind = hasTextBehind(Idx, Tokens);
    தருக்க HasTextAhead = hasTextAhead(Idx, Tokens);

    எனில் ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    எனில் ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == LastIdx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  திரும்பு Tokens;
}

// Custom stream to escape strings.
வகுப்பு EscapeStringStream : பொது MustacheOutputStream {
பொது:
  explicit EscapeStringStream(llvm::raw_ostream &WrappedStream,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.சாவிகள்().begin(), Escape.சாவிகள்().end()),
        WrappedStream(WrappedStream) {
    SetUnbuffered();
  }

பாதுகாக்கப்பட்ட:
  வெறுமை write_impl(மாறிலி எழுத்து *Ptr, size_t Size) override {
    StringRef Data(Ptr, Size);
    size_t Start = 0;
    போது (Start < Size) {
      // Find the next character that needs to be escaped.
      size_t Next = Data.find_first_of(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      எனில் (Next == StringRef::npos) {
        WrappedStream << Data.substr(Start);
        திரும்பு;
      }

      // Write the chunk of text before the escapable character.
      எனில் (Next > Start)
        WrappedStream << Data.substr(Start, Next - Start);

      // Look up and write the escaped version of the character.
      WrappedStream << Escape[Data[Next]];
      Start = Next + 1;
    }
  }

  uint64_t current_pos() மாறிலி override { திரும்பு WrappedStream.tell(); }

தனிப்பட்ட:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &WrappedStream;
};

// Custom stream to add indentation used to for rendering partials.
வகுப்பு AddIndentationStringStream : பொது MustacheOutputStream {
பொது:
  explicit AddIndentationStringStream(raw_ostream &WrappedStream,
                                      size_t Indentation)
      : Indentation(Indentation), WrappedStream(WrappedStream),
        NeedsIndent(உண்மை), IsSuspended(பொய்) {
    SetUnbuffered();
  }

  வெறுமை suspendIndentation() override { IsSuspended = உண்மை; }
  வெறுமை resumeIndentation() override { IsSuspended = பொய்; }

பாதுகாக்கப்பட்ட:
  வெறுமை write_impl(மாறிலி எழுத்து *Ptr, size_t Size) override {
    llvm::StringRef Data(Ptr, Size);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    க்கு (எழுத்து C : Data) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      எனில் (NeedsIndent && C != '\n') {
        WrappedStream << Indent;
        NeedsIndent = பொய்;
      }
      WrappedStream << C;
      எனில் (C == '\n' && !IsSuspended)
        NeedsIndent = உண்மை;
    }
  }

  uint64_t current_pos() மாறிலி override { திரும்பு WrappedStream.tell(); }

தனிப்பட்ட:
  size_t Indentation;
  raw_ostream &WrappedStream;
  தருக்க NeedsIndent;
  தருக்க IsSuspended;
};

வகுப்பு Parser {
பொது:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

தனிப்பட்ட:
  வெறுமை parseMustache(ASTNode *Parent);
  வெறுமை parseSection(ASTNode *Parent, ASTNode::Type Ty, மாறிலி Accessor &A);

  MustacheContext &Ctx;
  SmallVector<Token> Tokens;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

வெறுமை Parser::parseSection(ASTNode *Parent, ASTNode::Type Ty,
                          மாறிலி Accessor &A) {
  AstPtr Current_முனை = create_முனை(Ctx, Ty, A, Parent);
  size_t Start = CurrentPtr;
  parseMustache(Current_முனை);
  மாறிலி size_t End = CurrentPtr - 1;

  size_t RawBodySize = 0;
  க்கு (size_t I = Start; I < End; ++I)
    RawBodySize += Tokens[I].RawBody.size();

  SmallString<128> RawBody;
  RawBody.reserve(RawBodySize);
  க்கு (std::size_t I = Start; I < End; ++I)
    RawBody += Tokens[I].RawBody;

  Current_முனை->setRawBody(Ctx.Saver.save(StringRef(RawBody)));
  Parent->addChild(Current_முனை);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr வேர்_முனை = create_வேர்_முனை(Ctx);
  parseMustache(வேர்_முனை);
  திரும்பு வேர்_முனை;
}

வெறுமை Parser::parseMustache(ASTNode *Parent) {

  போது (CurrentPtr < Tokens.size()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_முனை;

    தேர்வு (CurrentToken.getType()) {
    நிலை Token::Type::Text: {
      Current_முனை = create_Text_முனை(Ctx, CurrentToken.TokenBody, Parent);
      Parent->addChild(Current_முனை);
      நிறுத்து;
    }
    நிலை Token::Type::Variable: {
      Current_முனை = create_முனை(Ctx, ASTNode::Variable, A, Parent);
      Parent->addChild(Current_முனை);
      நிறுத்து;
    }
    நிலை Token::Type::UnescapeVariable: {
      Current_முனை = create_முனை(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->addChild(Current_முனை);
      நிறுத்து;
    }
    நிலை Token::Type::Partial: {
      Current_முனை = create_முனை(Ctx, ASTNode::Partial, A, Parent);
      Current_முனை->setIndentation(CurrentToken.getIndentation());
      Parent->addChild(Current_முனை);
      நிறுத்து;
    }
    நிலை Token::Type::SectionOpen: {
      parseSection(Parent, ASTNode::Section, A);
      நிறுத்து;
    }
    நிலை Token::Type::InvertSectionOpen: {
      parseSection(Parent, ASTNode::InvertSection, A);
      நிறுத்து;
    }
    நிலை Token::Type::Comment:
    நிலை Token::Type::SetDelimiter:
      நிறுத்து;
    நிலை Token::Type::SectionClose:
      திரும்பு;
    }
  }
}
நிலையான வெறுமை toMustacheString(மாறிலி json::மதிப்பு &Data, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << jsonKindToString(Data.kind()) << ", Data: " << Data
                    << "\n");
  தேர்வு (Data.kind()) {
  நிலை json::மதிப்பு::Null:
    திரும்பு;
  நிலை json::மதிப்பு::எண்: {
    auto இலக்கம் = *Data.get_As_எண்();
    std::ostringstream SS;
    SS << இலக்கம்;
    OS << SS.str();
    திரும்பு;
  }
  நிலை json::மதிப்பு::String: {
    OS << *Data.getAsString();
    திரும்பு;
  }

  நிலை json::மதிப்பு::Array: {
    auto Arr = *Data.getAsArray();
    எனில் (Arr.empty())
      திரும்பு;
    [[fallthrough]];
  }
  நிலை json::மதிப்பு::Object:
  நிலை json::மதிப்பு::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.மதிப்பு_2(Data);
    நிறுத்து;
  }
  }
}

வெறுமை ASTNode::render_வேர்(மாறிலி json::மதிப்பு &CurrentCtx,
                         MustacheOutputStream &OS) {
  renderChild(CurrentCtx, OS);
}

வெறுமை ASTNode::renderText(MustacheOutputStream &OS) { OS << Body; }

வெறுமை ASTNode::renderPartial(மாறிலி json::மதிப்பு &CurrentCtx,
                            MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_மதிப்பு[0]
                    << ", Indentation:" << Indentation << "\n");
  auto Partial = Ctx.Partials.find(Accessor_மதிப்பு[0]);
  எனில் (Partial != Ctx.Partials.end())
    renderPartial(CurrentCtx, OS, Partial->get_மதிப்பு());
}

வெறுமை ASTNode::renderVariable(மாறிலி json::மதிப்பு &CurrentCtx,
                             MustacheOutputStream &OS) {
  auto Lambda = Ctx.Lambdas.find(Accessor_மதிப்பு[0]);
  எனில் (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_மதிப்பு());
  } இல்லையெனில் எனில் (மாறிலி json::மதிப்பு *ContextPtr = findContext()) {
    EscapeStringStream ES(OS, Ctx.Escapes);
    toMustacheString(*ContextPtr, ES);
  }
}

வெறுமை ASTNode::renderUnescapeVariable(மாறிலி json::மதிப்பு &CurrentCtx,
                                     MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_மதிப்பு[0]
                    << "\n");
  auto Lambda = Ctx.Lambdas.find(Accessor_மதிப்பு[0]);
  எனில் (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_மதிப்பு());
  } இல்லையெனில் எனில் (மாறிலி json::மதிப்பு *ContextPtr = findContext()) {
    OS.suspendIndentation();
    toMustacheString(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

வெறுமை ASTNode::renderSection(மாறிலி json::மதிப்பு &CurrentCtx,
                            MustacheOutputStream &OS) {
  auto SectionLambda = Ctx.SectionLambdas.find(Accessor_மதிப்பு[0]);
  எனில் (SectionLambda != Ctx.SectionLambdas.end()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_மதிப்பு());
    திரும்பு;
  }

  மாறிலி json::மதிப்பு *ContextPtr = findContext();
  எனில் (isContextFalsey(ContextPtr))
    திரும்பு;

  எனில் (மாறிலி json::Array *Arr = ContextPtr->getAsArray()) {
    க்கு (மாறிலி json::மதிப்பு &V : *Arr)
      renderChild(V, OS);
    திரும்பு;
  }
  renderChild(*ContextPtr, OS);
}

வெறுமை ASTNode::renderInvertSection(மாறிலி json::மதிப்பு &CurrentCtx,
                                  MustacheOutputStream &OS) {
  தருக்க IsLambda = Ctx.SectionLambdas.contains(Accessor_மதிப்பு[0]);
  மாறிலி json::மதிப்பு *ContextPtr = findContext();
  எனில் (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

வெறுமை ASTNode::render(மாறிலி llvm::json::மதிப்பு &Data, MustacheOutputStream &OS) {
  எனில் (Ty != வேர்_2 && Ty != Text && Accessor_மதிப்பு.empty())
    திரும்பு;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &Data;

  தேர்வு (Ty) {
  நிலை வேர்_2:
    render_வேர்(Data, OS);
    திரும்பு;
  நிலை Text:
    renderText(OS);
    திரும்பு;
  நிலை Partial:
    renderPartial(Data, OS);
    திரும்பு;
  நிலை Variable:
    renderVariable(Data, OS);
    திரும்பு;
  நிலை UnescapeVariable:
    renderUnescapeVariable(Data, OS);
    திரும்பு;
  நிலை Section:
    renderSection(Data, OS);
    திரும்பு;
  நிலை InvertSection:
    renderInvertSection(Data, OS);
    திரும்பு;
  }
  llvm_unreachable("Invalid ASTNode type");
}

மாறிலி json::மதிப்பு *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  எனில் (Accessor_மதிப்பு.empty())
    திரும்பு nullptr;
  எனில் (Accessor_மதிப்பு[0] == ".")
    திரும்பு ParentContext;

  மாறிலி json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = Accessor_மதிப்பு[0];
  ASTNode *CurrentParent = Parent;

  போது (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    எனில் (CurrentParent->Ty != வேர்_2) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      தொடர்;
    }
    திரும்பு nullptr;
  }
  மாறிலி json::மதிப்பு *Context = nullptr;
  க்கு (auto [Idx, Acc] : enumerate(Accessor_மதிப்பு)) {
    மாறிலி json::மதிப்பு *Current_மதிப்பு = CurrentContext->get(Acc);
    எனில் (!Current_மதிப்பு)
      திரும்பு nullptr;
    எனில் (Idx < Accessor_மதிப்பு.size() - 1) {
      CurrentContext = Current_மதிப்பு->getAsObject();
      எனில் (!CurrentContext)
        திரும்பு nullptr;
    } இல்லையெனில் {
      Context = Current_மதிப்பு;
    }
  }
  திரும்பு Context;
}

வெறுமை ASTNode::renderChild(மாறிலி json::மதிப்பு &Contexts,
                          MustacheOutputStream &OS) {
  க்கு (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

வெறுமை ASTNode::renderPartial(மாறிலி json::மதிப்பு &Contexts,
                            MustacheOutputStream &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  AddIndentationStringStream IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

வெறுமை ASTNode::renderLambdas(மாறிலி llvm::json::மதிப்பு &Contexts,
                            MustacheOutputStream &OS, Lambda &L) {
  json::மதிப்பு LambdaResult = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(LambdaResult, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_முனை = P.parse();

  EscapeStringStream ES(OS, Ctx.Escapes);
  எனில் (Ty == Variable) {
    Lambda_முனை->render(Contexts, ES);
    திரும்பு;
  }
  Lambda_முனை->render(Contexts, OS);
}

வெறுமை ASTNode::renderSectionLambdas(மாறிலி llvm::json::மதிப்பு &Contexts,
                                   MustacheOutputStream &OS, SectionLambda &L) {
  json::மதிப்பு Return = L(RawBody.str());
  எனில் (isFalsey(Return))
    திரும்பு;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_முனை = P.parse();
  Lambda_முனை->render(Contexts, OS);
}

வெறுமை Template::render(மாறிலி llvm::json::மதிப்பு &Data, llvm::raw_ostream &OS) {
  RawMustacheOutputStream MOS(OS);
  மரம்->render(Data, MOS);
}

வெறுமை Template::registerPartial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_மரம் = P.parse();
  Ctx.Partials.செருகு(std::make_pair(Name, Partial_மரம்));
}

வெறுமை Template::registerLambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

வெறுமை Template::registerLambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

வெறுமை Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  மரம் = P.parse();
  // The default behavior is to escape html entities.
  மாறிலி EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), மரம்(Other.மரம்) {
  Other.மரம் = nullptr;
}

Template::~Template() = இயல்பு;

} // namespace llvm::mustache

#undef DEBUG_TYPE
