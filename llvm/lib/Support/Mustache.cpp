//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#समावेश "llvm/Support/Mustache.h"
#समावेश "llvm/ADT/SmallVector.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <sstream>

#घोषणा DEBUG_TYPE "mustache"

using namespace llvm;
using namespace llvm::mustache;

namespace {

using Accessor = ArrayRef<StringRef>;

अटल bool isFalsey(स्थिर json::मूल्य &V) {
  वापस V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().मूल्य_2()) ||
         (V.getAsArray() && V.getAsArray()->empty());
}

अटल bool isContextFalsey(स्थिर json::मूल्य *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  अगर (!V)
    वापस true;
  वापस isFalsey(*V);
}

अटल व्योम splitAndTrim(StringRef Str, SmallVectorImpl<StringRef> &Tokens) {
  माप_प्रकार CurrentPos = 0;
  जबतक (CurrentPos < Str.आकार_2()) {
    // Find the next delimiter.
    माप_प्रकार DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    अगर (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.आकार_2();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.slice(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    माप_प्रकार Start = Part.find_first_not_of(" \t\r\n");
    अगर (Start != StringRef::npos) {
      माप_प्रकार End = Part.find_last_not_of(" \t\r\n");
      Tokens.धकेलो_back(Part.slice(Start, End + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

अटल Accessor splitMustacheString(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  SmallVector<StringRef> Tokens;
  अगर (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    Tokens.धकेलो_back(".");
  } अन्यथा {
    splitAndTrim(Str, Tokens);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.आकार_2());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  वापस ArrayRef<StringRef>(ArenaTokens, Tokens.आकार_2());
}
} // namespace

namespace llvm::mustache {

श्रेणी MustacheOutputStream : खुला raw_ostream {
खुला:
  MustacheOutputStream() = शेष;
  ~MustacheOutputStream() override = शेष;

  भव व्योम suspendIndentation() {}
  भव व्योम resumeIndentation() {}

निजी:
  व्योम anchor() override;
};

व्योम MustacheOutputStream::anchor() {}

श्रेणी RawMustacheOutputStream : खुला MustacheOutputStream {
खुला:
  RawMustacheOutputStream(raw_ostream &OS) : OS(OS) { SetUnbuffered(); }

निजी:
  raw_ostream &OS;

  व्योम write_impl(स्थिर अक्षर *Ptr, माप_प्रकार आकार) override {
    OS.ग_लिखो(Ptr, आकार);
  }
  uint64_t current_pos() स्थिर override { वापस OS.tell(); }
};

श्रेणी Token {
खुला:
  क्रमागत श्रेणी Type {
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
        Accessor_मूल्य({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, अक्षर Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    अगर (TokenType == Type::Comment)
      वापस;
    StringRef AccessorStr(यह->TokenBody);
    अगर (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_मूल्य = splitMustacheString(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() स्थिर { वापस Accessor_मूल्य; }

  Type getType() स्थिर { वापस TokenType; }

  व्योम setIndentation(माप_प्रकार NewIndentation) { Indentation = NewIndentation; }

  माप_प्रकार getIndentation() स्थिर { वापस Indentation; }

  अटल Type getTokenType(अक्षर Identifier) {
    चयन (Identifier) {
    हाल '#':
      वापस Type::SectionOpen;
    हाल '/':
      वापस Type::SectionClose;
    हाल '^':
      वापस Type::InvertSectionOpen;
    हाल '!':
      वापस Type::Comment;
    हाल '>':
      वापस Type::Partial;
    हाल '&':
      वापस Type::UnescapeVariable;
    हाल '=':
      वापस Type::SetDelimiter;
    शेष:
      वापस Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_मूल्य;
  माप_प्रकार Indentation;
};

using EscapeMap = DenseMap<अक्षर, std::string>;

श्रेणी ASTNode : खुला ilist_गाँठ<ASTNode> {
खुला:
  क्रमागत Type {
    मूल_2,
    Text,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::मूल_2), Parent(nullptr), ParentContext(nullptr) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *Parent)
      : Ctx(Ctx), Ty(Type::Text), Body(Body), Parent(Parent),
        ParentContext(nullptr) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *Parent)
      : Ctx(Ctx), Ty(Ty), Parent(Parent), Accessor_मूल्य(Accessor),
        ParentContext(nullptr) {}

  व्योम addChild(AstPtr Child) { Children.धकेलो_back(Child); };

  व्योम setRawBody(StringRef NewBody) { RawBody = NewBody; };

  व्योम setIndentation(माप_प्रकार NewIndentation) { Indentation = NewIndentation; };

  व्योम render(स्थिर llvm::json::मूल्य &तथ्य_2, MustacheOutputStream &OS);

निजी:
  व्योम renderLambdas(स्थिर llvm::json::मूल्य &Contexts,
                     MustacheOutputStream &OS, Lambda &L);

  व्योम renderSectionLambdas(स्थिर llvm::json::मूल्य &Contexts,
                            MustacheOutputStream &OS, SectionLambda &L);

  व्योम renderPartial(स्थिर llvm::json::मूल्य &Contexts,
                     MustacheOutputStream &OS, ASTNode *Partial);

  व्योम renderChild(स्थिर llvm::json::मूल्य &Context, MustacheOutputStream &OS);

  स्थिर llvm::json::मूल्य *findContext();

  व्योम render_मूल(स्थिर json::मूल्य &CurrentCtx, MustacheOutputStream &OS);
  व्योम renderText(MustacheOutputStream &OS);
  व्योम renderPartial(स्थिर json::मूल्य &CurrentCtx, MustacheOutputStream &OS);
  व्योम renderVariable(स्थिर json::मूल्य &CurrentCtx, MustacheOutputStream &OS);
  व्योम renderUnescapeVariable(स्थिर json::मूल्य &CurrentCtx,
                              MustacheOutputStream &OS);
  व्योम renderSection(स्थिर json::मूल्य &CurrentCtx, MustacheOutputStream &OS);
  व्योम renderInvertSection(स्थिर json::मूल्य &CurrentCtx,
                           MustacheOutputStream &OS);

  MustacheContext &Ctx;
  Type Ty;
  माप_प्रकार Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  स्थिर ArrayRef<StringRef> Accessor_मूल्य;
  स्थिर llvm::json::मूल्य *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
अटल AstPtr create_मूल_गाँठ(MustacheContext &Ctx) {
  वापस नया (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

अटल AstPtr create_गाँठ(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  वापस नया (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

अटल AstPtr create_Text_गाँठ(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  वापस नया (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
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
अटल bool hasTextBehind(माप_प्रकार Idx, स्थिर ArrayRef<Token> &Tokens) {
  अगर (Idx == 0)
    वापस true;

  माप_प्रकार पूर्व_Idx = Idx - 1;
  अगर (Tokens[पूर्व_Idx].getType() != Token::Type::Text)
    वापस true;

  स्थिर Token &पूर्व_Token = Tokens[पूर्व_Idx];
  StringRef TokenBody = StringRef(पूर्व_Token.RawBody).rtrim(" \r\t\v");
  वापस !TokenBody.ends_with("\n") && !(TokenBody.empty() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
अटल bool hasTextAhead(माप_प्रकार Idx, स्थिर ArrayRef<Token> &Tokens) {
  अगर (Idx >= Tokens.आकार_2() - 1)
    वापस true;

  माप_प्रकार अग्र_Idx = Idx + 1;
  अगर (Tokens[अग्र_Idx].getType() != Token::Type::Text)
    वापस true;

  स्थिर Token &अग्र_Token = Tokens[अग्र_Idx];
  StringRef TokenBody = StringRef(अग्र_Token.RawBody).ltrim(" ");
  वापस !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

अटल bool requiresCleanUp(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  वापस T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::SetDelimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
अटल व्योम stripTokenAhead(SmallVectorImpl<Token> &Tokens, माप_प्रकार Idx) {
  Token &अग्र_Token = Tokens[Idx + 1];
  StringRef अग्र_Token_Body = अग्र_Token.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  अगर (अग्र_Token_Body.starts_with("\r\n"))
    अग्र_Token.TokenBody = अग्र_Token_Body.substr(2);
  अन्यथा अगर (अग्र_Token_Body.starts_with("\n"))
    अग्र_Token.TokenBody = अग्र_Token_Body.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
व्योम stripTokenBefore(SmallVectorImpl<Token> &Tokens, माप_प्रकार Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &पूर्व_Token = Tokens[Idx - 1];
  StringRef पूर्व_Token_Body = पूर्व_Token.TokenBody;
  StringRef Unindented = पूर्व_Token_Body.rtrim(" \r\t\v");
  माप_प्रकार Indentation = पूर्व_Token_Body.आकार_2() - Unindented.आकार_2();
  पूर्व_Token.TokenBody = Unindented;
  CurrentToken.setIndentation(Indentation);
}

काष्ठा Tag {
  क्रमागत श्रेणी Kind {
    None,
    Normal, // {{...}}
    Triple, // {{{...}}}
  };

  Kind TagKind = Kind::None;
  StringRef Content;   // The content between the delimiters.
  StringRef FullMatch; // The entire tag, including delimiters.
  माप_प्रकार StartPosition = StringRef::npos;
};

[[maybe_unused]] अटल स्थिर अक्षर *tagKindToString(Tag::Kind K) {
  चयन (K) {
  हाल Tag::Kind::None:
    वापस "None";
  हाल Tag::Kind::Normal:
    वापस "Normal";
  हाल Tag::Kind::Triple:
    वापस "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_unused]] अटल स्थिर अक्षर *jsonKindToString(json::मूल्य::Kind K) {
  चयन (K) {
  हाल json::मूल्य::Kind::Null:
    वापस "JSON_KIND_NULL";
  हाल json::मूल्य::Kind::Boolean:
    वापस "JSON_KIND_BOOLEAN";
  हाल json::मूल्य::Kind::संख्या:
    वापस "JSON_KIND_NUMBER";
  हाल json::मूल्य::Kind::String:
    वापस "JSON_KIND_STRING";
  हाल json::मूल्य::Kind::Array:
    वापस "JSON_KIND_ARRAY";
  हाल json::मूल्य::Kind::Object:
    वापस "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
अटल SmallVector<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  SmallVector<Token> Tokens;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  माप_प्रकार Cursor = 0;
  माप_प्रकार TextStart = 0;

  स्थिर StringLiteral TripleOpen("{{{");
  स्थिर StringLiteral TripleClose("}}}");

  जबतक (Cursor < Template.आकार_2()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef TagOpen, TagClose;
    Tag::Kind Kind;

    // Determine which tag we've encountered.
    अगर (TemplateSuffix.starts_with(TripleOpen)) {
      Kind = Tag::Kind::Triple;
      TagOpen = TripleOpen;
      TagClose = TripleClose;
    } अन्यथा अगर (TemplateSuffix.starts_with(Open)) {
      Kind = Tag::Kind::Normal;
      TagOpen = Open;
      TagClose = Close;
    } अन्यथा {
      // Not at a tag, continue scanning.
      ++Cursor;
      जारी;
    }

    // Found a tag, first add the preceding text.
    अगर (Cursor > TextStart)
      Tokens.emplace_back(Template.slice(TextStart, Cursor));

    // Find the closing tag.
    माप_प्रकार EndPos = Template.find(TagClose, Cursor + TagOpen.आकार_2());
    अगर (EndPos == StringRef::npos) {
      // No closing tag, the rest is text.
      Tokens.emplace_back(Template.substr(Cursor));
      TextStart = Cursor = Template.आकार_2();
      अवरोध;
    }

    // Extract tag content and full match.
    माप_प्रकार ContentStart = Cursor + TagOpen.आकार_2();
    StringRef Content = Template.substr(ContentStart, EndPos - ContentStart);
    StringRef FullMatch =
        Template.substr(Cursor, (EndPos + TagClose.आकार_2()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << FullMatch << ", Content: " << Content
                      << ", Kind: " << tagKindToString(Kind) << "\n");
    अगर (Kind == Tag::Kind::Triple) {
      Tokens.emplace_back(FullMatch, Ctx.Saver.save("&" + Content), '&', Ctx);
    } अन्यथा { // Normal Tag
      StringRef Interpolated = Content;
      अगर (!Interpolated.trim().starts_with("=")) {
        अक्षर Front = Interpolated.empty() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_back(FullMatch, Interpolated, Front, Ctx);
      } अन्यथा { // Set Delimiter
        Tokens.emplace_back(FullMatch, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](अक्षर C) { वापस C == '='; });
        DelimSpec = DelimSpec.trim();

        स्वतः [NewOpen, NewClose] = DelimSpec.split(' ');
        LLVM_DEBUG(dbgs() << "[Set Delimiter] NewOpen: " << NewOpen
                          << ", NewClose: " << NewClose << "\n");
        Open = NewOpen;
        Close = NewClose;
      }
    }

    // Move past the tag for the next iteration.
    Cursor += FullMatch.आकार_2();
    TextStart = Cursor;
  }

  // Add any remaining text after the last tag.
  अगर (TextStart < Template.आकार_2())
    Tokens.emplace_back(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  माप_प्रकार LastIdx = Tokens.आकार_2() - 1;
  क्रम (माप_प्रकार Idx = 0, End = Tokens.आकार_2(); Idx < End; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    अगर (!requiresCleanUp(CurrentType))
      जारी;

    bool HasTextBehind = hasTextBehind(Idx, Tokens);
    bool HasTextAhead = hasTextAhead(Idx, Tokens);

    अगर ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    अगर ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == LastIdx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  वापस Tokens;
}

// Custom stream to escape strings.
श्रेणी EscapeStringStream : खुला MustacheOutputStream {
खुला:
  explicit EscapeStringStream(llvm::raw_ostream &WrappedStream,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.कुंजियाँ().begin(), Escape.कुंजियाँ().end()),
        WrappedStream(WrappedStream) {
    SetUnbuffered();
  }

रक्षित:
  व्योम write_impl(स्थिर अक्षर *Ptr, माप_प्रकार आकार) override {
    StringRef तथ्य_2(Ptr, आकार);
    माप_प्रकार Start = 0;
    जबतक (Start < आकार) {
      // Find the next character that needs to be escaped.
      माप_प्रकार अग्र_2 = तथ्य_2.find_first_of(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      अगर (अग्र_2 == StringRef::npos) {
        WrappedStream << तथ्य_2.substr(Start);
        वापस;
      }

      // Write the chunk of text before the escapable character.
      अगर (अग्र_2 > Start)
        WrappedStream << तथ्य_2.substr(Start, अग्र_2 - Start);

      // Look up and write the escaped version of the character.
      WrappedStream << Escape[तथ्य_2[अग्र_2]];
      Start = अग्र_2 + 1;
    }
  }

  uint64_t current_pos() स्थिर override { वापस WrappedStream.tell(); }

निजी:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &WrappedStream;
};

// Custom stream to add indentation used to for rendering partials.
श्रेणी AddIndentationStringStream : खुला MustacheOutputStream {
खुला:
  explicit AddIndentationStringStream(raw_ostream &WrappedStream,
                                      माप_प्रकार Indentation)
      : Indentation(Indentation), WrappedStream(WrappedStream),
        NeedsIndent(true), IsSuspended(false) {
    SetUnbuffered();
  }

  व्योम suspendIndentation() override { IsSuspended = true; }
  व्योम resumeIndentation() override { IsSuspended = false; }

रक्षित:
  व्योम write_impl(स्थिर अक्षर *Ptr, माप_प्रकार आकार) override {
    llvm::StringRef तथ्य_2(Ptr, आकार);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    क्रम (अक्षर C : तथ्य_2) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      अगर (NeedsIndent && C != '\n') {
        WrappedStream << Indent;
        NeedsIndent = false;
      }
      WrappedStream << C;
      अगर (C == '\n' && !IsSuspended)
        NeedsIndent = true;
    }
  }

  uint64_t current_pos() स्थिर override { वापस WrappedStream.tell(); }

निजी:
  माप_प्रकार Indentation;
  raw_ostream &WrappedStream;
  bool NeedsIndent;
  bool IsSuspended;
};

श्रेणी Parser {
खुला:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

निजी:
  व्योम parseMustache(ASTNode *Parent);
  व्योम parseSection(ASTNode *Parent, ASTNode::Type Ty, स्थिर Accessor &A);

  MustacheContext &Ctx;
  SmallVector<Token> Tokens;
  माप_प्रकार CurrentPtr;
  StringRef TemplateStr;
};

व्योम Parser::parseSection(ASTNode *Parent, ASTNode::Type Ty,
                          स्थिर Accessor &A) {
  AstPtr Current_गाँठ = create_गाँठ(Ctx, Ty, A, Parent);
  माप_प्रकार Start = CurrentPtr;
  parseMustache(Current_गाँठ);
  स्थिर माप_प्रकार End = CurrentPtr - 1;

  माप_प्रकार Raw_Body_आकार = 0;
  क्रम (माप_प्रकार I = Start; I < End; ++I)
    Raw_Body_आकार += Tokens[I].RawBody.आकार_2();

  SmallString<128> RawBody;
  RawBody.reserve(Raw_Body_आकार);
  क्रम (std::माप_प्रकार I = Start; I < End; ++I)
    RawBody += Tokens[I].RawBody;

  Current_गाँठ->setRawBody(Ctx.Saver.save(StringRef(RawBody)));
  Parent->addChild(Current_गाँठ);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr मूल_गाँठ = create_मूल_गाँठ(Ctx);
  parseMustache(मूल_गाँठ);
  वापस मूल_गाँठ;
}

व्योम Parser::parseMustache(ASTNode *Parent) {

  जबतक (CurrentPtr < Tokens.आकार_2()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_गाँठ;

    चयन (CurrentToken.getType()) {
    हाल Token::Type::Text: {
      Current_गाँठ = create_Text_गाँठ(Ctx, CurrentToken.TokenBody, Parent);
      Parent->addChild(Current_गाँठ);
      अवरोध;
    }
    हाल Token::Type::Variable: {
      Current_गाँठ = create_गाँठ(Ctx, ASTNode::Variable, A, Parent);
      Parent->addChild(Current_गाँठ);
      अवरोध;
    }
    हाल Token::Type::UnescapeVariable: {
      Current_गाँठ = create_गाँठ(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->addChild(Current_गाँठ);
      अवरोध;
    }
    हाल Token::Type::Partial: {
      Current_गाँठ = create_गाँठ(Ctx, ASTNode::Partial, A, Parent);
      Current_गाँठ->setIndentation(CurrentToken.getIndentation());
      Parent->addChild(Current_गाँठ);
      अवरोध;
    }
    हाल Token::Type::SectionOpen: {
      parseSection(Parent, ASTNode::Section, A);
      अवरोध;
    }
    हाल Token::Type::InvertSectionOpen: {
      parseSection(Parent, ASTNode::InvertSection, A);
      अवरोध;
    }
    हाल Token::Type::Comment:
    हाल Token::Type::SetDelimiter:
      अवरोध;
    हाल Token::Type::SectionClose:
      वापस;
    }
  }
}
अटल व्योम toMustacheString(स्थिर json::मूल्य &तथ्य_2, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << jsonKindToString(तथ्य_2.kind()) << ", Data: " << तथ्य_2
                    << "\n");
  चयन (तथ्य_2.kind()) {
  हाल json::मूल्य::Null:
    वापस;
  हाल json::मूल्य::संख्या: {
    स्वतः अंक = *तथ्य_2.get_As_संख्या();
    std::ostringstream SS;
    SS << अंक;
    OS << SS.str();
    वापस;
  }
  हाल json::मूल्य::String: {
    OS << *तथ्य_2.getAsString();
    वापस;
  }

  हाल json::मूल्य::Array: {
    स्वतः Arr = *तथ्य_2.getAsArray();
    अगर (Arr.empty())
      वापस;
    [[fallthrough]];
  }
  हाल json::मूल्य::Object:
  हाल json::मूल्य::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.मूल्य_2(तथ्य_2);
    अवरोध;
  }
  }
}

व्योम ASTNode::render_मूल(स्थिर json::मूल्य &CurrentCtx,
                         MustacheOutputStream &OS) {
  renderChild(CurrentCtx, OS);
}

व्योम ASTNode::renderText(MustacheOutputStream &OS) { OS << Body; }

व्योम ASTNode::renderPartial(स्थिर json::मूल्य &CurrentCtx,
                            MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_मूल्य[0]
                    << ", Indentation:" << Indentation << "\n");
  स्वतः Partial = Ctx.Partials.find(Accessor_मूल्य[0]);
  अगर (Partial != Ctx.Partials.end())
    renderPartial(CurrentCtx, OS, Partial->get_मूल्य());
}

व्योम ASTNode::renderVariable(स्थिर json::मूल्य &CurrentCtx,
                             MustacheOutputStream &OS) {
  स्वतः Lambda = Ctx.Lambdas.find(Accessor_मूल्य[0]);
  अगर (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_मूल्य());
  } अन्यथा अगर (स्थिर json::मूल्य *ContextPtr = findContext()) {
    EscapeStringStream ES(OS, Ctx.Escapes);
    toMustacheString(*ContextPtr, ES);
  }
}

व्योम ASTNode::renderUnescapeVariable(स्थिर json::मूल्य &CurrentCtx,
                                     MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_मूल्य[0]
                    << "\n");
  स्वतः Lambda = Ctx.Lambdas.find(Accessor_मूल्य[0]);
  अगर (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_मूल्य());
  } अन्यथा अगर (स्थिर json::मूल्य *ContextPtr = findContext()) {
    OS.suspendIndentation();
    toMustacheString(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

व्योम ASTNode::renderSection(स्थिर json::मूल्य &CurrentCtx,
                            MustacheOutputStream &OS) {
  स्वतः SectionLambda = Ctx.SectionLambdas.find(Accessor_मूल्य[0]);
  अगर (SectionLambda != Ctx.SectionLambdas.end()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_मूल्य());
    वापस;
  }

  स्थिर json::मूल्य *ContextPtr = findContext();
  अगर (isContextFalsey(ContextPtr))
    वापस;

  अगर (स्थिर json::Array *Arr = ContextPtr->getAsArray()) {
    क्रम (स्थिर json::मूल्य &V : *Arr)
      renderChild(V, OS);
    वापस;
  }
  renderChild(*ContextPtr, OS);
}

व्योम ASTNode::renderInvertSection(स्थिर json::मूल्य &CurrentCtx,
                                  MustacheOutputStream &OS) {
  bool IsLambda = Ctx.SectionLambdas.contains(Accessor_मूल्य[0]);
  स्थिर json::मूल्य *ContextPtr = findContext();
  अगर (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

व्योम ASTNode::render(स्थिर llvm::json::मूल्य &तथ्य_2, MustacheOutputStream &OS) {
  अगर (Ty != मूल_2 && Ty != Text && Accessor_मूल्य.empty())
    वापस;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &तथ्य_2;

  चयन (Ty) {
  हाल मूल_2:
    render_मूल(तथ्य_2, OS);
    वापस;
  हाल Text:
    renderText(OS);
    वापस;
  हाल Partial:
    renderPartial(तथ्य_2, OS);
    वापस;
  हाल Variable:
    renderVariable(तथ्य_2, OS);
    वापस;
  हाल UnescapeVariable:
    renderUnescapeVariable(तथ्य_2, OS);
    वापस;
  हाल Section:
    renderSection(तथ्य_2, OS);
    वापस;
  हाल InvertSection:
    renderInvertSection(तथ्य_2, OS);
    वापस;
  }
  llvm_unreachable("Invalid ASTNode type");
}

स्थिर json::मूल्य *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  अगर (Accessor_मूल्य.empty())
    वापस nullptr;
  अगर (Accessor_मूल्य[0] == ".")
    वापस ParentContext;

  स्थिर json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = Accessor_मूल्य[0];
  ASTNode *CurrentParent = Parent;

  जबतक (!CurrentContext || !CurrentContext->लो(CurrentAccessor)) {
    अगर (CurrentParent->Ty != मूल_2) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      जारी;
    }
    वापस nullptr;
  }
  स्थिर json::मूल्य *Context = nullptr;
  क्रम (स्वतः [Idx, Acc] : enumerate(Accessor_मूल्य)) {
    स्थिर json::मूल्य *Current_मूल्य = CurrentContext->लो(Acc);
    अगर (!Current_मूल्य)
      वापस nullptr;
    अगर (Idx < Accessor_मूल्य.आकार_2() - 1) {
      CurrentContext = Current_मूल्य->getAsObject();
      अगर (!CurrentContext)
        वापस nullptr;
    } अन्यथा {
      Context = Current_मूल्य;
    }
  }
  वापस Context;
}

व्योम ASTNode::renderChild(स्थिर json::मूल्य &Contexts,
                          MustacheOutputStream &OS) {
  क्रम (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

व्योम ASTNode::renderPartial(स्थिर json::मूल्य &Contexts,
                            MustacheOutputStream &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  AddIndentationStringStream IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

व्योम ASTNode::renderLambdas(स्थिर llvm::json::मूल्य &Contexts,
                            MustacheOutputStream &OS, Lambda &L) {
  json::मूल्य Lambda_परिणाम = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Lambda_परिणाम, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_गाँठ = P.parse();

  EscapeStringStream ES(OS, Ctx.Escapes);
  अगर (Ty == Variable) {
    Lambda_गाँठ->render(Contexts, ES);
    वापस;
  }
  Lambda_गाँठ->render(Contexts, OS);
}

व्योम ASTNode::renderSectionLambdas(स्थिर llvm::json::मूल्य &Contexts,
                                   MustacheOutputStream &OS, SectionLambda &L) {
  json::मूल्य Return = L(RawBody.str());
  अगर (isFalsey(Return))
    वापस;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_गाँठ = P.parse();
  Lambda_गाँठ->render(Contexts, OS);
}

व्योम Template::render(स्थिर llvm::json::मूल्य &तथ्य_2, llvm::raw_ostream &OS) {
  RawMustacheOutputStream MOS(OS);
  वृक्ष->render(तथ्य_2, MOS);
}

व्योम Template::registerPartial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_वृक्ष = P.parse();
  Ctx.Partials.जोड़ो(std::make_pair(Name, Partial_वृक्ष));
}

व्योम Template::registerLambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

व्योम Template::registerLambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

व्योम Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  वृक्ष = P.parse();
  // The default behavior is to escape html entities.
  स्थिर EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), वृक्ष(Other.वृक्ष) {
  Other.वृक्ष = nullptr;
}

Template::~Template() = शेष;

} // namespace llvm::mustache

#अघोषित DEBUG_TYPE
