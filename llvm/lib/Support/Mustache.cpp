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

स्थिर तार्किक isFalsey(अचर json::मान &V) {
  फर्काऊ V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().मान_2()) ||
         (V.getAsArray() && V.getAsArray()->empty());
}

स्थिर तार्किक isContextFalsey(अचर json::मान *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  यदि (!V)
    फर्काऊ सत्य;
  फर्काऊ isFalsey(*V);
}

स्थिर शून्य splitAndTrim(StringRef Str, SmallVectorImpl<StringRef> &Tokens) {
  माप_प्रकार CurrentPos = 0;
  जबसम्म (CurrentPos < Str.size()) {
    // Find the next delimiter.
    माप_प्रकार DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    यदि (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.size();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.slice(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    माप_प्रकार Start = Part.find_first_not_of(" \t\r\n");
    यदि (Start != StringRef::npos) {
      माप_प्रकार End = Part.find_last_not_of(" \t\r\n");
      Tokens.थप_back(Part.slice(Start, End + 1));
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
  यदि (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    Tokens.थप_back(".");
  } अन्यथा {
    splitAndTrim(Str, Tokens);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.size());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  फर्काऊ ArrayRef<StringRef>(ArenaTokens, Tokens.size());
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

  शून्य write_impl(अचर वर्ण *Ptr, माप_प्रकार Size) override {
    OS.लिख(Ptr, Size);
  }
  uint64_t current_pos() अचर override { फर्काऊ OS.tell(); }
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
        Accessor_मान({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, वर्ण Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    यदि (TokenType == Type::Comment)
      फर्काऊ;
    StringRef AccessorStr(एतत्->TokenBody);
    यदि (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_मान = splitMustacheString(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() अचर { फर्काऊ Accessor_मान; }

  Type getType() अचर { फर्काऊ TokenType; }

  शून्य setIndentation(माप_प्रकार NewIndentation) { Indentation = NewIndentation; }

  माप_प्रकार getIndentation() अचर { फर्काऊ Indentation; }

  स्थिर Type getTokenType(वर्ण Identifier) {
    विकल्प (Identifier) {
    पक्ष '#':
      फर्काऊ Type::SectionOpen;
    पक्ष '/':
      फर्काऊ Type::SectionClose;
    पक्ष '^':
      फर्काऊ Type::InvertSectionOpen;
    पक्ष '!':
      फर्काऊ Type::Comment;
    पक्ष '>':
      फर्काऊ Type::Partial;
    पक्ष '&':
      फर्काऊ Type::UnescapeVariable;
    पक्ष '=':
      फर्काऊ Type::SetDelimiter;
    स्वभाव:
      फर्काऊ Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_मान;
  माप_प्रकार Indentation;
};

using EscapeMap = DenseMap<वर्ण, std::string>;

वर्ग ASTNode : सार्वजनिक ilist_गाँठो<ASTNode> {
सार्वजनिक:
  गणना Type {
    जरा_2,
    Text,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::जरा_2), Parent(nullptr), ParentContext(nullptr) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *Parent)
      : Ctx(Ctx), Ty(Type::Text), Body(Body), Parent(Parent),
        ParentContext(nullptr) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *Parent)
      : Ctx(Ctx), Ty(Ty), Parent(Parent), Accessor_मान(Accessor),
        ParentContext(nullptr) {}

  शून्य addChild(AstPtr Child) { Children.थप_back(Child); };

  शून्य setRawBody(StringRef NewBody) { RawBody = NewBody; };

  शून्य setIndentation(माप_प्रकार NewIndentation) { Indentation = NewIndentation; };

  शून्य render(अचर llvm::json::मान &Data, MustacheOutputStream &OS);

गुप्त:
  शून्य renderLambdas(अचर llvm::json::मान &Contexts,
                     MustacheOutputStream &OS, Lambda &L);

  शून्य renderSectionLambdas(अचर llvm::json::मान &Contexts,
                            MustacheOutputStream &OS, SectionLambda &L);

  शून्य renderPartial(अचर llvm::json::मान &Contexts,
                     MustacheOutputStream &OS, ASTNode *Partial);

  शून्य renderChild(अचर llvm::json::मान &Context, MustacheOutputStream &OS);

  अचर llvm::json::मान *findContext();

  शून्य render_जरा(अचर json::मान &CurrentCtx, MustacheOutputStream &OS);
  शून्य renderText(MustacheOutputStream &OS);
  शून्य renderPartial(अचर json::मान &CurrentCtx, MustacheOutputStream &OS);
  शून्य renderVariable(अचर json::मान &CurrentCtx, MustacheOutputStream &OS);
  शून्य renderUnescapeVariable(अचर json::मान &CurrentCtx,
                              MustacheOutputStream &OS);
  शून्य renderSection(अचर json::मान &CurrentCtx, MustacheOutputStream &OS);
  शून्य renderInvertSection(अचर json::मान &CurrentCtx,
                           MustacheOutputStream &OS);

  MustacheContext &Ctx;
  Type Ty;
  माप_प्रकार Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  अचर ArrayRef<StringRef> Accessor_मान;
  अचर llvm::json::मान *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
स्थिर AstPtr create_जरा_गाँठो(MustacheContext &Ctx) {
  फर्काऊ नूतन (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

स्थिर AstPtr create_गाँठो(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  फर्काऊ नूतन (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

स्थिर AstPtr create_Text_गाँठो(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  फर्काऊ नूतन (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
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
स्थिर तार्किक hasTextBehind(माप_प्रकार Idx, अचर ArrayRef<Token> &Tokens) {
  यदि (Idx == 0)
    फर्काऊ सत्य;

  माप_प्रकार PrevIdx = Idx - 1;
  यदि (Tokens[PrevIdx].getType() != Token::Type::Text)
    फर्काऊ सत्य;

  अचर Token &PrevToken = Tokens[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  फर्काऊ !TokenBody.ends_with("\n") && !(TokenBody.empty() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
स्थिर तार्किक hasTextAhead(माप_प्रकार Idx, अचर ArrayRef<Token> &Tokens) {
  यदि (Idx >= Tokens.size() - 1)
    फर्काऊ सत्य;

  माप_प्रकार NextIdx = Idx + 1;
  यदि (Tokens[NextIdx].getType() != Token::Type::Text)
    फर्काऊ सत्य;

  अचर Token &NextToken = Tokens[NextIdx];
  StringRef TokenBody = StringRef(NextToken.RawBody).ltrim(" ");
  फर्काऊ !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

स्थिर तार्किक requiresCleanUp(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  फर्काऊ T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
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
  यदि (NextTokenBody.starts_with("\r\n"))
    NextToken.TokenBody = NextTokenBody.substr(2);
  अन्यथा यदि (NextTokenBody.starts_with("\n"))
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

[[maybe_unused]] स्थिर अचर वर्ण *tagKindToString(Tag::Kind K) {
  विकल्प (K) {
  पक्ष Tag::Kind::None:
    फर्काऊ "None";
  पक्ष Tag::Kind::Normal:
    फर्काऊ "Normal";
  पक्ष Tag::Kind::Triple:
    फर्काऊ "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_unused]] स्थिर अचर वर्ण *jsonKindToString(json::मान::Kind K) {
  विकल्प (K) {
  पक्ष json::मान::Kind::Null:
    फर्काऊ "JSON_KIND_NULL";
  पक्ष json::मान::Kind::Boolean:
    फर्काऊ "JSON_KIND_BOOLEAN";
  पक्ष json::मान::Kind::सङ्ख्या:
    फर्काऊ "JSON_KIND_NUMBER";
  पक्ष json::मान::Kind::String:
    फर्काऊ "JSON_KIND_STRING";
  पक्ष json::मान::Kind::Array:
    फर्काऊ "JSON_KIND_ARRAY";
  पक्ष json::मान::Kind::Object:
    फर्काऊ "JSON_KIND_OBJECT";
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

  अचर StringLiteral TripleOpen("{{{");
  अचर StringLiteral TripleClose("}}}");

  जबसम्म (Cursor < Template.size()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef TagOpen, TagClose;
    Tag::Kind Kind;

    // Determine which tag we've encountered.
    यदि (TemplateSuffix.starts_with(TripleOpen)) {
      Kind = Tag::Kind::Triple;
      TagOpen = TripleOpen;
      TagClose = TripleClose;
    } अन्यथा यदि (TemplateSuffix.starts_with(Open)) {
      Kind = Tag::Kind::Normal;
      TagOpen = Open;
      TagClose = Close;
    } अन्यथा {
      // Not at a tag, continue scanning.
      ++Cursor;
      जारी;
    }

    // Found a tag, first add the preceding text.
    यदि (Cursor > TextStart)
      Tokens.emplace_back(Template.slice(TextStart, Cursor));

    // Find the closing tag.
    माप_प्रकार EndPos = Template.find(TagClose, Cursor + TagOpen.size());
    यदि (EndPos == StringRef::npos) {
      // No closing tag, the rest is text.
      Tokens.emplace_back(Template.substr(Cursor));
      TextStart = Cursor = Template.size();
      रोक;
    }

    // Extract tag content and full match.
    माप_प्रकार ContentStart = Cursor + TagOpen.size();
    StringRef Content = Template.substr(ContentStart, EndPos - ContentStart);
    StringRef FullMatch =
        Template.substr(Cursor, (EndPos + TagClose.size()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << FullMatch << ", Content: " << Content
                      << ", Kind: " << tagKindToString(Kind) << "\n");
    यदि (Kind == Tag::Kind::Triple) {
      Tokens.emplace_back(FullMatch, Ctx.Saver.save("&" + Content), '&', Ctx);
    } अन्यथा { // Normal Tag
      StringRef Interpolated = Content;
      यदि (!Interpolated.trim().starts_with("=")) {
        वर्ण Front = Interpolated.empty() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_back(FullMatch, Interpolated, Front, Ctx);
      } अन्यथा { // Set Delimiter
        Tokens.emplace_back(FullMatch, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](वर्ण C) { फर्काऊ C == '='; });
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
  यदि (TextStart < Template.size())
    Tokens.emplace_back(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  माप_प्रकार LastIdx = Tokens.size() - 1;
  लागि (माप_प्रकार Idx = 0, End = Tokens.size(); Idx < End; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    यदि (!requiresCleanUp(CurrentType))
      जारी;

    तार्किक HasTextBehind = hasTextBehind(Idx, Tokens);
    तार्किक HasTextAhead = hasTextAhead(Idx, Tokens);

    यदि ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    यदि ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == LastIdx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  फर्काऊ Tokens;
}

// Custom stream to escape strings.
वर्ग EscapeStringStream : सार्वजनिक MustacheOutputStream {
सार्वजनिक:
  explicit EscapeStringStream(llvm::raw_ostream &WrappedStream,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.साँचोहरू().begin(), Escape.साँचोहरू().end()),
        WrappedStream(WrappedStream) {
    SetUnbuffered();
  }

रक्षित:
  शून्य write_impl(अचर वर्ण *Ptr, माप_प्रकार Size) override {
    StringRef Data(Ptr, Size);
    माप_प्रकार Start = 0;
    जबसम्म (Start < Size) {
      // Find the next character that needs to be escaped.
      माप_प्रकार Next = Data.find_first_of(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      यदि (Next == StringRef::npos) {
        WrappedStream << Data.substr(Start);
        फर्काऊ;
      }

      // Write the chunk of text before the escapable character.
      यदि (Next > Start)
        WrappedStream << Data.substr(Start, Next - Start);

      // Look up and write the escaped version of the character.
      WrappedStream << Escape[Data[Next]];
      Start = Next + 1;
    }
  }

  uint64_t current_pos() अचर override { फर्काऊ WrappedStream.tell(); }

गुप्त:
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

रक्षित:
  शून्य write_impl(अचर वर्ण *Ptr, माप_प्रकार Size) override {
    llvm::StringRef Data(Ptr, Size);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    लागि (वर्ण C : Data) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      यदि (NeedsIndent && C != '\n') {
        WrappedStream << Indent;
        NeedsIndent = असत्य;
      }
      WrappedStream << C;
      यदि (C == '\n' && !IsSuspended)
        NeedsIndent = सत्य;
    }
  }

  uint64_t current_pos() अचर override { फर्काऊ WrappedStream.tell(); }

गुप्त:
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

गुप्त:
  शून्य parseMustache(ASTNode *Parent);
  शून्य parseSection(ASTNode *Parent, ASTNode::Type Ty, अचर Accessor &A);

  MustacheContext &Ctx;
  SmallVector<Token> Tokens;
  माप_प्रकार CurrentPtr;
  StringRef TemplateStr;
};

शून्य Parser::parseSection(ASTNode *Parent, ASTNode::Type Ty,
                          अचर Accessor &A) {
  AstPtr Current_गाँठो = create_गाँठो(Ctx, Ty, A, Parent);
  माप_प्रकार Start = CurrentPtr;
  parseMustache(Current_गाँठो);
  अचर माप_प्रकार End = CurrentPtr - 1;

  माप_प्रकार RawBodySize = 0;
  लागि (माप_प्रकार I = Start; I < End; ++I)
    RawBodySize += Tokens[I].RawBody.size();

  SmallString<128> RawBody;
  RawBody.reserve(RawBodySize);
  लागि (std::माप_प्रकार I = Start; I < End; ++I)
    RawBody += Tokens[I].RawBody;

  Current_गाँठो->setRawBody(Ctx.Saver.save(StringRef(RawBody)));
  Parent->addChild(Current_गाँठो);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr जरा_गाँठो = create_जरा_गाँठो(Ctx);
  parseMustache(जरा_गाँठो);
  फर्काऊ जरा_गाँठो;
}

शून्य Parser::parseMustache(ASTNode *Parent) {

  जबसम्म (CurrentPtr < Tokens.size()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_गाँठो;

    विकल्प (CurrentToken.getType()) {
    पक्ष Token::Type::Text: {
      Current_गाँठो = create_Text_गाँठो(Ctx, CurrentToken.TokenBody, Parent);
      Parent->addChild(Current_गाँठो);
      रोक;
    }
    पक्ष Token::Type::Variable: {
      Current_गाँठो = create_गाँठो(Ctx, ASTNode::Variable, A, Parent);
      Parent->addChild(Current_गाँठो);
      रोक;
    }
    पक्ष Token::Type::UnescapeVariable: {
      Current_गाँठो = create_गाँठो(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->addChild(Current_गाँठो);
      रोक;
    }
    पक्ष Token::Type::Partial: {
      Current_गाँठो = create_गाँठो(Ctx, ASTNode::Partial, A, Parent);
      Current_गाँठो->setIndentation(CurrentToken.getIndentation());
      Parent->addChild(Current_गाँठो);
      रोक;
    }
    पक्ष Token::Type::SectionOpen: {
      parseSection(Parent, ASTNode::Section, A);
      रोक;
    }
    पक्ष Token::Type::InvertSectionOpen: {
      parseSection(Parent, ASTNode::InvertSection, A);
      रोक;
    }
    पक्ष Token::Type::Comment:
    पक्ष Token::Type::SetDelimiter:
      रोक;
    पक्ष Token::Type::SectionClose:
      फर्काऊ;
    }
  }
}
स्थिर शून्य toMustacheString(अचर json::मान &Data, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << jsonKindToString(Data.kind()) << ", Data: " << Data
                    << "\n");
  विकल्प (Data.kind()) {
  पक्ष json::मान::Null:
    फर्काऊ;
  पक्ष json::मान::सङ्ख्या: {
    स्वतः अङ्क = *Data.get_As_सङ्ख्या();
    std::ostringstream SS;
    SS << अङ्क;
    OS << SS.str();
    फर्काऊ;
  }
  पक्ष json::मान::String: {
    OS << *Data.getAsString();
    फर्काऊ;
  }

  पक्ष json::मान::Array: {
    स्वतः Arr = *Data.getAsArray();
    यदि (Arr.empty())
      फर्काऊ;
    [[fallthrough]];
  }
  पक्ष json::मान::Object:
  पक्ष json::मान::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.मान_2(Data);
    रोक;
  }
  }
}

शून्य ASTNode::render_जरा(अचर json::मान &CurrentCtx,
                         MustacheOutputStream &OS) {
  renderChild(CurrentCtx, OS);
}

शून्य ASTNode::renderText(MustacheOutputStream &OS) { OS << Body; }

शून्य ASTNode::renderPartial(अचर json::मान &CurrentCtx,
                            MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_मान[0]
                    << ", Indentation:" << Indentation << "\n");
  स्वतः Partial = Ctx.Partials.find(Accessor_मान[0]);
  यदि (Partial != Ctx.Partials.end())
    renderPartial(CurrentCtx, OS, Partial->get_मान());
}

शून्य ASTNode::renderVariable(अचर json::मान &CurrentCtx,
                             MustacheOutputStream &OS) {
  स्वतः Lambda = Ctx.Lambdas.find(Accessor_मान[0]);
  यदि (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_मान());
  } अन्यथा यदि (अचर json::मान *ContextPtr = findContext()) {
    EscapeStringStream ES(OS, Ctx.Escapes);
    toMustacheString(*ContextPtr, ES);
  }
}

शून्य ASTNode::renderUnescapeVariable(अचर json::मान &CurrentCtx,
                                     MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_मान[0]
                    << "\n");
  स्वतः Lambda = Ctx.Lambdas.find(Accessor_मान[0]);
  यदि (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_मान());
  } अन्यथा यदि (अचर json::मान *ContextPtr = findContext()) {
    OS.suspendIndentation();
    toMustacheString(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

शून्य ASTNode::renderSection(अचर json::मान &CurrentCtx,
                            MustacheOutputStream &OS) {
  स्वतः SectionLambda = Ctx.SectionLambdas.find(Accessor_मान[0]);
  यदि (SectionLambda != Ctx.SectionLambdas.end()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_मान());
    फर्काऊ;
  }

  अचर json::मान *ContextPtr = findContext();
  यदि (isContextFalsey(ContextPtr))
    फर्काऊ;

  यदि (अचर json::Array *Arr = ContextPtr->getAsArray()) {
    लागि (अचर json::मान &V : *Arr)
      renderChild(V, OS);
    फर्काऊ;
  }
  renderChild(*ContextPtr, OS);
}

शून्य ASTNode::renderInvertSection(अचर json::मान &CurrentCtx,
                                  MustacheOutputStream &OS) {
  तार्किक IsLambda = Ctx.SectionLambdas.contains(Accessor_मान[0]);
  अचर json::मान *ContextPtr = findContext();
  यदि (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

शून्य ASTNode::render(अचर llvm::json::मान &Data, MustacheOutputStream &OS) {
  यदि (Ty != जरा_2 && Ty != Text && Accessor_मान.empty())
    फर्काऊ;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &Data;

  विकल्प (Ty) {
  पक्ष जरा_2:
    render_जरा(Data, OS);
    फर्काऊ;
  पक्ष Text:
    renderText(OS);
    फर्काऊ;
  पक्ष Partial:
    renderPartial(Data, OS);
    फर्काऊ;
  पक्ष Variable:
    renderVariable(Data, OS);
    फर्काऊ;
  पक्ष UnescapeVariable:
    renderUnescapeVariable(Data, OS);
    फर्काऊ;
  पक्ष Section:
    renderSection(Data, OS);
    फर्काऊ;
  पक्ष InvertSection:
    renderInvertSection(Data, OS);
    फर्काऊ;
  }
  llvm_unreachable("Invalid ASTNode type");
}

अचर json::मान *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  यदि (Accessor_मान.empty())
    फर्काऊ nullptr;
  यदि (Accessor_मान[0] == ".")
    फर्काऊ ParentContext;

  अचर json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = Accessor_मान[0];
  ASTNode *CurrentParent = Parent;

  जबसम्म (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    यदि (CurrentParent->Ty != जरा_2) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      जारी;
    }
    फर्काऊ nullptr;
  }
  अचर json::मान *Context = nullptr;
  लागि (स्वतः [Idx, Acc] : enumerate(Accessor_मान)) {
    अचर json::मान *Current_मान = CurrentContext->get(Acc);
    यदि (!Current_मान)
      फर्काऊ nullptr;
    यदि (Idx < Accessor_मान.size() - 1) {
      CurrentContext = Current_मान->getAsObject();
      यदि (!CurrentContext)
        फर्काऊ nullptr;
    } अन्यथा {
      Context = Current_मान;
    }
  }
  फर्काऊ Context;
}

शून्य ASTNode::renderChild(अचर json::मान &Contexts,
                          MustacheOutputStream &OS) {
  लागि (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

शून्य ASTNode::renderPartial(अचर json::मान &Contexts,
                            MustacheOutputStream &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  AddIndentationStringStream IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

शून्य ASTNode::renderLambdas(अचर llvm::json::मान &Contexts,
                            MustacheOutputStream &OS, Lambda &L) {
  json::मान LambdaResult = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(LambdaResult, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_गाँठो = P.parse();

  EscapeStringStream ES(OS, Ctx.Escapes);
  यदि (Ty == Variable) {
    Lambda_गाँठो->render(Contexts, ES);
    फर्काऊ;
  }
  Lambda_गाँठो->render(Contexts, OS);
}

शून्य ASTNode::renderSectionLambdas(अचर llvm::json::मान &Contexts,
                                   MustacheOutputStream &OS, SectionLambda &L) {
  json::मान Return = L(RawBody.str());
  यदि (isFalsey(Return))
    फर्काऊ;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_गाँठो = P.parse();
  Lambda_गाँठो->render(Contexts, OS);
}

शून्य Template::render(अचर llvm::json::मान &Data, llvm::raw_ostream &OS) {
  RawMustacheOutputStream MOS(OS);
  रूख->render(Data, MOS);
}

शून्य Template::registerPartial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_रूख = P.parse();
  Ctx.Partials.घुसाउ(std::make_pair(Name, Partial_रूख));
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
  रूख = P.parse();
  // The default behavior is to escape html entities.
  अचर EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), रूख(Other.रूख) {
  Other.रूख = nullptr;
}

Template::~Template() = स्वभाव;

} // namespace llvm::mustache

#undef DEBUG_TYPE
