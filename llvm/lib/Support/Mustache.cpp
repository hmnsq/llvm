//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#अन्तर्भाव "llvm/Support/Mustache.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <sstream>

#define DEBUG_TYPE "mustache"

using नामाकाश llvm;
using नामाकाश llvm::mustache;

नामाकाश {

using Accessor = ArrayRef<StringRef>;

स्थिर तार्किक isFalsey(स्थिरांक json::मूल्य &V) {
  परत V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().मूल्य_2()) ||
         (V.getAsArray() && V.getAsArray()->empty());
}

स्थिर तार्किक isContextFalsey(स्थिरांक json::मूल्य *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  जर (!V)
    परत सत्य;
  परत isFalsey(*V);
}

स्थिर शून्य splitAndTrim(StringRef Str, SmallVectorImpl<StringRef> &Tokens) {
  माप_प्रकार CurrentPos = 0;
  पर्यंत (CurrentPos < Str.size()) {
    // Find the next delimiter.
    माप_प्रकार DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    जर (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.size();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.slice(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    माप_प्रकार Start = Part.find_first_not_of(" \t\r\n");
    जर (Start != StringRef::npos) {
      माप_प्रकार End = Part.find_last_not_of(" \t\r\n");
      Tokens.ढकला_back(Part.slice(Start, End + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

स्थिर Accessor splitMustacheString(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  SmallVector<StringRef> Tokens;
  जर (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    Tokens.ढकला_back(".");
  } अन्यथा {
    splitAndTrim(Str, Tokens);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.size());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  परत ArrayRef<StringRef>(ArenaTokens, Tokens.size());
}
} // namespace

नामाकाश llvm::mustache {

वर्ग MustacheOutputStream : सार्वजनिक raw_ostream {
सार्वजनिक:
  MustacheOutputStream() = पूर्वनिर्धारित;
  ~MustacheOutputStream() override = पूर्वनिर्धारित;

  आभासी शून्य suspendIndentation() {}
  आभासी शून्य resumeIndentation() {}

खाजगी:
  शून्य anchor() override;
};

शून्य MustacheOutputStream::anchor() {}

वर्ग RawMustacheOutputStream : सार्वजनिक MustacheOutputStream {
सार्वजनिक:
  RawMustacheOutputStream(raw_ostream &OS) : OS(OS) { SetUnbuffered(); }

खाजगी:
  raw_ostream &OS;

  शून्य write_impl(स्थिरांक अक्षर *Ptr, माप_प्रकार Size) override {
    OS.लिख(Ptr, Size);
  }
  uint64_t current_pos() स्थिरांक override { परत OS.tell(); }
};

वर्ग Token {
सार्वजनिक:
  गणना वर्ग Type {
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
    जर (TokenType == Type::Comment)
      परत;
    StringRef AccessorStr(हे->TokenBody);
    जर (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_मूल्य = splitMustacheString(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() स्थिरांक { परत Accessor_मूल्य; }

  Type getType() स्थिरांक { परत TokenType; }

  शून्य setIndentation(माप_प्रकार NewIndentation) { Indentation = NewIndentation; }

  माप_प्रकार getIndentation() स्थिरांक { परत Indentation; }

  स्थिर Type getTokenType(अक्षर Identifier) {
    निवड (Identifier) {
    स्थिती '#':
      परत Type::SectionOpen;
    स्थिती '/':
      परत Type::SectionClose;
    स्थिती '^':
      परत Type::InvertSectionOpen;
    स्थिती '!':
      परत Type::Comment;
    स्थिती '>':
      परत Type::Partial;
    स्थिती '&':
      परत Type::UnescapeVariable;
    स्थिती '=':
      परत Type::SetDelimiter;
    पूर्वनिर्धारित:
      परत Type::Variable;
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

वर्ग ASTNode : सार्वजनिक ilist_गाठ<ASTNode> {
सार्वजनिक:
  गणना Type {
    मूळ_2,
    Text,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::मूळ_2), Parent(nullptr), ParentContext(nullptr) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *Parent)
      : Ctx(Ctx), Ty(Type::Text), Body(Body), Parent(Parent),
        ParentContext(nullptr) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *Parent)
      : Ctx(Ctx), Ty(Ty), Parent(Parent), Accessor_मूल्य(Accessor),
        ParentContext(nullptr) {}

  शून्य addChild(AstPtr Child) { Children.ढकला_back(Child); };

  शून्य setRawBody(StringRef NewBody) { RawBody = NewBody; };

  शून्य setIndentation(माप_प्रकार NewIndentation) { Indentation = NewIndentation; };

  शून्य render(स्थिरांक llvm::json::मूल्य &Data, MustacheOutputStream &OS);

खाजगी:
  शून्य renderLambdas(स्थिरांक llvm::json::मूल्य &Contexts,
                     MustacheOutputStream &OS, Lambda &L);

  शून्य renderSectionLambdas(स्थिरांक llvm::json::मूल्य &Contexts,
                            MustacheOutputStream &OS, SectionLambda &L);

  शून्य renderPartial(स्थिरांक llvm::json::मूल्य &Contexts,
                     MustacheOutputStream &OS, ASTNode *Partial);

  शून्य renderChild(स्थिरांक llvm::json::मूल्य &Context, MustacheOutputStream &OS);

  स्थिरांक llvm::json::मूल्य *findContext();

  शून्य render_मूळ(स्थिरांक json::मूल्य &CurrentCtx, MustacheOutputStream &OS);
  शून्य renderText(MustacheOutputStream &OS);
  शून्य renderPartial(स्थिरांक json::मूल्य &CurrentCtx, MustacheOutputStream &OS);
  शून्य renderVariable(स्थिरांक json::मूल्य &CurrentCtx, MustacheOutputStream &OS);
  शून्य renderUnescapeVariable(स्थिरांक json::मूल्य &CurrentCtx,
                              MustacheOutputStream &OS);
  शून्य renderSection(स्थिरांक json::मूल्य &CurrentCtx, MustacheOutputStream &OS);
  शून्य renderInvertSection(स्थिरांक json::मूल्य &CurrentCtx,
                           MustacheOutputStream &OS);

  MustacheContext &Ctx;
  Type Ty;
  माप_प्रकार Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  स्थिरांक ArrayRef<StringRef> Accessor_मूल्य;
  स्थिरांक llvm::json::मूल्य *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
स्थिर AstPtr create_मूळ_गाठ(MustacheContext &Ctx) {
  परत नवीन (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

स्थिर AstPtr create_गाठ(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  परत नवीन (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

स्थिर AstPtr create_Text_गाठ(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  परत नवीन (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
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
स्थिर तार्किक hasTextBehind(माप_प्रकार Idx, स्थिरांक ArrayRef<Token> &Tokens) {
  जर (Idx == 0)
    परत सत्य;

  माप_प्रकार PrevIdx = Idx - 1;
  जर (Tokens[PrevIdx].getType() != Token::Type::Text)
    परत सत्य;

  स्थिरांक Token &PrevToken = Tokens[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  परत !TokenBody.ends_with("\n") && !(TokenBody.empty() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
स्थिर तार्किक hasTextAhead(माप_प्रकार Idx, स्थिरांक ArrayRef<Token> &Tokens) {
  जर (Idx >= Tokens.size() - 1)
    परत सत्य;

  माप_प्रकार NextIdx = Idx + 1;
  जर (Tokens[NextIdx].getType() != Token::Type::Text)
    परत सत्य;

  स्थिरांक Token &NextToken = Tokens[NextIdx];
  StringRef TokenBody = StringRef(NextToken.RawBody).ltrim(" ");
  परत !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

स्थिर तार्किक requiresCleanUp(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  परत T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::SetDelimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
स्थिर शून्य stripTokenAhead(SmallVectorImpl<Token> &Tokens, माप_प्रकार Idx) {
  Token &NextToken = Tokens[Idx + 1];
  StringRef NextTokenBody = NextToken.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  जर (NextTokenBody.starts_with("\r\n"))
    NextToken.TokenBody = NextTokenBody.substr(2);
  अन्यथा जर (NextTokenBody.starts_with("\n"))
    NextToken.TokenBody = NextTokenBody.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
शून्य stripTokenBefore(SmallVectorImpl<Token> &Tokens, माप_प्रकार Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = Tokens[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  माप_प्रकार Indentation = PrevTokenBody.size() - Unindented.size();
  PrevToken.TokenBody = Unindented;
  CurrentToken.setIndentation(Indentation);
}

संरचना Tag {
  गणना वर्ग Kind {
    None,
    Normal, // {{...}}
    Triple, // {{{...}}}
  };

  Kind TagKind = Kind::None;
  StringRef Content;   // The content between the delimiters.
  StringRef FullMatch; // The entire tag, including delimiters.
  माप_प्रकार StartPosition = StringRef::npos;
};

[[maybe_unused]] स्थिर स्थिरांक अक्षर *tagKindToString(Tag::Kind K) {
  निवड (K) {
  स्थिती Tag::Kind::None:
    परत "None";
  स्थिती Tag::Kind::Normal:
    परत "Normal";
  स्थिती Tag::Kind::Triple:
    परत "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_unused]] स्थिर स्थिरांक अक्षर *jsonKindToString(json::मूल्य::Kind K) {
  निवड (K) {
  स्थिती json::मूल्य::Kind::Null:
    परत "JSON_KIND_NULL";
  स्थिती json::मूल्य::Kind::Boolean:
    परत "JSON_KIND_BOOLEAN";
  स्थिती json::मूल्य::Kind::संख्या:
    परत "JSON_KIND_NUMBER";
  स्थिती json::मूल्य::Kind::String:
    परत "JSON_KIND_STRING";
  स्थिती json::मूल्य::Kind::Array:
    परत "JSON_KIND_ARRAY";
  स्थिती json::मूल्य::Kind::Object:
    परत "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
स्थिर SmallVector<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  SmallVector<Token> Tokens;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  माप_प्रकार Cursor = 0;
  माप_प्रकार TextStart = 0;

  स्थिरांक StringLiteral TripleOpen("{{{");
  स्थिरांक StringLiteral TripleClose("}}}");

  पर्यंत (Cursor < Template.size()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef TagOpen, TagClose;
    Tag::Kind Kind;

    // Determine which tag we've encountered.
    जर (TemplateSuffix.starts_with(TripleOpen)) {
      Kind = Tag::Kind::Triple;
      TagOpen = TripleOpen;
      TagClose = TripleClose;
    } अन्यथा जर (TemplateSuffix.starts_with(Open)) {
      Kind = Tag::Kind::Normal;
      TagOpen = Open;
      TagClose = Close;
    } अन्यथा {
      // Not at a tag, continue scanning.
      ++Cursor;
      सुरूठेवा;
    }

    // Found a tag, first add the preceding text.
    जर (Cursor > TextStart)
      Tokens.emplace_back(Template.slice(TextStart, Cursor));

    // Find the closing tag.
    माप_प्रकार EndPos = Template.find(TagClose, Cursor + TagOpen.size());
    जर (EndPos == StringRef::npos) {
      // No closing tag, the rest is text.
      Tokens.emplace_back(Template.substr(Cursor));
      TextStart = Cursor = Template.size();
      थांबा;
    }

    // Extract tag content and full match.
    माप_प्रकार ContentStart = Cursor + TagOpen.size();
    StringRef Content = Template.substr(ContentStart, EndPos - ContentStart);
    StringRef FullMatch =
        Template.substr(Cursor, (EndPos + TagClose.size()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << FullMatch << ", Content: " << Content
                      << ", Kind: " << tagKindToString(Kind) << "\n");
    जर (Kind == Tag::Kind::Triple) {
      Tokens.emplace_back(FullMatch, Ctx.Saver.save("&" + Content), '&', Ctx);
    } अन्यथा { // Normal Tag
      StringRef Interpolated = Content;
      जर (!Interpolated.trim().starts_with("=")) {
        अक्षर Front = Interpolated.empty() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_back(FullMatch, Interpolated, Front, Ctx);
      } अन्यथा { // Set Delimiter
        Tokens.emplace_back(FullMatch, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](अक्षर C) { परत C == '='; });
        DelimSpec = DelimSpec.trim();

        स्वतः [NewOpen, NewClose] = DelimSpec.split(' ');
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
  जर (TextStart < Template.size())
    Tokens.emplace_back(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  माप_प्रकार LastIdx = Tokens.size() - 1;
  साठी (माप_प्रकार Idx = 0, End = Tokens.size(); Idx < End; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    जर (!requiresCleanUp(CurrentType))
      सुरूठेवा;

    तार्किक HasTextBehind = hasTextBehind(Idx, Tokens);
    तार्किक HasTextAhead = hasTextAhead(Idx, Tokens);

    जर ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    जर ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == LastIdx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  परत Tokens;
}

// Custom stream to escape strings.
वर्ग EscapeStringStream : सार्वजनिक MustacheOutputStream {
सार्वजनिक:
  explicit EscapeStringStream(llvm::raw_ostream &WrappedStream,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.किल्ल्या().begin(), Escape.किल्ल्या().end()),
        WrappedStream(WrappedStream) {
    SetUnbuffered();
  }

संरक्षित:
  शून्य write_impl(स्थिरांक अक्षर *Ptr, माप_प्रकार Size) override {
    StringRef Data(Ptr, Size);
    माप_प्रकार Start = 0;
    पर्यंत (Start < Size) {
      // Find the next character that needs to be escaped.
      माप_प्रकार Next = Data.find_first_of(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      जर (Next == StringRef::npos) {
        WrappedStream << Data.substr(Start);
        परत;
      }

      // Write the chunk of text before the escapable character.
      जर (Next > Start)
        WrappedStream << Data.substr(Start, Next - Start);

      // Look up and write the escaped version of the character.
      WrappedStream << Escape[Data[Next]];
      Start = Next + 1;
    }
  }

  uint64_t current_pos() स्थिरांक override { परत WrappedStream.tell(); }

खाजगी:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &WrappedStream;
};

// Custom stream to add indentation used to for rendering partials.
वर्ग AddIndentationStringStream : सार्वजनिक MustacheOutputStream {
सार्वजनिक:
  explicit AddIndentationStringStream(raw_ostream &WrappedStream,
                                      माप_प्रकार Indentation)
      : Indentation(Indentation), WrappedStream(WrappedStream),
        NeedsIndent(सत्य), IsSuspended(असत्य) {
    SetUnbuffered();
  }

  शून्य suspendIndentation() override { IsSuspended = सत्य; }
  शून्य resumeIndentation() override { IsSuspended = असत्य; }

संरक्षित:
  शून्य write_impl(स्थिरांक अक्षर *Ptr, माप_प्रकार Size) override {
    llvm::StringRef Data(Ptr, Size);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    साठी (अक्षर C : Data) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      जर (NeedsIndent && C != '\n') {
        WrappedStream << Indent;
        NeedsIndent = असत्य;
      }
      WrappedStream << C;
      जर (C == '\n' && !IsSuspended)
        NeedsIndent = सत्य;
    }
  }

  uint64_t current_pos() स्थिरांक override { परत WrappedStream.tell(); }

खाजगी:
  माप_प्रकार Indentation;
  raw_ostream &WrappedStream;
  तार्किक NeedsIndent;
  तार्किक IsSuspended;
};

वर्ग Parser {
सार्वजनिक:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

खाजगी:
  शून्य parseMustache(ASTNode *Parent);
  शून्य parseSection(ASTNode *Parent, ASTNode::Type Ty, स्थिरांक Accessor &A);

  MustacheContext &Ctx;
  SmallVector<Token> Tokens;
  माप_प्रकार CurrentPtr;
  StringRef TemplateStr;
};

शून्य Parser::parseSection(ASTNode *Parent, ASTNode::Type Ty,
                          स्थिरांक Accessor &A) {
  AstPtr Current_गाठ = create_गाठ(Ctx, Ty, A, Parent);
  माप_प्रकार Start = CurrentPtr;
  parseMustache(Current_गाठ);
  स्थिरांक माप_प्रकार End = CurrentPtr - 1;

  माप_प्रकार RawBodySize = 0;
  साठी (माप_प्रकार I = Start; I < End; ++I)
    RawBodySize += Tokens[I].RawBody.size();

  SmallString<128> RawBody;
  RawBody.reserve(RawBodySize);
  साठी (std::माप_प्रकार I = Start; I < End; ++I)
    RawBody += Tokens[I].RawBody;

  Current_गाठ->setRawBody(Ctx.Saver.save(StringRef(RawBody)));
  Parent->addChild(Current_गाठ);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr मूळ_गाठ = create_मूळ_गाठ(Ctx);
  parseMustache(मूळ_गाठ);
  परत मूळ_गाठ;
}

शून्य Parser::parseMustache(ASTNode *Parent) {

  पर्यंत (CurrentPtr < Tokens.size()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_गाठ;

    निवड (CurrentToken.getType()) {
    स्थिती Token::Type::Text: {
      Current_गाठ = create_Text_गाठ(Ctx, CurrentToken.TokenBody, Parent);
      Parent->addChild(Current_गाठ);
      थांबा;
    }
    स्थिती Token::Type::Variable: {
      Current_गाठ = create_गाठ(Ctx, ASTNode::Variable, A, Parent);
      Parent->addChild(Current_गाठ);
      थांबा;
    }
    स्थिती Token::Type::UnescapeVariable: {
      Current_गाठ = create_गाठ(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->addChild(Current_गाठ);
      थांबा;
    }
    स्थिती Token::Type::Partial: {
      Current_गाठ = create_गाठ(Ctx, ASTNode::Partial, A, Parent);
      Current_गाठ->setIndentation(CurrentToken.getIndentation());
      Parent->addChild(Current_गाठ);
      थांबा;
    }
    स्थिती Token::Type::SectionOpen: {
      parseSection(Parent, ASTNode::Section, A);
      थांबा;
    }
    स्थिती Token::Type::InvertSectionOpen: {
      parseSection(Parent, ASTNode::InvertSection, A);
      थांबा;
    }
    स्थिती Token::Type::Comment:
    स्थिती Token::Type::SetDelimiter:
      थांबा;
    स्थिती Token::Type::SectionClose:
      परत;
    }
  }
}
स्थिर शून्य toMustacheString(स्थिरांक json::मूल्य &Data, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << jsonKindToString(Data.kind()) << ", Data: " << Data
                    << "\n");
  निवड (Data.kind()) {
  स्थिती json::मूल्य::Null:
    परत;
  स्थिती json::मूल्य::संख्या: {
    स्वतः अंक = *Data.get_As_संख्या();
    std::ostringstream SS;
    SS << अंक;
    OS << SS.str();
    परत;
  }
  स्थिती json::मूल्य::String: {
    OS << *Data.getAsString();
    परत;
  }

  स्थिती json::मूल्य::Array: {
    स्वतः Arr = *Data.getAsArray();
    जर (Arr.empty())
      परत;
    [[fallthrough]];
  }
  स्थिती json::मूल्य::Object:
  स्थिती json::मूल्य::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.मूल्य_2(Data);
    थांबा;
  }
  }
}

शून्य ASTNode::render_मूळ(स्थिरांक json::मूल्य &CurrentCtx,
                         MustacheOutputStream &OS) {
  renderChild(CurrentCtx, OS);
}

शून्य ASTNode::renderText(MustacheOutputStream &OS) { OS << Body; }

शून्य ASTNode::renderPartial(स्थिरांक json::मूल्य &CurrentCtx,
                            MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_मूल्य[0]
                    << ", Indentation:" << Indentation << "\n");
  स्वतः Partial = Ctx.Partials.find(Accessor_मूल्य[0]);
  जर (Partial != Ctx.Partials.end())
    renderPartial(CurrentCtx, OS, Partial->get_मूल्य());
}

शून्य ASTNode::renderVariable(स्थिरांक json::मूल्य &CurrentCtx,
                             MustacheOutputStream &OS) {
  स्वतः Lambda = Ctx.Lambdas.find(Accessor_मूल्य[0]);
  जर (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_मूल्य());
  } अन्यथा जर (स्थिरांक json::मूल्य *ContextPtr = findContext()) {
    EscapeStringStream ES(OS, Ctx.Escapes);
    toMustacheString(*ContextPtr, ES);
  }
}

शून्य ASTNode::renderUnescapeVariable(स्थिरांक json::मूल्य &CurrentCtx,
                                     MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_मूल्य[0]
                    << "\n");
  स्वतः Lambda = Ctx.Lambdas.find(Accessor_मूल्य[0]);
  जर (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_मूल्य());
  } अन्यथा जर (स्थिरांक json::मूल्य *ContextPtr = findContext()) {
    OS.suspendIndentation();
    toMustacheString(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

शून्य ASTNode::renderSection(स्थिरांक json::मूल्य &CurrentCtx,
                            MustacheOutputStream &OS) {
  स्वतः SectionLambda = Ctx.SectionLambdas.find(Accessor_मूल्य[0]);
  जर (SectionLambda != Ctx.SectionLambdas.end()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_मूल्य());
    परत;
  }

  स्थिरांक json::मूल्य *ContextPtr = findContext();
  जर (isContextFalsey(ContextPtr))
    परत;

  जर (स्थिरांक json::Array *Arr = ContextPtr->getAsArray()) {
    साठी (स्थिरांक json::मूल्य &V : *Arr)
      renderChild(V, OS);
    परत;
  }
  renderChild(*ContextPtr, OS);
}

शून्य ASTNode::renderInvertSection(स्थिरांक json::मूल्य &CurrentCtx,
                                  MustacheOutputStream &OS) {
  तार्किक IsLambda = Ctx.SectionLambdas.contains(Accessor_मूल्य[0]);
  स्थिरांक json::मूल्य *ContextPtr = findContext();
  जर (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

शून्य ASTNode::render(स्थिरांक llvm::json::मूल्य &Data, MustacheOutputStream &OS) {
  जर (Ty != मूळ_2 && Ty != Text && Accessor_मूल्य.empty())
    परत;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &Data;

  निवड (Ty) {
  स्थिती मूळ_2:
    render_मूळ(Data, OS);
    परत;
  स्थिती Text:
    renderText(OS);
    परत;
  स्थिती Partial:
    renderPartial(Data, OS);
    परत;
  स्थिती Variable:
    renderVariable(Data, OS);
    परत;
  स्थिती UnescapeVariable:
    renderUnescapeVariable(Data, OS);
    परत;
  स्थिती Section:
    renderSection(Data, OS);
    परत;
  स्थिती InvertSection:
    renderInvertSection(Data, OS);
    परत;
  }
  llvm_unreachable("Invalid ASTNode type");
}

स्थिरांक json::मूल्य *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  जर (Accessor_मूल्य.empty())
    परत nullptr;
  जर (Accessor_मूल्य[0] == ".")
    परत ParentContext;

  स्थिरांक json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = Accessor_मूल्य[0];
  ASTNode *CurrentParent = Parent;

  पर्यंत (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    जर (CurrentParent->Ty != मूळ_2) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      सुरूठेवा;
    }
    परत nullptr;
  }
  स्थिरांक json::मूल्य *Context = nullptr;
  साठी (स्वतः [Idx, Acc] : enumerate(Accessor_मूल्य)) {
    स्थिरांक json::मूल्य *Current_मूल्य = CurrentContext->get(Acc);
    जर (!Current_मूल्य)
      परत nullptr;
    जर (Idx < Accessor_मूल्य.size() - 1) {
      CurrentContext = Current_मूल्य->getAsObject();
      जर (!CurrentContext)
        परत nullptr;
    } अन्यथा {
      Context = Current_मूल्य;
    }
  }
  परत Context;
}

शून्य ASTNode::renderChild(स्थिरांक json::मूल्य &Contexts,
                          MustacheOutputStream &OS) {
  साठी (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

शून्य ASTNode::renderPartial(स्थिरांक json::मूल्य &Contexts,
                            MustacheOutputStream &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  AddIndentationStringStream IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

शून्य ASTNode::renderLambdas(स्थिरांक llvm::json::मूल्य &Contexts,
                            MustacheOutputStream &OS, Lambda &L) {
  json::मूल्य LambdaResult = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(LambdaResult, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_गाठ = P.parse();

  EscapeStringStream ES(OS, Ctx.Escapes);
  जर (Ty == Variable) {
    Lambda_गाठ->render(Contexts, ES);
    परत;
  }
  Lambda_गाठ->render(Contexts, OS);
}

शून्य ASTNode::renderSectionLambdas(स्थिरांक llvm::json::मूल्य &Contexts,
                                   MustacheOutputStream &OS, SectionLambda &L) {
  json::मूल्य Return = L(RawBody.str());
  जर (isFalsey(Return))
    परत;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_गाठ = P.parse();
  Lambda_गाठ->render(Contexts, OS);
}

शून्य Template::render(स्थिरांक llvm::json::मूल्य &Data, llvm::raw_ostream &OS) {
  RawMustacheOutputStream MOS(OS);
  वृक्ष->render(Data, MOS);
}

शून्य Template::registerPartial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_वृक्ष = P.parse();
  Ctx.Partials.घाला(std::make_pair(Name, Partial_वृक्ष));
}

शून्य Template::registerLambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

शून्य Template::registerLambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

शून्य Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  वृक्ष = P.parse();
  // The default behavior is to escape html entities.
  स्थिरांक EscapeMap HtmlEntities = {{'&', "&amp;"},
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

Template::~Template() = पूर्वनिर्धारित;

} // namespace llvm::mustache

#undef DEBUG_TYPE
