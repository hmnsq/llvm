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

स्थिर तार्किक isFalsey(ध्रुव json::मानम् &V) {
  प्रत्यागम V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().मानम्_2()) ||
         (V.getAsArray() && V.getAsArray()->empty());
}

स्थिर तार्किक isContextFalsey(ध्रुव json::मानम् *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  जइ (!V)
    प्रत्यागम सच्च;
  प्रत्यागम isFalsey(*V);
}

स्थिर शून्य splitAndTrim(StringRef Str, SmallVectorImpl<StringRef> &Tokens) {
  size_t CurrentPos = 0;
  जाव (CurrentPos < Str.size()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    जइ (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.size();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.slice(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t Start = Part.find_first_not_of(" \t\r\n");
    जइ (Start != StringRef::npos) {
      size_t End = Part.find_last_not_of(" \t\r\n");
      Tokens.प्रक्षिप_back(Part.slice(Start, End + 1));
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
  जइ (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    Tokens.प्रक्षिप_back(".");
  } अण्णहा {
    splitAndTrim(Str, Tokens);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.size());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  प्रत्यागम ArrayRef<StringRef>(ArenaTokens, Tokens.size());
}
} // namespace

नामाकाश llvm::mustache {

वर्ग MustacheOutputStream : सार्वजनिक raw_ostream {
सार्वजनिक:
  MustacheOutputStream() = स्वभाव;
  ~MustacheOutputStream() override = स्वभाव;

  आभास शून्य suspendIndentation() {}
  आभास शून्य resumeIndentation() {}

गुप्त:
  शून्य anchor() override;
};

शून्य MustacheOutputStream::anchor() {}

वर्ग RawMustacheOutputStream : सार्वजनिक MustacheOutputStream {
सार्वजनिक:
  RawMustacheOutputStream(raw_ostream &OS) : OS(OS) { SetUnbuffered(); }

गुप्त:
  raw_ostream &OS;

  शून्य write_impl(ध्रुव वर्ण *Ptr, size_t Size) override {
    OS.लिख(Ptr, Size);
  }
  uint64_t current_pos() ध्रुव override { प्रत्यागम OS.tell(); }
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
        Accessor_मानम्({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, वर्ण Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    जइ (TokenType == Type::Comment)
      प्रत्यागम;
    StringRef AccessorStr(एतत्->TokenBody);
    जइ (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_मानम् = splitMustacheString(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ध्रुव { प्रत्यागम Accessor_मानम्; }

  Type getType() ध्रुव { प्रत्यागम TokenType; }

  शून्य setIndentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() ध्रुव { प्रत्यागम Indentation; }

  स्थिर Type getTokenType(वर्ण Identifier) {
    विकल्प (Identifier) {
    पक्ष '#':
      प्रत्यागम Type::SectionOpen;
    पक्ष '/':
      प्रत्यागम Type::SectionClose;
    पक्ष '^':
      प्रत्यागम Type::InvertSectionOpen;
    पक्ष '!':
      प्रत्यागम Type::Comment;
    पक्ष '>':
      प्रत्यागम Type::Partial;
    पक्ष '&':
      प्रत्यागम Type::UnescapeVariable;
    पक्ष '=':
      प्रत्यागम Type::SetDelimiter;
    स्वभाव:
      प्रत्यागम Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_मानम्;
  size_t Indentation;
};

using EscapeMap = DenseMap<वर्ण, std::string>;

वर्ग ASTNode : सार्वजनिक ilist_ग्रन्थिः<ASTNode> {
सार्वजनिक:
  गणना Type {
    मूलम्_2,
    Text,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::मूलम्_2), Parent(nullptr), ParentContext(nullptr) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *Parent)
      : Ctx(Ctx), Ty(Type::Text), Body(Body), Parent(Parent),
        ParentContext(nullptr) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *Parent)
      : Ctx(Ctx), Ty(Ty), Parent(Parent), Accessor_मानम्(Accessor),
        ParentContext(nullptr) {}

  शून्य addChild(AstPtr Child) { Children.प्रक्षिप_back(Child); };

  शून्य setRawBody(StringRef NewBody) { RawBody = NewBody; };

  शून्य setIndentation(size_t NewIndentation) { Indentation = NewIndentation; };

  शून्य render(ध्रुव llvm::json::मानम् &Data, MustacheOutputStream &OS);

गुप्त:
  शून्य renderLambdas(ध्रुव llvm::json::मानम् &Contexts,
                     MustacheOutputStream &OS, Lambda &L);

  शून्य renderSectionLambdas(ध्रुव llvm::json::मानम् &Contexts,
                            MustacheOutputStream &OS, SectionLambda &L);

  शून्य renderPartial(ध्रुव llvm::json::मानम् &Contexts,
                     MustacheOutputStream &OS, ASTNode *Partial);

  शून्य renderChild(ध्रुव llvm::json::मानम् &Context, MustacheOutputStream &OS);

  ध्रुव llvm::json::मानम् *findContext();

  शून्य render_मूलम्(ध्रुव json::मानम् &CurrentCtx, MustacheOutputStream &OS);
  शून्य renderText(MustacheOutputStream &OS);
  शून्य renderPartial(ध्रुव json::मानम् &CurrentCtx, MustacheOutputStream &OS);
  शून्य renderVariable(ध्रुव json::मानम् &CurrentCtx, MustacheOutputStream &OS);
  शून्य renderUnescapeVariable(ध्रुव json::मानम् &CurrentCtx,
                              MustacheOutputStream &OS);
  शून्य renderSection(ध्रुव json::मानम् &CurrentCtx, MustacheOutputStream &OS);
  शून्य renderInvertSection(ध्रुव json::मानम् &CurrentCtx,
                           MustacheOutputStream &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  ध्रुव ArrayRef<StringRef> Accessor_मानम्;
  ध्रुव llvm::json::मानम् *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
स्थिर AstPtr create_मूलम्_ग्रन्थिः(MustacheContext &Ctx) {
  प्रत्यागम नूतन (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

स्थिर AstPtr create_ग्रन्थिः(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  प्रत्यागम नूतन (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

स्थिर AstPtr create_Text_ग्रन्थिः(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  प्रत्यागम नूतन (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
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
स्थिर तार्किक hasTextBehind(size_t Idx, ध्रुव ArrayRef<Token> &Tokens) {
  जइ (Idx == 0)
    प्रत्यागम सच्च;

  size_t PrevIdx = Idx - 1;
  जइ (Tokens[PrevIdx].getType() != Token::Type::Text)
    प्रत्यागम सच्च;

  ध्रुव Token &PrevToken = Tokens[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  प्रत्यागम !TokenBody.ends_with("\n") && !(TokenBody.empty() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
स्थिर तार्किक hasTextAhead(size_t Idx, ध्रुव ArrayRef<Token> &Tokens) {
  जइ (Idx >= Tokens.size() - 1)
    प्रत्यागम सच्च;

  size_t NextIdx = Idx + 1;
  जइ (Tokens[NextIdx].getType() != Token::Type::Text)
    प्रत्यागम सच्च;

  ध्रुव Token &NextToken = Tokens[NextIdx];
  StringRef TokenBody = StringRef(NextToken.RawBody).ltrim(" ");
  प्रत्यागम !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

स्थिर तार्किक requiresCleanUp(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  प्रत्यागम T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::SetDelimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
स्थिर शून्य stripTokenAhead(SmallVectorImpl<Token> &Tokens, size_t Idx) {
  Token &NextToken = Tokens[Idx + 1];
  StringRef NextTokenBody = NextToken.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  जइ (NextTokenBody.starts_with("\r\n"))
    NextToken.TokenBody = NextTokenBody.substr(2);
  अण्णहा जइ (NextTokenBody.starts_with("\n"))
    NextToken.TokenBody = NextTokenBody.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
शून्य stripTokenBefore(SmallVectorImpl<Token> &Tokens, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = Tokens[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.size() - Unindented.size();
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
  size_t StartPosition = StringRef::npos;
};

[[maybe_unused]] स्थिर ध्रुव वर्ण *tagKindToString(Tag::Kind K) {
  विकल्प (K) {
  पक्ष Tag::Kind::None:
    प्रत्यागम "None";
  पक्ष Tag::Kind::Normal:
    प्रत्यागम "Normal";
  पक्ष Tag::Kind::Triple:
    प्रत्यागम "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_unused]] स्थिर ध्रुव वर्ण *jsonKindToString(json::मानम्::Kind K) {
  विकल्प (K) {
  पक्ष json::मानम्::Kind::Null:
    प्रत्यागम "JSON_KIND_NULL";
  पक्ष json::मानम्::Kind::Boolean:
    प्रत्यागम "JSON_KIND_BOOLEAN";
  पक्ष json::मानम्::Kind::सङ्ख्या:
    प्रत्यागम "JSON_KIND_NUMBER";
  पक्ष json::मानम्::Kind::String:
    प्रत्यागम "JSON_KIND_STRING";
  पक्ष json::मानम्::Kind::Array:
    प्रत्यागम "JSON_KIND_ARRAY";
  पक्ष json::मानम्::Kind::Object:
    प्रत्यागम "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
स्थिर SmallVector<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  SmallVector<Token> Tokens;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t TextStart = 0;

  ध्रुव StringLiteral TripleOpen("{{{");
  ध्रुव StringLiteral TripleClose("}}}");

  जाव (Cursor < Template.size()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef TagOpen, TagClose;
    Tag::Kind Kind;

    // Determine which tag we've encountered.
    जइ (TemplateSuffix.starts_with(TripleOpen)) {
      Kind = Tag::Kind::Triple;
      TagOpen = TripleOpen;
      TagClose = TripleClose;
    } अण्णहा जइ (TemplateSuffix.starts_with(Open)) {
      Kind = Tag::Kind::Normal;
      TagOpen = Open;
      TagClose = Close;
    } अण्णहा {
      // Not at a tag, continue scanning.
      ++Cursor;
      अनुवर्त;
    }

    // Found a tag, first add the preceding text.
    जइ (Cursor > TextStart)
      Tokens.emplace_back(Template.slice(TextStart, Cursor));

    // Find the closing tag.
    size_t EndPos = Template.find(TagClose, Cursor + TagOpen.size());
    जइ (EndPos == StringRef::npos) {
      // No closing tag, the rest is text.
      Tokens.emplace_back(Template.substr(Cursor));
      TextStart = Cursor = Template.size();
      भङ्ग;
    }

    // Extract tag content and full match.
    size_t ContentStart = Cursor + TagOpen.size();
    StringRef Content = Template.substr(ContentStart, EndPos - ContentStart);
    StringRef FullMatch =
        Template.substr(Cursor, (EndPos + TagClose.size()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << FullMatch << ", Content: " << Content
                      << ", Kind: " << tagKindToString(Kind) << "\n");
    जइ (Kind == Tag::Kind::Triple) {
      Tokens.emplace_back(FullMatch, Ctx.Saver.save("&" + Content), '&', Ctx);
    } अण्णहा { // Normal Tag
      StringRef Interpolated = Content;
      जइ (!Interpolated.trim().starts_with("=")) {
        वर्ण Front = Interpolated.empty() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_back(FullMatch, Interpolated, Front, Ctx);
      } अण्णहा { // Set Delimiter
        Tokens.emplace_back(FullMatch, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](वर्ण C) { प्रत्यागम C == '='; });
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
  जइ (TextStart < Template.size())
    Tokens.emplace_back(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  size_t LastIdx = Tokens.size() - 1;
  प्रति (size_t Idx = 0, End = Tokens.size(); Idx < End; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    जइ (!requiresCleanUp(CurrentType))
      अनुवर्त;

    तार्किक HasTextBehind = hasTextBehind(Idx, Tokens);
    तार्किक HasTextAhead = hasTextAhead(Idx, Tokens);

    जइ ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    जइ ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == LastIdx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  प्रत्यागम Tokens;
}

// Custom stream to escape strings.
वर्ग EscapeStringStream : सार्वजनिक MustacheOutputStream {
सार्वजनिक:
  explicit EscapeStringStream(llvm::raw_ostream &WrappedStream,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.कुञ्चिकाः().begin(), Escape.कुञ्चिकाः().end()),
        WrappedStream(WrappedStream) {
    SetUnbuffered();
  }

रक्षित:
  शून्य write_impl(ध्रुव वर्ण *Ptr, size_t Size) override {
    StringRef Data(Ptr, Size);
    size_t Start = 0;
    जाव (Start < Size) {
      // Find the next character that needs to be escaped.
      size_t Next = Data.find_first_of(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      जइ (Next == StringRef::npos) {
        WrappedStream << Data.substr(Start);
        प्रत्यागम;
      }

      // Write the chunk of text before the escapable character.
      जइ (Next > Start)
        WrappedStream << Data.substr(Start, Next - Start);

      // Look up and write the escaped version of the character.
      WrappedStream << Escape[Data[Next]];
      Start = Next + 1;
    }
  }

  uint64_t current_pos() ध्रुव override { प्रत्यागम WrappedStream.tell(); }

गुप्त:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &WrappedStream;
};

// Custom stream to add indentation used to for rendering partials.
वर्ग AddIndentationStringStream : सार्वजनिक MustacheOutputStream {
सार्वजनिक:
  explicit AddIndentationStringStream(raw_ostream &WrappedStream,
                                      size_t Indentation)
      : Indentation(Indentation), WrappedStream(WrappedStream),
        NeedsIndent(सच्च), IsSuspended(मिच्छ) {
    SetUnbuffered();
  }

  शून्य suspendIndentation() override { IsSuspended = सच्च; }
  शून्य resumeIndentation() override { IsSuspended = मिच्छ; }

रक्षित:
  शून्य write_impl(ध्रुव वर्ण *Ptr, size_t Size) override {
    llvm::StringRef Data(Ptr, Size);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    प्रति (वर्ण C : Data) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      जइ (NeedsIndent && C != '\n') {
        WrappedStream << Indent;
        NeedsIndent = मिच्छ;
      }
      WrappedStream << C;
      जइ (C == '\n' && !IsSuspended)
        NeedsIndent = सच्च;
    }
  }

  uint64_t current_pos() ध्रुव override { प्रत्यागम WrappedStream.tell(); }

गुप्त:
  size_t Indentation;
  raw_ostream &WrappedStream;
  तार्किक NeedsIndent;
  तार्किक IsSuspended;
};

वर्ग Parser {
सार्वजनिक:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

गुप्त:
  शून्य parseMustache(ASTNode *Parent);
  शून्य parseSection(ASTNode *Parent, ASTNode::Type Ty, ध्रुव Accessor &A);

  MustacheContext &Ctx;
  SmallVector<Token> Tokens;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

शून्य Parser::parseSection(ASTNode *Parent, ASTNode::Type Ty,
                          ध्रुव Accessor &A) {
  AstPtr Current_ग्रन्थिः = create_ग्रन्थिः(Ctx, Ty, A, Parent);
  size_t Start = CurrentPtr;
  parseMustache(Current_ग्रन्थिः);
  ध्रुव size_t End = CurrentPtr - 1;

  size_t RawBodySize = 0;
  प्रति (size_t I = Start; I < End; ++I)
    RawBodySize += Tokens[I].RawBody.size();

  SmallString<128> RawBody;
  RawBody.reserve(RawBodySize);
  प्रति (std::size_t I = Start; I < End; ++I)
    RawBody += Tokens[I].RawBody;

  Current_ग्रन्थिः->setRawBody(Ctx.Saver.save(StringRef(RawBody)));
  Parent->addChild(Current_ग्रन्थिः);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr मूलम्_ग्रन्थिः = create_मूलम्_ग्रन्थिः(Ctx);
  parseMustache(मूलम्_ग्रन्थिः);
  प्रत्यागम मूलम्_ग्रन्थिः;
}

शून्य Parser::parseMustache(ASTNode *Parent) {

  जाव (CurrentPtr < Tokens.size()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_ग्रन्थिः;

    विकल्प (CurrentToken.getType()) {
    पक्ष Token::Type::Text: {
      Current_ग्रन्थिः = create_Text_ग्रन्थिः(Ctx, CurrentToken.TokenBody, Parent);
      Parent->addChild(Current_ग्रन्थिः);
      भङ्ग;
    }
    पक्ष Token::Type::Variable: {
      Current_ग्रन्थिः = create_ग्रन्थिः(Ctx, ASTNode::Variable, A, Parent);
      Parent->addChild(Current_ग्रन्थिः);
      भङ्ग;
    }
    पक्ष Token::Type::UnescapeVariable: {
      Current_ग्रन्थिः = create_ग्रन्थिः(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->addChild(Current_ग्रन्थिः);
      भङ्ग;
    }
    पक्ष Token::Type::Partial: {
      Current_ग्रन्थिः = create_ग्रन्थिः(Ctx, ASTNode::Partial, A, Parent);
      Current_ग्रन्थिः->setIndentation(CurrentToken.getIndentation());
      Parent->addChild(Current_ग्रन्थिः);
      भङ्ग;
    }
    पक्ष Token::Type::SectionOpen: {
      parseSection(Parent, ASTNode::Section, A);
      भङ्ग;
    }
    पक्ष Token::Type::InvertSectionOpen: {
      parseSection(Parent, ASTNode::InvertSection, A);
      भङ्ग;
    }
    पक्ष Token::Type::Comment:
    पक्ष Token::Type::SetDelimiter:
      भङ्ग;
    पक्ष Token::Type::SectionClose:
      प्रत्यागम;
    }
  }
}
स्थिर शून्य toMustacheString(ध्रुव json::मानम् &Data, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << jsonKindToString(Data.kind()) << ", Data: " << Data
                    << "\n");
  विकल्प (Data.kind()) {
  पक्ष json::मानम्::Null:
    प्रत्यागम;
  पक्ष json::मानम्::सङ्ख्या: {
    auto अङ्कः = *Data.get_As_सङ्ख्या();
    std::ostringstream SS;
    SS << अङ्कः;
    OS << SS.str();
    प्रत्यागम;
  }
  पक्ष json::मानम्::String: {
    OS << *Data.getAsString();
    प्रत्यागम;
  }

  पक्ष json::मानम्::Array: {
    auto Arr = *Data.getAsArray();
    जइ (Arr.empty())
      प्रत्यागम;
    [[fallthrough]];
  }
  पक्ष json::मानम्::Object:
  पक्ष json::मानम्::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.मानम्_2(Data);
    भङ्ग;
  }
  }
}

शून्य ASTNode::render_मूलम्(ध्रुव json::मानम् &CurrentCtx,
                         MustacheOutputStream &OS) {
  renderChild(CurrentCtx, OS);
}

शून्य ASTNode::renderText(MustacheOutputStream &OS) { OS << Body; }

शून्य ASTNode::renderPartial(ध्रुव json::मानम् &CurrentCtx,
                            MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_मानम्[0]
                    << ", Indentation:" << Indentation << "\n");
  auto Partial = Ctx.Partials.find(Accessor_मानम्[0]);
  जइ (Partial != Ctx.Partials.end())
    renderPartial(CurrentCtx, OS, Partial->get_मानम्());
}

शून्य ASTNode::renderVariable(ध्रुव json::मानम् &CurrentCtx,
                             MustacheOutputStream &OS) {
  auto Lambda = Ctx.Lambdas.find(Accessor_मानम्[0]);
  जइ (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_मानम्());
  } अण्णहा जइ (ध्रुव json::मानम् *ContextPtr = findContext()) {
    EscapeStringStream ES(OS, Ctx.Escapes);
    toMustacheString(*ContextPtr, ES);
  }
}

शून्य ASTNode::renderUnescapeVariable(ध्रुव json::मानम् &CurrentCtx,
                                     MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_मानम्[0]
                    << "\n");
  auto Lambda = Ctx.Lambdas.find(Accessor_मानम्[0]);
  जइ (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_मानम्());
  } अण्णहा जइ (ध्रुव json::मानम् *ContextPtr = findContext()) {
    OS.suspendIndentation();
    toMustacheString(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

शून्य ASTNode::renderSection(ध्रुव json::मानम् &CurrentCtx,
                            MustacheOutputStream &OS) {
  auto SectionLambda = Ctx.SectionLambdas.find(Accessor_मानम्[0]);
  जइ (SectionLambda != Ctx.SectionLambdas.end()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_मानम्());
    प्रत्यागम;
  }

  ध्रुव json::मानम् *ContextPtr = findContext();
  जइ (isContextFalsey(ContextPtr))
    प्रत्यागम;

  जइ (ध्रुव json::Array *Arr = ContextPtr->getAsArray()) {
    प्रति (ध्रुव json::मानम् &V : *Arr)
      renderChild(V, OS);
    प्रत्यागम;
  }
  renderChild(*ContextPtr, OS);
}

शून्य ASTNode::renderInvertSection(ध्रुव json::मानम् &CurrentCtx,
                                  MustacheOutputStream &OS) {
  तार्किक IsLambda = Ctx.SectionLambdas.contains(Accessor_मानम्[0]);
  ध्रुव json::मानम् *ContextPtr = findContext();
  जइ (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

शून्य ASTNode::render(ध्रुव llvm::json::मानम् &Data, MustacheOutputStream &OS) {
  जइ (Ty != मूलम्_2 && Ty != Text && Accessor_मानम्.empty())
    प्रत्यागम;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &Data;

  विकल्प (Ty) {
  पक्ष मूलम्_2:
    render_मूलम्(Data, OS);
    प्रत्यागम;
  पक्ष Text:
    renderText(OS);
    प्रत्यागम;
  पक्ष Partial:
    renderPartial(Data, OS);
    प्रत्यागम;
  पक्ष Variable:
    renderVariable(Data, OS);
    प्रत्यागम;
  पक्ष UnescapeVariable:
    renderUnescapeVariable(Data, OS);
    प्रत्यागम;
  पक्ष Section:
    renderSection(Data, OS);
    प्रत्यागम;
  पक्ष InvertSection:
    renderInvertSection(Data, OS);
    प्रत्यागम;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ध्रुव json::मानम् *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  जइ (Accessor_मानम्.empty())
    प्रत्यागम nullptr;
  जइ (Accessor_मानम्[0] == ".")
    प्रत्यागम ParentContext;

  ध्रुव json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = Accessor_मानम्[0];
  ASTNode *CurrentParent = Parent;

  जाव (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    जइ (CurrentParent->Ty != मूलम्_2) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      अनुवर्त;
    }
    प्रत्यागम nullptr;
  }
  ध्रुव json::मानम् *Context = nullptr;
  प्रति (auto [Idx, Acc] : enumerate(Accessor_मानम्)) {
    ध्रुव json::मानम् *Current_मानम् = CurrentContext->get(Acc);
    जइ (!Current_मानम्)
      प्रत्यागम nullptr;
    जइ (Idx < Accessor_मानम्.size() - 1) {
      CurrentContext = Current_मानम्->getAsObject();
      जइ (!CurrentContext)
        प्रत्यागम nullptr;
    } अण्णहा {
      Context = Current_मानम्;
    }
  }
  प्रत्यागम Context;
}

शून्य ASTNode::renderChild(ध्रुव json::मानम् &Contexts,
                          MustacheOutputStream &OS) {
  प्रति (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

शून्य ASTNode::renderPartial(ध्रुव json::मानम् &Contexts,
                            MustacheOutputStream &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  AddIndentationStringStream IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

शून्य ASTNode::renderLambdas(ध्रुव llvm::json::मानम् &Contexts,
                            MustacheOutputStream &OS, Lambda &L) {
  json::मानम् LambdaResult = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(LambdaResult, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ग्रन्थिः = P.parse();

  EscapeStringStream ES(OS, Ctx.Escapes);
  जइ (Ty == Variable) {
    Lambda_ग्रन्थिः->render(Contexts, ES);
    प्रत्यागम;
  }
  Lambda_ग्रन्थिः->render(Contexts, OS);
}

शून्य ASTNode::renderSectionLambdas(ध्रुव llvm::json::मानम् &Contexts,
                                   MustacheOutputStream &OS, SectionLambda &L) {
  json::मानम् Return = L(RawBody.str());
  जइ (isFalsey(Return))
    प्रत्यागम;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ग्रन्थिः = P.parse();
  Lambda_ग्रन्थिः->render(Contexts, OS);
}

शून्य Template::render(ध्रुव llvm::json::मानम् &Data, llvm::raw_ostream &OS) {
  RawMustacheOutputStream MOS(OS);
  वृक्षः->render(Data, MOS);
}

शून्य Template::registerPartial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_वृक्षः = P.parse();
  Ctx.Partials.निवेशय(std::make_pair(Name, Partial_वृक्षः));
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
  वृक्षः = P.parse();
  // The default behavior is to escape html entities.
  ध्रुव EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), वृक्षः(Other.वृक्षः) {
  Other.वृक्षः = nullptr;
}

Template::~Template() = स्वभाव;

} // namespace llvm::mustache

#undef DEBUG_TYPE
