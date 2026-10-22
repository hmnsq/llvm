//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#અન્તર્ભાવ "llvm/Support/Mustache.h"
#અન્તર્ભાવ "llvm/ADT/SmallVector.h"
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <sstream>

#define DEBUG_TYPE "mustache"

using નામાકાશ llvm;
using નામાકાશ llvm::mustache;

નામાકાશ {

using Accessor = ArrayRef<StringRef>;

સ્થિર તાર્કિક isFalsey(અચલ json::કિંમત &V) {
  પાછા V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().કિંમત_2()) ||
         (V.getAsArray() && V.getAsArray()->empty());
}

સ્થિર તાર્કિક isContextFalsey(અચલ json::કિંમત *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  જો (!V)
    પાછા સાચું;
  પાછા isFalsey(*V);
}

સ્થિર શૂન્ય splitAndTrim(StringRef Str, SmallVectorImpl<StringRef> &Tokens) {
  size_t CurrentPos = 0;
  જ્યારે (CurrentPos < Str.size()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    જો (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.size();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.slice(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t Start = Part.find_first_not_of(" \t\r\n");
    જો (Start != StringRef::npos) {
      size_t End = Part.find_last_not_of(" \t\r\n");
      Tokens.ધકેલો_back(Part.slice(Start, End + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

સ્થિર Accessor splitMustacheString(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  SmallVector<StringRef> Tokens;
  જો (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    Tokens.ધકેલો_back(".");
  } નહીંતર {
    splitAndTrim(Str, Tokens);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.size());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  પાછા ArrayRef<StringRef>(ArenaTokens, Tokens.size());
}
} // namespace

નામાકાશ llvm::mustache {

વર્ગ MustacheOutputStream : જાહેર raw_ostream {
જાહેર:
  MustacheOutputStream() = મૂળભૂત;
  ~MustacheOutputStream() override = મૂળભૂત;

  આભાસી શૂન્ય suspendIndentation() {}
  આભાસી શૂન્ય resumeIndentation() {}

ખાનગી:
  શૂન્ય anchor() override;
};

શૂન્ય MustacheOutputStream::anchor() {}

વર્ગ RawMustacheOutputStream : જાહેર MustacheOutputStream {
જાહેર:
  RawMustacheOutputStream(raw_ostream &OS) : OS(OS) { SetUnbuffered(); }

ખાનગી:
  raw_ostream &OS;

  શૂન્ય write_impl(અચલ અક્ષર *Ptr, size_t Size) override {
    OS.લિખ(Ptr, Size);
  }
  uint64_t current_pos() અચલ override { પાછા OS.tell(); }
};

વર્ગ Token {
જાહેર:
  ગણના વર્ગ Type {
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
        Accessor_કિંમત({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, અક્ષર Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    જો (TokenType == Type::Comment)
      પાછા;
    StringRef AccessorStr(આ->TokenBody);
    જો (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_કિંમત = splitMustacheString(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() અચલ { પાછા Accessor_કિંમત; }

  Type getType() અચલ { પાછા TokenType; }

  શૂન્ય setIndentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() અચલ { પાછા Indentation; }

  સ્થિર Type getTokenType(અક્ષર Identifier) {
    પસંદગી (Identifier) {
    સ્થિતિ '#':
      પાછા Type::SectionOpen;
    સ્થિતિ '/':
      પાછા Type::SectionClose;
    સ્થિતિ '^':
      પાછા Type::InvertSectionOpen;
    સ્થિતિ '!':
      પાછા Type::Comment;
    સ્થિતિ '>':
      પાછા Type::Partial;
    સ્થિતિ '&':
      પાછા Type::UnescapeVariable;
    સ્થિતિ '=':
      પાછા Type::SetDelimiter;
    મૂળભૂત:
      પાછા Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_કિંમત;
  size_t Indentation;
};

using EscapeMap = DenseMap<અક્ષર, std::string>;

વર્ગ ASTNode : જાહેર ilist_ગાંઠ<ASTNode> {
જાહેર:
  ગણના Type {
    મૂળ_2,
    Text,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::મૂળ_2), Parent(nullptr), ParentContext(nullptr) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *Parent)
      : Ctx(Ctx), Ty(Type::Text), Body(Body), Parent(Parent),
        ParentContext(nullptr) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *Parent)
      : Ctx(Ctx), Ty(Ty), Parent(Parent), Accessor_કિંમત(Accessor),
        ParentContext(nullptr) {}

  શૂન્ય addChild(AstPtr Child) { Children.ધકેલો_back(Child); };

  શૂન્ય setRawBody(StringRef NewBody) { RawBody = NewBody; };

  શૂન્ય setIndentation(size_t NewIndentation) { Indentation = NewIndentation; };

  શૂન્ય render(અચલ llvm::json::કિંમત &Data, MustacheOutputStream &OS);

ખાનગી:
  શૂન્ય renderLambdas(અચલ llvm::json::કિંમત &Contexts,
                     MustacheOutputStream &OS, Lambda &L);

  શૂન્ય renderSectionLambdas(અચલ llvm::json::કિંમત &Contexts,
                            MustacheOutputStream &OS, SectionLambda &L);

  શૂન્ય renderPartial(અચલ llvm::json::કિંમત &Contexts,
                     MustacheOutputStream &OS, ASTNode *Partial);

  શૂન્ય renderChild(અચલ llvm::json::કિંમત &Context, MustacheOutputStream &OS);

  અચલ llvm::json::કિંમત *findContext();

  શૂન્ય render_મૂળ(અચલ json::કિંમત &CurrentCtx, MustacheOutputStream &OS);
  શૂન્ય renderText(MustacheOutputStream &OS);
  શૂન્ય renderPartial(અચલ json::કિંમત &CurrentCtx, MustacheOutputStream &OS);
  શૂન્ય renderVariable(અચલ json::કિંમત &CurrentCtx, MustacheOutputStream &OS);
  શૂન્ય renderUnescapeVariable(અચલ json::કિંમત &CurrentCtx,
                              MustacheOutputStream &OS);
  શૂન્ય renderSection(અચલ json::કિંમત &CurrentCtx, MustacheOutputStream &OS);
  શૂન્ય renderInvertSection(અચલ json::કિંમત &CurrentCtx,
                           MustacheOutputStream &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  અચલ ArrayRef<StringRef> Accessor_કિંમત;
  અચલ llvm::json::કિંમત *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
સ્થિર AstPtr create_મૂળ_ગાંઠ(MustacheContext &Ctx) {
  પાછા નવું (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

સ્થિર AstPtr create_ગાંઠ(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  પાછા નવું (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

સ્થિર AstPtr create_Text_ગાંઠ(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  પાછા નવું (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
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
સ્થિર તાર્કિક hasTextBehind(size_t Idx, અચલ ArrayRef<Token> &Tokens) {
  જો (Idx == 0)
    પાછા સાચું;

  size_t PrevIdx = Idx - 1;
  જો (Tokens[PrevIdx].getType() != Token::Type::Text)
    પાછા સાચું;

  અચલ Token &PrevToken = Tokens[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  પાછા !TokenBody.ends_with("\n") && !(TokenBody.empty() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
સ્થિર તાર્કિક hasTextAhead(size_t Idx, અચલ ArrayRef<Token> &Tokens) {
  જો (Idx >= Tokens.size() - 1)
    પાછા સાચું;

  size_t NextIdx = Idx + 1;
  જો (Tokens[NextIdx].getType() != Token::Type::Text)
    પાછા સાચું;

  અચલ Token &NextToken = Tokens[NextIdx];
  StringRef TokenBody = StringRef(NextToken.RawBody).ltrim(" ");
  પાછા !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

સ્થિર તાર્કિક requiresCleanUp(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  પાછા T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::SetDelimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
સ્થિર શૂન્ય stripTokenAhead(SmallVectorImpl<Token> &Tokens, size_t Idx) {
  Token &NextToken = Tokens[Idx + 1];
  StringRef NextTokenBody = NextToken.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  જો (NextTokenBody.starts_with("\r\n"))
    NextToken.TokenBody = NextTokenBody.substr(2);
  નહીંતર જો (NextTokenBody.starts_with("\n"))
    NextToken.TokenBody = NextTokenBody.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
શૂન્ય stripTokenBefore(SmallVectorImpl<Token> &Tokens, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = Tokens[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.size() - Unindented.size();
  PrevToken.TokenBody = Unindented;
  CurrentToken.setIndentation(Indentation);
}

સંરચના Tag {
  ગણના વર્ગ Kind {
    None,
    Normal, // {{...}}
    Triple, // {{{...}}}
  };

  Kind TagKind = Kind::None;
  StringRef Content;   // The content between the delimiters.
  StringRef FullMatch; // The entire tag, including delimiters.
  size_t StartPosition = StringRef::npos;
};

[[maybe_unused]] સ્થિર અચલ અક્ષર *tagKindToString(Tag::Kind K) {
  પસંદગી (K) {
  સ્થિતિ Tag::Kind::None:
    પાછા "None";
  સ્થિતિ Tag::Kind::Normal:
    પાછા "Normal";
  સ્થિતિ Tag::Kind::Triple:
    પાછા "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_unused]] સ્થિર અચલ અક્ષર *jsonKindToString(json::કિંમત::Kind K) {
  પસંદગી (K) {
  સ્થિતિ json::કિંમત::Kind::Null:
    પાછા "JSON_KIND_NULL";
  સ્થિતિ json::કિંમત::Kind::Boolean:
    પાછા "JSON_KIND_BOOLEAN";
  સ્થિતિ json::કિંમત::Kind::સંખ્યા:
    પાછા "JSON_KIND_NUMBER";
  સ્થિતિ json::કિંમત::Kind::String:
    પાછા "JSON_KIND_STRING";
  સ્થિતિ json::કિંમત::Kind::Array:
    પાછા "JSON_KIND_ARRAY";
  સ્થિતિ json::કિંમત::Kind::Object:
    પાછા "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
સ્થિર SmallVector<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  SmallVector<Token> Tokens;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t TextStart = 0;

  અચલ StringLiteral TripleOpen("{{{");
  અચલ StringLiteral TripleClose("}}}");

  જ્યારે (Cursor < Template.size()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef TagOpen, TagClose;
    Tag::Kind Kind;

    // Determine which tag we've encountered.
    જો (TemplateSuffix.starts_with(TripleOpen)) {
      Kind = Tag::Kind::Triple;
      TagOpen = TripleOpen;
      TagClose = TripleClose;
    } નહીંતર જો (TemplateSuffix.starts_with(Open)) {
      Kind = Tag::Kind::Normal;
      TagOpen = Open;
      TagClose = Close;
    } નહીંતર {
      // Not at a tag, continue scanning.
      ++Cursor;
      ચાલુ;
    }

    // Found a tag, first add the preceding text.
    જો (Cursor > TextStart)
      Tokens.emplace_back(Template.slice(TextStart, Cursor));

    // Find the closing tag.
    size_t EndPos = Template.find(TagClose, Cursor + TagOpen.size());
    જો (EndPos == StringRef::npos) {
      // No closing tag, the rest is text.
      Tokens.emplace_back(Template.substr(Cursor));
      TextStart = Cursor = Template.size();
      તોડો;
    }

    // Extract tag content and full match.
    size_t ContentStart = Cursor + TagOpen.size();
    StringRef Content = Template.substr(ContentStart, EndPos - ContentStart);
    StringRef FullMatch =
        Template.substr(Cursor, (EndPos + TagClose.size()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << FullMatch << ", Content: " << Content
                      << ", Kind: " << tagKindToString(Kind) << "\n");
    જો (Kind == Tag::Kind::Triple) {
      Tokens.emplace_back(FullMatch, Ctx.Saver.save("&" + Content), '&', Ctx);
    } નહીંતર { // Normal Tag
      StringRef Interpolated = Content;
      જો (!Interpolated.trim().starts_with("=")) {
        અક્ષર Front = Interpolated.empty() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_back(FullMatch, Interpolated, Front, Ctx);
      } નહીંતર { // Set Delimiter
        Tokens.emplace_back(FullMatch, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](અક્ષર C) { પાછા C == '='; });
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
  જો (TextStart < Template.size())
    Tokens.emplace_back(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  size_t LastIdx = Tokens.size() - 1;
  માટે (size_t Idx = 0, End = Tokens.size(); Idx < End; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    જો (!requiresCleanUp(CurrentType))
      ચાલુ;

    તાર્કિક HasTextBehind = hasTextBehind(Idx, Tokens);
    તાર્કિક HasTextAhead = hasTextAhead(Idx, Tokens);

    જો ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    જો ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == LastIdx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  પાછા Tokens;
}

// Custom stream to escape strings.
વર્ગ EscapeStringStream : જાહેર MustacheOutputStream {
જાહેર:
  explicit EscapeStringStream(llvm::raw_ostream &WrappedStream,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.ચાવીઓ().begin(), Escape.ચાવીઓ().end()),
        WrappedStream(WrappedStream) {
    SetUnbuffered();
  }

સુરક્ષિત:
  શૂન્ય write_impl(અચલ અક્ષર *Ptr, size_t Size) override {
    StringRef Data(Ptr, Size);
    size_t Start = 0;
    જ્યારે (Start < Size) {
      // Find the next character that needs to be escaped.
      size_t Next = Data.find_first_of(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      જો (Next == StringRef::npos) {
        WrappedStream << Data.substr(Start);
        પાછા;
      }

      // Write the chunk of text before the escapable character.
      જો (Next > Start)
        WrappedStream << Data.substr(Start, Next - Start);

      // Look up and write the escaped version of the character.
      WrappedStream << Escape[Data[Next]];
      Start = Next + 1;
    }
  }

  uint64_t current_pos() અચલ override { પાછા WrappedStream.tell(); }

ખાનગી:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &WrappedStream;
};

// Custom stream to add indentation used to for rendering partials.
વર્ગ AddIndentationStringStream : જાહેર MustacheOutputStream {
જાહેર:
  explicit AddIndentationStringStream(raw_ostream &WrappedStream,
                                      size_t Indentation)
      : Indentation(Indentation), WrappedStream(WrappedStream),
        NeedsIndent(સાચું), IsSuspended(ખોટું) {
    SetUnbuffered();
  }

  શૂન્ય suspendIndentation() override { IsSuspended = સાચું; }
  શૂન્ય resumeIndentation() override { IsSuspended = ખોટું; }

સુરક્ષિત:
  શૂન્ય write_impl(અચલ અક્ષર *Ptr, size_t Size) override {
    llvm::StringRef Data(Ptr, Size);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    માટે (અક્ષર C : Data) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      જો (NeedsIndent && C != '\n') {
        WrappedStream << Indent;
        NeedsIndent = ખોટું;
      }
      WrappedStream << C;
      જો (C == '\n' && !IsSuspended)
        NeedsIndent = સાચું;
    }
  }

  uint64_t current_pos() અચલ override { પાછા WrappedStream.tell(); }

ખાનગી:
  size_t Indentation;
  raw_ostream &WrappedStream;
  તાર્કિક NeedsIndent;
  તાર્કિક IsSuspended;
};

વર્ગ Parser {
જાહેર:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

ખાનગી:
  શૂન્ય parseMustache(ASTNode *Parent);
  શૂન્ય parseSection(ASTNode *Parent, ASTNode::Type Ty, અચલ Accessor &A);

  MustacheContext &Ctx;
  SmallVector<Token> Tokens;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

શૂન્ય Parser::parseSection(ASTNode *Parent, ASTNode::Type Ty,
                          અચલ Accessor &A) {
  AstPtr Current_ગાંઠ = create_ગાંઠ(Ctx, Ty, A, Parent);
  size_t Start = CurrentPtr;
  parseMustache(Current_ગાંઠ);
  અચલ size_t End = CurrentPtr - 1;

  size_t RawBodySize = 0;
  માટે (size_t I = Start; I < End; ++I)
    RawBodySize += Tokens[I].RawBody.size();

  SmallString<128> RawBody;
  RawBody.reserve(RawBodySize);
  માટે (std::size_t I = Start; I < End; ++I)
    RawBody += Tokens[I].RawBody;

  Current_ગાંઠ->setRawBody(Ctx.Saver.save(StringRef(RawBody)));
  Parent->addChild(Current_ગાંઠ);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr મૂળ_ગાંઠ = create_મૂળ_ગાંઠ(Ctx);
  parseMustache(મૂળ_ગાંઠ);
  પાછા મૂળ_ગાંઠ;
}

શૂન્ય Parser::parseMustache(ASTNode *Parent) {

  જ્યારે (CurrentPtr < Tokens.size()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_ગાંઠ;

    પસંદગી (CurrentToken.getType()) {
    સ્થિતિ Token::Type::Text: {
      Current_ગાંઠ = create_Text_ગાંઠ(Ctx, CurrentToken.TokenBody, Parent);
      Parent->addChild(Current_ગાંઠ);
      તોડો;
    }
    સ્થિતિ Token::Type::Variable: {
      Current_ગાંઠ = create_ગાંઠ(Ctx, ASTNode::Variable, A, Parent);
      Parent->addChild(Current_ગાંઠ);
      તોડો;
    }
    સ્થિતિ Token::Type::UnescapeVariable: {
      Current_ગાંઠ = create_ગાંઠ(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->addChild(Current_ગાંઠ);
      તોડો;
    }
    સ્થિતિ Token::Type::Partial: {
      Current_ગાંઠ = create_ગાંઠ(Ctx, ASTNode::Partial, A, Parent);
      Current_ગાંઠ->setIndentation(CurrentToken.getIndentation());
      Parent->addChild(Current_ગાંઠ);
      તોડો;
    }
    સ્થિતિ Token::Type::SectionOpen: {
      parseSection(Parent, ASTNode::Section, A);
      તોડો;
    }
    સ્થિતિ Token::Type::InvertSectionOpen: {
      parseSection(Parent, ASTNode::InvertSection, A);
      તોડો;
    }
    સ્થિતિ Token::Type::Comment:
    સ્થિતિ Token::Type::SetDelimiter:
      તોડો;
    સ્થિતિ Token::Type::SectionClose:
      પાછા;
    }
  }
}
સ્થિર શૂન્ય toMustacheString(અચલ json::કિંમત &Data, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << jsonKindToString(Data.kind()) << ", Data: " << Data
                    << "\n");
  પસંદગી (Data.kind()) {
  સ્થિતિ json::કિંમત::Null:
    પાછા;
  સ્થિતિ json::કિંમત::સંખ્યા: {
    auto અંક = *Data.get_As_સંખ્યા();
    std::ostringstream SS;
    SS << અંક;
    OS << SS.str();
    પાછા;
  }
  સ્થિતિ json::કિંમત::String: {
    OS << *Data.getAsString();
    પાછા;
  }

  સ્થિતિ json::કિંમત::Array: {
    auto Arr = *Data.getAsArray();
    જો (Arr.empty())
      પાછા;
    [[fallthrough]];
  }
  સ્થિતિ json::કિંમત::Object:
  સ્થિતિ json::કિંમત::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.કિંમત_2(Data);
    તોડો;
  }
  }
}

શૂન્ય ASTNode::render_મૂળ(અચલ json::કિંમત &CurrentCtx,
                         MustacheOutputStream &OS) {
  renderChild(CurrentCtx, OS);
}

શૂન્ય ASTNode::renderText(MustacheOutputStream &OS) { OS << Body; }

શૂન્ય ASTNode::renderPartial(અચલ json::કિંમત &CurrentCtx,
                            MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_કિંમત[0]
                    << ", Indentation:" << Indentation << "\n");
  auto Partial = Ctx.Partials.find(Accessor_કિંમત[0]);
  જો (Partial != Ctx.Partials.end())
    renderPartial(CurrentCtx, OS, Partial->get_કિંમત());
}

શૂન્ય ASTNode::renderVariable(અચલ json::કિંમત &CurrentCtx,
                             MustacheOutputStream &OS) {
  auto Lambda = Ctx.Lambdas.find(Accessor_કિંમત[0]);
  જો (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_કિંમત());
  } નહીંતર જો (અચલ json::કિંમત *ContextPtr = findContext()) {
    EscapeStringStream ES(OS, Ctx.Escapes);
    toMustacheString(*ContextPtr, ES);
  }
}

શૂન્ય ASTNode::renderUnescapeVariable(અચલ json::કિંમત &CurrentCtx,
                                     MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_કિંમત[0]
                    << "\n");
  auto Lambda = Ctx.Lambdas.find(Accessor_કિંમત[0]);
  જો (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_કિંમત());
  } નહીંતર જો (અચલ json::કિંમત *ContextPtr = findContext()) {
    OS.suspendIndentation();
    toMustacheString(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

શૂન્ય ASTNode::renderSection(અચલ json::કિંમત &CurrentCtx,
                            MustacheOutputStream &OS) {
  auto SectionLambda = Ctx.SectionLambdas.find(Accessor_કિંમત[0]);
  જો (SectionLambda != Ctx.SectionLambdas.end()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_કિંમત());
    પાછા;
  }

  અચલ json::કિંમત *ContextPtr = findContext();
  જો (isContextFalsey(ContextPtr))
    પાછા;

  જો (અચલ json::Array *Arr = ContextPtr->getAsArray()) {
    માટે (અચલ json::કિંમત &V : *Arr)
      renderChild(V, OS);
    પાછા;
  }
  renderChild(*ContextPtr, OS);
}

શૂન્ય ASTNode::renderInvertSection(અચલ json::કિંમત &CurrentCtx,
                                  MustacheOutputStream &OS) {
  તાર્કિક IsLambda = Ctx.SectionLambdas.contains(Accessor_કિંમત[0]);
  અચલ json::કિંમત *ContextPtr = findContext();
  જો (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

શૂન્ય ASTNode::render(અચલ llvm::json::કિંમત &Data, MustacheOutputStream &OS) {
  જો (Ty != મૂળ_2 && Ty != Text && Accessor_કિંમત.empty())
    પાછા;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &Data;

  પસંદગી (Ty) {
  સ્થિતિ મૂળ_2:
    render_મૂળ(Data, OS);
    પાછા;
  સ્થિતિ Text:
    renderText(OS);
    પાછા;
  સ્થિતિ Partial:
    renderPartial(Data, OS);
    પાછા;
  સ્થિતિ Variable:
    renderVariable(Data, OS);
    પાછા;
  સ્થિતિ UnescapeVariable:
    renderUnescapeVariable(Data, OS);
    પાછા;
  સ્થિતિ Section:
    renderSection(Data, OS);
    પાછા;
  સ્થિતિ InvertSection:
    renderInvertSection(Data, OS);
    પાછા;
  }
  llvm_unreachable("Invalid ASTNode type");
}

અચલ json::કિંમત *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  જો (Accessor_કિંમત.empty())
    પાછા nullptr;
  જો (Accessor_કિંમત[0] == ".")
    પાછા ParentContext;

  અચલ json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = Accessor_કિંમત[0];
  ASTNode *CurrentParent = Parent;

  જ્યારે (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    જો (CurrentParent->Ty != મૂળ_2) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      ચાલુ;
    }
    પાછા nullptr;
  }
  અચલ json::કિંમત *Context = nullptr;
  માટે (auto [Idx, Acc] : enumerate(Accessor_કિંમત)) {
    અચલ json::કિંમત *Current_કિંમત = CurrentContext->get(Acc);
    જો (!Current_કિંમત)
      પાછા nullptr;
    જો (Idx < Accessor_કિંમત.size() - 1) {
      CurrentContext = Current_કિંમત->getAsObject();
      જો (!CurrentContext)
        પાછા nullptr;
    } નહીંતર {
      Context = Current_કિંમત;
    }
  }
  પાછા Context;
}

શૂન્ય ASTNode::renderChild(અચલ json::કિંમત &Contexts,
                          MustacheOutputStream &OS) {
  માટે (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

શૂન્ય ASTNode::renderPartial(અચલ json::કિંમત &Contexts,
                            MustacheOutputStream &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  AddIndentationStringStream IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

શૂન્ય ASTNode::renderLambdas(અચલ llvm::json::કિંમત &Contexts,
                            MustacheOutputStream &OS, Lambda &L) {
  json::કિંમત LambdaResult = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(LambdaResult, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ગાંઠ = P.parse();

  EscapeStringStream ES(OS, Ctx.Escapes);
  જો (Ty == Variable) {
    Lambda_ગાંઠ->render(Contexts, ES);
    પાછા;
  }
  Lambda_ગાંઠ->render(Contexts, OS);
}

શૂન્ય ASTNode::renderSectionLambdas(અચલ llvm::json::કિંમત &Contexts,
                                   MustacheOutputStream &OS, SectionLambda &L) {
  json::કિંમત Return = L(RawBody.str());
  જો (isFalsey(Return))
    પાછા;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ગાંઠ = P.parse();
  Lambda_ગાંઠ->render(Contexts, OS);
}

શૂન્ય Template::render(અચલ llvm::json::કિંમત &Data, llvm::raw_ostream &OS) {
  RawMustacheOutputStream MOS(OS);
  વૃક્ષ->render(Data, MOS);
}

શૂન્ય Template::registerPartial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_વૃક્ષ = P.parse();
  Ctx.Partials.ઉમેરો(std::make_pair(Name, Partial_વૃક્ષ));
}

શૂન્ય Template::registerLambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

શૂન્ય Template::registerLambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

શૂન્ય Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  વૃક્ષ = P.parse();
  // The default behavior is to escape html entities.
  અચલ EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), વૃક્ષ(Other.વૃક્ષ) {
  Other.વૃક્ષ = nullptr;
}

Template::~Template() = મૂળભૂત;

} // namespace llvm::mustache

#undef DEBUG_TYPE
