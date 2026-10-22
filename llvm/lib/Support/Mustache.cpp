//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#شامل "llvm/Support/Mustache.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <sstream>

#define DEBUG_TYPE "mustache"

using فضاینام llvm;
using فضاینام llvm::mustache;

فضاینام {

using Accessor = ArrayRef<StringRef>;

ایستا منطقی isFalsey(ثابت json::مقدار &V) {
  بازگشت V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().مقدار_2()) ||
         (V.getAsArray() && V.getAsArray()->empty());
}

ایستا منطقی isContextFalsey(ثابت json::مقدار *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  اگر (!V)
    بازگشت درست;
  بازگشت isFalsey(*V);
}

ایستا تهی splitAndTrim(StringRef Str, SmallVectorImpl<StringRef> &Tokens) {
  size_t CurrentPos = 0;
  وقتی (CurrentPos < Str.size()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    اگر (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.size();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.slice(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t Start = Part.find_first_not_of(" \t\r\n");
    اگر (Start != StringRef::npos) {
      size_t End = Part.find_last_not_of(" \t\r\n");
      Tokens.افزودن_back(Part.slice(Start, End + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

ایستا Accessor splitMustacheString(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  SmallVector<StringRef> Tokens;
  اگر (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    Tokens.افزودن_back(".");
  } وگرنه {
    splitAndTrim(Str, Tokens);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.size());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  بازگشت ArrayRef<StringRef>(ArenaTokens, Tokens.size());
}
} // namespace

فضاینام llvm::mustache {

رده MustacheOutputStream : عمومی raw_ostream {
عمومی:
  MustacheOutputStream() = پیشفرض;
  ~MustacheOutputStream() override = پیشفرض;

  مجازی تهی suspendIndentation() {}
  مجازی تهی resumeIndentation() {}

خصوصی:
  تهی anchor() override;
};

تهی MustacheOutputStream::anchor() {}

رده RawMustacheOutputStream : عمومی MustacheOutputStream {
عمومی:
  RawMustacheOutputStream(raw_ostream &OS) : OS(OS) { SetUnbuffered(); }

خصوصی:
  raw_ostream &OS;

  تهی write_impl(ثابت نویسه *Ptr, size_t Size) override {
    OS.اكتب(Ptr, Size);
  }
  uint64_t current_pos() ثابت override { بازگشت OS.tell(); }
};

رده Token {
عمومی:
  شمارش رده Type {
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
        Accessor_مقدار({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, نویسه Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    اگر (TokenType == Type::Comment)
      بازگشت;
    StringRef AccessorStr(این->TokenBody);
    اگر (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_مقدار = splitMustacheString(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ثابت { بازگشت Accessor_مقدار; }

  Type getType() ثابت { بازگشت TokenType; }

  تهی setIndentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() ثابت { بازگشت Indentation; }

  ایستا Type getTokenType(نویسه Identifier) {
    انتخاب (Identifier) {
    حالت '#':
      بازگشت Type::SectionOpen;
    حالت '/':
      بازگشت Type::SectionClose;
    حالت '^':
      بازگشت Type::InvertSectionOpen;
    حالت '!':
      بازگشت Type::Comment;
    حالت '>':
      بازگشت Type::Partial;
    حالت '&':
      بازگشت Type::UnescapeVariable;
    حالت '=':
      بازگشت Type::SetDelimiter;
    پیشفرض:
      بازگشت Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_مقدار;
  size_t Indentation;
};

using EscapeMap = DenseMap<نویسه, std::string>;

رده ASTNode : عمومی ilist_گره<ASTNode> {
عمومی:
  شمارش Type {
    ریشه_2,
    Text,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::ریشه_2), Parent(nullptr), ParentContext(nullptr) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *Parent)
      : Ctx(Ctx), Ty(Type::Text), Body(Body), Parent(Parent),
        ParentContext(nullptr) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *Parent)
      : Ctx(Ctx), Ty(Ty), Parent(Parent), Accessor_مقدار(Accessor),
        ParentContext(nullptr) {}

  تهی addChild(AstPtr Child) { Children.افزودن_back(Child); };

  تهی setRawBody(StringRef NewBody) { RawBody = NewBody; };

  تهی setIndentation(size_t NewIndentation) { Indentation = NewIndentation; };

  تهی render(ثابت llvm::json::مقدار &Data, MustacheOutputStream &OS);

خصوصی:
  تهی renderLambdas(ثابت llvm::json::مقدار &Contexts,
                     MustacheOutputStream &OS, Lambda &L);

  تهی renderSectionLambdas(ثابت llvm::json::مقدار &Contexts,
                            MustacheOutputStream &OS, SectionLambda &L);

  تهی renderPartial(ثابت llvm::json::مقدار &Contexts,
                     MustacheOutputStream &OS, ASTNode *Partial);

  تهی renderChild(ثابت llvm::json::مقدار &Context, MustacheOutputStream &OS);

  ثابت llvm::json::مقدار *findContext();

  تهی render_ریشه(ثابت json::مقدار &CurrentCtx, MustacheOutputStream &OS);
  تهی renderText(MustacheOutputStream &OS);
  تهی renderPartial(ثابت json::مقدار &CurrentCtx, MustacheOutputStream &OS);
  تهی renderVariable(ثابت json::مقدار &CurrentCtx, MustacheOutputStream &OS);
  تهی renderUnescapeVariable(ثابت json::مقدار &CurrentCtx,
                              MustacheOutputStream &OS);
  تهی renderSection(ثابت json::مقدار &CurrentCtx, MustacheOutputStream &OS);
  تهی renderInvertSection(ثابت json::مقدار &CurrentCtx,
                           MustacheOutputStream &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  ثابت ArrayRef<StringRef> Accessor_مقدار;
  ثابت llvm::json::مقدار *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
ایستا AstPtr create_ریشه_گره(MustacheContext &Ctx) {
  بازگشت جدید (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

ایستا AstPtr create_گره(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  بازگشت جدید (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

ایستا AstPtr create_Text_گره(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  بازگشت جدید (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
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
ایستا منطقی hasTextBehind(size_t Idx, ثابت ArrayRef<Token> &Tokens) {
  اگر (Idx == 0)
    بازگشت درست;

  size_t PrevIdx = Idx - 1;
  اگر (Tokens[PrevIdx].getType() != Token::Type::Text)
    بازگشت درست;

  ثابت Token &PrevToken = Tokens[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  بازگشت !TokenBody.ends_with("\n") && !(TokenBody.empty() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
ایستا منطقی hasTextAhead(size_t Idx, ثابت ArrayRef<Token> &Tokens) {
  اگر (Idx >= Tokens.size() - 1)
    بازگشت درست;

  size_t NextIdx = Idx + 1;
  اگر (Tokens[NextIdx].getType() != Token::Type::Text)
    بازگشت درست;

  ثابت Token &NextToken = Tokens[NextIdx];
  StringRef TokenBody = StringRef(NextToken.RawBody).ltrim(" ");
  بازگشت !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

ایستا منطقی requiresCleanUp(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  بازگشت T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::SetDelimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
ایستا تهی stripTokenAhead(SmallVectorImpl<Token> &Tokens, size_t Idx) {
  Token &NextToken = Tokens[Idx + 1];
  StringRef NextTokenBody = NextToken.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  اگر (NextTokenBody.starts_with("\r\n"))
    NextToken.TokenBody = NextTokenBody.substr(2);
  وگرنه اگر (NextTokenBody.starts_with("\n"))
    NextToken.TokenBody = NextTokenBody.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
تهی stripTokenBefore(SmallVectorImpl<Token> &Tokens, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = Tokens[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.size() - Unindented.size();
  PrevToken.TokenBody = Unindented;
  CurrentToken.setIndentation(Indentation);
}

ساختار Tag {
  شمارش رده Kind {
    None,
    Normal, // {{...}}
    Triple, // {{{...}}}
  };

  Kind TagKind = Kind::None;
  StringRef Content;   // The content between the delimiters.
  StringRef FullMatch; // The entire tag, including delimiters.
  size_t StartPosition = StringRef::npos;
};

[[maybe_unused]] ایستا ثابت نویسه *tagKindToString(Tag::Kind K) {
  انتخاب (K) {
  حالت Tag::Kind::None:
    بازگشت "None";
  حالت Tag::Kind::Normal:
    بازگشت "Normal";
  حالت Tag::Kind::Triple:
    بازگشت "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_unused]] ایستا ثابت نویسه *jsonKindToString(json::مقدار::Kind K) {
  انتخاب (K) {
  حالت json::مقدار::Kind::Null:
    بازگشت "JSON_KIND_NULL";
  حالت json::مقدار::Kind::Boolean:
    بازگشت "JSON_KIND_BOOLEAN";
  حالت json::مقدار::Kind::عدد:
    بازگشت "JSON_KIND_NUMBER";
  حالت json::مقدار::Kind::String:
    بازگشت "JSON_KIND_STRING";
  حالت json::مقدار::Kind::Array:
    بازگشت "JSON_KIND_ARRAY";
  حالت json::مقدار::Kind::Object:
    بازگشت "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
ایستا SmallVector<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  SmallVector<Token> Tokens;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t TextStart = 0;

  ثابت StringLiteral TripleOpen("{{{");
  ثابت StringLiteral TripleClose("}}}");

  وقتی (Cursor < Template.size()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef TagOpen, TagClose;
    Tag::Kind Kind;

    // Determine which tag we've encountered.
    اگر (TemplateSuffix.starts_with(TripleOpen)) {
      Kind = Tag::Kind::Triple;
      TagOpen = TripleOpen;
      TagClose = TripleClose;
    } وگرنه اگر (TemplateSuffix.starts_with(Open)) {
      Kind = Tag::Kind::Normal;
      TagOpen = Open;
      TagClose = Close;
    } وگرنه {
      // Not at a tag, continue scanning.
      ++Cursor;
      ادامه;
    }

    // Found a tag, first add the preceding text.
    اگر (Cursor > TextStart)
      Tokens.emplace_back(Template.slice(TextStart, Cursor));

    // Find the closing tag.
    size_t EndPos = Template.find(TagClose, Cursor + TagOpen.size());
    اگر (EndPos == StringRef::npos) {
      // No closing tag, the rest is text.
      Tokens.emplace_back(Template.substr(Cursor));
      TextStart = Cursor = Template.size();
      بشکن;
    }

    // Extract tag content and full match.
    size_t ContentStart = Cursor + TagOpen.size();
    StringRef Content = Template.substr(ContentStart, EndPos - ContentStart);
    StringRef FullMatch =
        Template.substr(Cursor, (EndPos + TagClose.size()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << FullMatch << ", Content: " << Content
                      << ", Kind: " << tagKindToString(Kind) << "\n");
    اگر (Kind == Tag::Kind::Triple) {
      Tokens.emplace_back(FullMatch, Ctx.Saver.save("&" + Content), '&', Ctx);
    } وگرنه { // Normal Tag
      StringRef Interpolated = Content;
      اگر (!Interpolated.trim().starts_with("=")) {
        نویسه Front = Interpolated.empty() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_back(FullMatch, Interpolated, Front, Ctx);
      } وگرنه { // Set Delimiter
        Tokens.emplace_back(FullMatch, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](نویسه C) { بازگشت C == '='; });
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
  اگر (TextStart < Template.size())
    Tokens.emplace_back(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  size_t LastIdx = Tokens.size() - 1;
  برای (size_t Idx = 0, End = Tokens.size(); Idx < End; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    اگر (!requiresCleanUp(CurrentType))
      ادامه;

    منطقی HasTextBehind = hasTextBehind(Idx, Tokens);
    منطقی HasTextAhead = hasTextAhead(Idx, Tokens);

    اگر ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    اگر ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == LastIdx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  بازگشت Tokens;
}

// Custom stream to escape strings.
رده EscapeStringStream : عمومی MustacheOutputStream {
عمومی:
  explicit EscapeStringStream(llvm::raw_ostream &WrappedStream,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.کلیدها().begin(), Escape.کلیدها().end()),
        WrappedStream(WrappedStream) {
    SetUnbuffered();
  }

محافظتشده:
  تهی write_impl(ثابت نویسه *Ptr, size_t Size) override {
    StringRef Data(Ptr, Size);
    size_t Start = 0;
    وقتی (Start < Size) {
      // Find the next character that needs to be escaped.
      size_t Next = Data.find_first_of(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      اگر (Next == StringRef::npos) {
        WrappedStream << Data.substr(Start);
        بازگشت;
      }

      // Write the chunk of text before the escapable character.
      اگر (Next > Start)
        WrappedStream << Data.substr(Start, Next - Start);

      // Look up and write the escaped version of the character.
      WrappedStream << Escape[Data[Next]];
      Start = Next + 1;
    }
  }

  uint64_t current_pos() ثابت override { بازگشت WrappedStream.tell(); }

خصوصی:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &WrappedStream;
};

// Custom stream to add indentation used to for rendering partials.
رده AddIndentationStringStream : عمومی MustacheOutputStream {
عمومی:
  explicit AddIndentationStringStream(raw_ostream &WrappedStream,
                                      size_t Indentation)
      : Indentation(Indentation), WrappedStream(WrappedStream),
        NeedsIndent(درست), IsSuspended(نادرست) {
    SetUnbuffered();
  }

  تهی suspendIndentation() override { IsSuspended = درست; }
  تهی resumeIndentation() override { IsSuspended = نادرست; }

محافظتشده:
  تهی write_impl(ثابت نویسه *Ptr, size_t Size) override {
    llvm::StringRef Data(Ptr, Size);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    برای (نویسه C : Data) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      اگر (NeedsIndent && C != '\n') {
        WrappedStream << Indent;
        NeedsIndent = نادرست;
      }
      WrappedStream << C;
      اگر (C == '\n' && !IsSuspended)
        NeedsIndent = درست;
    }
  }

  uint64_t current_pos() ثابت override { بازگشت WrappedStream.tell(); }

خصوصی:
  size_t Indentation;
  raw_ostream &WrappedStream;
  منطقی NeedsIndent;
  منطقی IsSuspended;
};

رده Parser {
عمومی:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

خصوصی:
  تهی parseMustache(ASTNode *Parent);
  تهی parseSection(ASTNode *Parent, ASTNode::Type Ty, ثابت Accessor &A);

  MustacheContext &Ctx;
  SmallVector<Token> Tokens;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

تهی Parser::parseSection(ASTNode *Parent, ASTNode::Type Ty,
                          ثابت Accessor &A) {
  AstPtr Current_گره = create_گره(Ctx, Ty, A, Parent);
  size_t Start = CurrentPtr;
  parseMustache(Current_گره);
  ثابت size_t End = CurrentPtr - 1;

  size_t RawBodySize = 0;
  برای (size_t I = Start; I < End; ++I)
    RawBodySize += Tokens[I].RawBody.size();

  SmallString<128> RawBody;
  RawBody.reserve(RawBodySize);
  برای (std::size_t I = Start; I < End; ++I)
    RawBody += Tokens[I].RawBody;

  Current_گره->setRawBody(Ctx.Saver.save(StringRef(RawBody)));
  Parent->addChild(Current_گره);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr ریشه_گره = create_ریشه_گره(Ctx);
  parseMustache(ریشه_گره);
  بازگشت ریشه_گره;
}

تهی Parser::parseMustache(ASTNode *Parent) {

  وقتی (CurrentPtr < Tokens.size()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_گره;

    انتخاب (CurrentToken.getType()) {
    حالت Token::Type::Text: {
      Current_گره = create_Text_گره(Ctx, CurrentToken.TokenBody, Parent);
      Parent->addChild(Current_گره);
      بشکن;
    }
    حالت Token::Type::Variable: {
      Current_گره = create_گره(Ctx, ASTNode::Variable, A, Parent);
      Parent->addChild(Current_گره);
      بشکن;
    }
    حالت Token::Type::UnescapeVariable: {
      Current_گره = create_گره(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->addChild(Current_گره);
      بشکن;
    }
    حالت Token::Type::Partial: {
      Current_گره = create_گره(Ctx, ASTNode::Partial, A, Parent);
      Current_گره->setIndentation(CurrentToken.getIndentation());
      Parent->addChild(Current_گره);
      بشکن;
    }
    حالت Token::Type::SectionOpen: {
      parseSection(Parent, ASTNode::Section, A);
      بشکن;
    }
    حالت Token::Type::InvertSectionOpen: {
      parseSection(Parent, ASTNode::InvertSection, A);
      بشکن;
    }
    حالت Token::Type::Comment:
    حالت Token::Type::SetDelimiter:
      بشکن;
    حالت Token::Type::SectionClose:
      بازگشت;
    }
  }
}
ایستا تهی toMustacheString(ثابت json::مقدار &Data, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << jsonKindToString(Data.kind()) << ", Data: " << Data
                    << "\n");
  انتخاب (Data.kind()) {
  حالت json::مقدار::Null:
    بازگشت;
  حالت json::مقدار::عدد: {
    auto رقم = *Data.get_As_عدد();
    std::ostringstream SS;
    SS << رقم;
    OS << SS.str();
    بازگشت;
  }
  حالت json::مقدار::String: {
    OS << *Data.getAsString();
    بازگشت;
  }

  حالت json::مقدار::Array: {
    auto Arr = *Data.getAsArray();
    اگر (Arr.empty())
      بازگشت;
    [[fallthrough]];
  }
  حالت json::مقدار::Object:
  حالت json::مقدار::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.مقدار_2(Data);
    بشکن;
  }
  }
}

تهی ASTNode::render_ریشه(ثابت json::مقدار &CurrentCtx,
                         MustacheOutputStream &OS) {
  renderChild(CurrentCtx, OS);
}

تهی ASTNode::renderText(MustacheOutputStream &OS) { OS << Body; }

تهی ASTNode::renderPartial(ثابت json::مقدار &CurrentCtx,
                            MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_مقدار[0]
                    << ", Indentation:" << Indentation << "\n");
  auto Partial = Ctx.Partials.find(Accessor_مقدار[0]);
  اگر (Partial != Ctx.Partials.end())
    renderPartial(CurrentCtx, OS, Partial->get_مقدار());
}

تهی ASTNode::renderVariable(ثابت json::مقدار &CurrentCtx,
                             MustacheOutputStream &OS) {
  auto Lambda = Ctx.Lambdas.find(Accessor_مقدار[0]);
  اگر (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_مقدار());
  } وگرنه اگر (ثابت json::مقدار *ContextPtr = findContext()) {
    EscapeStringStream ES(OS, Ctx.Escapes);
    toMustacheString(*ContextPtr, ES);
  }
}

تهی ASTNode::renderUnescapeVariable(ثابت json::مقدار &CurrentCtx,
                                     MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_مقدار[0]
                    << "\n");
  auto Lambda = Ctx.Lambdas.find(Accessor_مقدار[0]);
  اگر (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_مقدار());
  } وگرنه اگر (ثابت json::مقدار *ContextPtr = findContext()) {
    OS.suspendIndentation();
    toMustacheString(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

تهی ASTNode::renderSection(ثابت json::مقدار &CurrentCtx,
                            MustacheOutputStream &OS) {
  auto SectionLambda = Ctx.SectionLambdas.find(Accessor_مقدار[0]);
  اگر (SectionLambda != Ctx.SectionLambdas.end()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_مقدار());
    بازگشت;
  }

  ثابت json::مقدار *ContextPtr = findContext();
  اگر (isContextFalsey(ContextPtr))
    بازگشت;

  اگر (ثابت json::Array *Arr = ContextPtr->getAsArray()) {
    برای (ثابت json::مقدار &V : *Arr)
      renderChild(V, OS);
    بازگشت;
  }
  renderChild(*ContextPtr, OS);
}

تهی ASTNode::renderInvertSection(ثابت json::مقدار &CurrentCtx,
                                  MustacheOutputStream &OS) {
  منطقی IsLambda = Ctx.SectionLambdas.contains(Accessor_مقدار[0]);
  ثابت json::مقدار *ContextPtr = findContext();
  اگر (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

تهی ASTNode::render(ثابت llvm::json::مقدار &Data, MustacheOutputStream &OS) {
  اگر (Ty != ریشه_2 && Ty != Text && Accessor_مقدار.empty())
    بازگشت;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &Data;

  انتخاب (Ty) {
  حالت ریشه_2:
    render_ریشه(Data, OS);
    بازگشت;
  حالت Text:
    renderText(OS);
    بازگشت;
  حالت Partial:
    renderPartial(Data, OS);
    بازگشت;
  حالت Variable:
    renderVariable(Data, OS);
    بازگشت;
  حالت UnescapeVariable:
    renderUnescapeVariable(Data, OS);
    بازگشت;
  حالت Section:
    renderSection(Data, OS);
    بازگشت;
  حالت InvertSection:
    renderInvertSection(Data, OS);
    بازگشت;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ثابت json::مقدار *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  اگر (Accessor_مقدار.empty())
    بازگشت nullptr;
  اگر (Accessor_مقدار[0] == ".")
    بازگشت ParentContext;

  ثابت json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = Accessor_مقدار[0];
  ASTNode *CurrentParent = Parent;

  وقتی (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    اگر (CurrentParent->Ty != ریشه_2) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      ادامه;
    }
    بازگشت nullptr;
  }
  ثابت json::مقدار *Context = nullptr;
  برای (auto [Idx, Acc] : enumerate(Accessor_مقدار)) {
    ثابت json::مقدار *Current_مقدار = CurrentContext->get(Acc);
    اگر (!Current_مقدار)
      بازگشت nullptr;
    اگر (Idx < Accessor_مقدار.size() - 1) {
      CurrentContext = Current_مقدار->getAsObject();
      اگر (!CurrentContext)
        بازگشت nullptr;
    } وگرنه {
      Context = Current_مقدار;
    }
  }
  بازگشت Context;
}

تهی ASTNode::renderChild(ثابت json::مقدار &Contexts,
                          MustacheOutputStream &OS) {
  برای (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

تهی ASTNode::renderPartial(ثابت json::مقدار &Contexts,
                            MustacheOutputStream &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  AddIndentationStringStream IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

تهی ASTNode::renderLambdas(ثابت llvm::json::مقدار &Contexts,
                            MustacheOutputStream &OS, Lambda &L) {
  json::مقدار LambdaResult = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(LambdaResult, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_گره = P.parse();

  EscapeStringStream ES(OS, Ctx.Escapes);
  اگر (Ty == Variable) {
    Lambda_گره->render(Contexts, ES);
    بازگشت;
  }
  Lambda_گره->render(Contexts, OS);
}

تهی ASTNode::renderSectionLambdas(ثابت llvm::json::مقدار &Contexts,
                                   MustacheOutputStream &OS, SectionLambda &L) {
  json::مقدار Return = L(RawBody.str());
  اگر (isFalsey(Return))
    بازگشت;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_گره = P.parse();
  Lambda_گره->render(Contexts, OS);
}

تهی Template::render(ثابت llvm::json::مقدار &Data, llvm::raw_ostream &OS) {
  RawMustacheOutputStream MOS(OS);
  درخت->render(Data, MOS);
}

تهی Template::registerPartial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_درخت = P.parse();
  Ctx.Partials.درج(std::make_pair(Name, Partial_درخت));
}

تهی Template::registerLambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

تهی Template::registerLambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

تهی Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  درخت = P.parse();
  // The default behavior is to escape html entities.
  ثابت EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), درخت(Other.درخت) {
  Other.درخت = nullptr;
}

Template::~Template() = پیشفرض;

} // namespace llvm::mustache

#undef DEBUG_TYPE
