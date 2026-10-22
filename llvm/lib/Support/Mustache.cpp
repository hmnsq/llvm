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

#تعریف_کرو DEBUG_TYPE "mustache"

استعمال نامخلا llvm;
استعمال نامخلا llvm::mustache;

نامخلا {

استعمال Accessor = ArrayRef<StringRef>;

ساکن منطقی isFalsey(ثابت json::قدر &V) {
  واپس V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().قدر_2()) ||
         (V.getAsArray() && V.getAsArray()->خالی_خانے());
}

ساکن منطقی isContextFalsey(ثابت json::قدر *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  اگر (!V)
    واپس سچ;
  واپس isFalsey(*V);
}

ساکن خالی بانٹو_And_Trim(StringRef Str, Small_سمتیہ_Impl<StringRef> &علامات) {
  size_t CurrentPos = 0;
  جب (CurrentPos < Str.سائز_2()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.ڈھونڈو('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    اگر (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.سائز_2();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.قاش(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t آغاز = Part.ڈھونڈو_پہلا_not_کا(" \t\r\n");
    اگر (آغاز != StringRef::npos) {
      size_t انتہا_2 = Part.ڈھونڈو_آخری_not_کا(" \t\r\n");
      علامات.ڈالو_عقب(Part.قاش(آغاز, انتہا_2 + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

ساکن Accessor بانٹو_Mustache_String(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  Small_سمتیہ<StringRef> علامات;
  اگر (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    علامات.ڈالو_عقب(".");
  } ورنہ {
    بانٹو_And_Trim(Str, علامات);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *Arena_علامات = Ctx.Allocator.Allocate<StringRef>(علامات.سائز_2());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(علامات, Arena_علامات);
  // Return an ArrayRef pointing to the stable arena memory.
  واپس ArrayRef<StringRef>(Arena_علامات, علامات.سائز_2());
}
} // namespace

نامخلا llvm::mustache {

جماعت Mustache_Output_دھارا : عوامی raw_ostream {
عوامی:
  Mustache_Output_دھارا() = طےشدہ;
  ~Mustache_Output_دھارا() override = طےشدہ;

  مجازی خالی suspendIndentation() {}
  مجازی خالی resumeIndentation() {}

نجی:
  خالی anchor() override;
};

خالی Mustache_Output_دھارا::anchor() {}

جماعت Raw_Mustache_Output_دھارا : عوامی Mustache_Output_دھارا {
عوامی:
  Raw_Mustache_Output_دھارا(raw_ostream &OS) : OS(OS) { سیٹ_Unbuffered(); }

نجی:
  raw_ostream &OS;

  خالی write_impl(ثابت حرف *Ptr, size_t سائز) override {
    OS.اكتب(Ptr, سائز);
  }
  uint64_t current_pos() ثابت override { واپس OS.tell(); }
};

جماعت Token {
عوامی:
  شمار جماعت Type {
    متن_2,
    Variable,
    Partial,
    SectionOpen,
    SectionClose,
    InvertSectionOpen,
    UnescapeVariable,
    Comment,
    سیٹ_Delimiter,
  };

  Token(StringRef Str)
      : TokenType(Type::متن_2), RawBody(Str), TokenBody(RawBody),
        Accessor_قدر({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, حرف Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    اگر (TokenType == Type::Comment)
      واپس;
    StringRef AccessorStr(یہ->TokenBody);
    اگر (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_قدر = بانٹو_Mustache_String(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ثابت { واپس Accessor_قدر; }

  Type getType() ثابت { واپس TokenType; }

  خالی سیٹ_Indentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() ثابت { واپس Indentation; }

  ساکن Type getTokenType(حرف Identifier) {
    انتخاب (Identifier) {
    حالت '#':
      واپس Type::SectionOpen;
    حالت '/':
      واپس Type::SectionClose;
    حالت '^':
      واپس Type::InvertSectionOpen;
    حالت '!':
      واپس Type::Comment;
    حالت '>':
      واپس Type::Partial;
    حالت '&':
      واپس Type::UnescapeVariable;
    حالت '=':
      واپس Type::سیٹ_Delimiter;
    طےشدہ:
      واپس Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_قدر;
  size_t Indentation;
};

استعمال EscapeMap = DenseMap<حرف, std::string>;

جماعت ASTNode : عوامی ilist_گرہ<ASTNode> {
عوامی:
  شمار Type {
    جڑ_2,
    متن_2,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::جڑ_2), والد_2(خالی_اشاریہ), والد_Context(خالی_اشاریہ) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *والد_2)
      : Ctx(Ctx), Ty(Type::متن_2), Body(Body), والد_2(والد_2),
        والد_Context(خالی_اشاریہ) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *والد_2)
      : Ctx(Ctx), Ty(Ty), والد_2(والد_2), Accessor_قدر(Accessor),
        والد_Context(خالی_اشاریہ) {}

  خالی جوڑو_بچہ(AstPtr بچہ) { Children.ڈالو_عقب(بچہ); };

  خالی سیٹ_Raw_Body(StringRef NewBody) { RawBody = NewBody; };

  خالی سیٹ_Indentation(size_t NewIndentation) { Indentation = NewIndentation; };

  خالی render(ثابت llvm::json::قدر &ڈیٹا_2, Mustache_Output_دھارا &OS);

نجی:
  خالی renderLambdas(ثابت llvm::json::قدر &Contexts,
                     Mustache_Output_دھارا &OS, Lambda &L);

  خالی renderSectionLambdas(ثابت llvm::json::قدر &Contexts,
                            Mustache_Output_دھارا &OS, SectionLambda &L);

  خالی renderPartial(ثابت llvm::json::قدر &Contexts,
                     Mustache_Output_دھارا &OS, ASTNode *Partial);

  خالی render_بچہ(ثابت llvm::json::قدر &Context, Mustache_Output_دھارا &OS);

  ثابت llvm::json::قدر *ڈھونڈو_Context();

  خالی render_جڑ(ثابت json::قدر &CurrentCtx, Mustache_Output_دھارا &OS);
  خالی render_متن(Mustache_Output_دھارا &OS);
  خالی renderPartial(ثابت json::قدر &CurrentCtx, Mustache_Output_دھارا &OS);
  خالی renderVariable(ثابت json::قدر &CurrentCtx, Mustache_Output_دھارا &OS);
  خالی renderUnescapeVariable(ثابت json::قدر &CurrentCtx,
                              Mustache_Output_دھارا &OS);
  خالی renderSection(ثابت json::قدر &CurrentCtx, Mustache_Output_دھارا &OS);
  خالی renderInvertSection(ثابت json::قدر &CurrentCtx,
                           Mustache_Output_دھارا &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *والد_2;
  ASTNodeList Children;
  ثابت ArrayRef<StringRef> Accessor_قدر;
  ثابت llvm::json::قدر *والد_Context;
};

// A wrapper for arena allocator for ASTNodes
ساکن AstPtr create_جڑ_گرہ(MustacheContext &Ctx) {
  واپس نیا (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

ساکن AstPtr create_گرہ(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *والد_2) {
  واپس نیا (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, والد_2);
}

ساکن AstPtr create_متن_گرہ(MustacheContext &Ctx, StringRef Body,
                             ASTNode *والد_2) {
  واپس نیا (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, والد_2);
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
ساکن منطقی has_متن_Behind(size_t Idx, ثابت ArrayRef<Token> &علامات) {
  اگر (Idx == 0)
    واپس سچ;

  size_t پچھلا_Idx = Idx - 1;
  اگر (علامات[پچھلا_Idx].getType() != Token::Type::متن_2)
    واپس سچ;

  ثابت Token &پچھلا_Token = علامات[پچھلا_Idx];
  StringRef TokenBody = StringRef(پچھلا_Token.RawBody).rtrim(" \r\t\v");
  واپس !TokenBody.سرے_with("\n") && !(TokenBody.خالی_خانے() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
ساکن منطقی has_متن_Ahead(size_t Idx, ثابت ArrayRef<Token> &علامات) {
  اگر (Idx >= علامات.سائز_2() - 1)
    واپس سچ;

  size_t اگلا_Idx = Idx + 1;
  اگر (علامات[اگلا_Idx].getType() != Token::Type::متن_2)
    واپس سچ;

  ثابت Token &اگلا_Token = علامات[اگلا_Idx];
  StringRef TokenBody = StringRef(اگلا_Token.RawBody).ltrim(" ");
  واپس !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

ساکن منطقی requires_Clean_اوپر(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  واپس T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::سیٹ_Delimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
ساکن خالی stripTokenAhead(Small_سمتیہ_Impl<Token> &علامات, size_t Idx) {
  Token &اگلا_Token = علامات[Idx + 1];
  StringRef اگلا_Token_Body = اگلا_Token.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  اگر (اگلا_Token_Body.starts_with("\r\n"))
    اگلا_Token.TokenBody = اگلا_Token_Body.substr(2);
  ورنہ اگر (اگلا_Token_Body.starts_with("\n"))
    اگلا_Token.TokenBody = اگلا_Token_Body.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
خالی stripTokenBefore(Small_سمتیہ_Impl<Token> &علامات, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &پچھلا_Token = علامات[Idx - 1];
  StringRef پچھلا_Token_Body = پچھلا_Token.TokenBody;
  StringRef Unindented = پچھلا_Token_Body.rtrim(" \r\t\v");
  size_t Indentation = پچھلا_Token_Body.سائز_2() - Unindented.سائز_2();
  پچھلا_Token.TokenBody = Unindented;
  CurrentToken.سیٹ_Indentation(Indentation);
}

ساخت لیبل {
  شمار جماعت قسم_3 {
    None,
    معمول, // {{...}}
    Triple, // {{{...}}}
  };

  قسم_3 لیبل_قسم = قسم_3::None;
  StringRef Content;   // The content between the delimiters.
  StringRef بھرا_مطابقت; // The entire tag, including delimiters.
  size_t آغاز_Position = StringRef::npos;
};

[[maybe_غیر_مستعمل]] ساکن ثابت حرف *لیبل_قسم_مقصد_String(لیبل::قسم_3 K) {
  انتخاب (K) {
  حالت لیبل::قسم_3::None:
    واپس "None";
  حالت لیبل::قسم_3::معمول:
    واپس "Normal";
  حالت لیبل::قسم_3::Triple:
    واپس "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_غیر_مستعمل]] ساکن ثابت حرف *json_قسم_مقصد_String(json::قدر::قسم_3 K) {
  انتخاب (K) {
  حالت json::قدر::قسم_3::Null:
    واپس "JSON_KIND_NULL";
  حالت json::قدر::قسم_3::Boolean:
    واپس "JSON_KIND_BOOLEAN";
  حالت json::قدر::قسم_3::نمبر:
    واپس "JSON_KIND_NUMBER";
  حالت json::قدر::قسم_3::String:
    واپس "JSON_KIND_STRING";
  حالت json::قدر::قسم_3::Array:
    واپس "JSON_KIND_ARRAY";
  حالت json::قدر::قسم_3::Object:
    واپس "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
ساکن Small_سمتیہ<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  Small_سمتیہ<Token> علامات;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t متن_آغاز = 0;

  ثابت StringLiteral TripleOpen("{{{");
  ثابت StringLiteral TripleClose("}}}");

  جب (Cursor < Template.سائز_2()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef لیبل_Open, لیبل_Close;
    لیبل::قسم_3 قسم_3;

    // Determine which tag we've encountered.
    اگر (TemplateSuffix.starts_with(TripleOpen)) {
      قسم_3 = لیبل::قسم_3::Triple;
      لیبل_Open = TripleOpen;
      لیبل_Close = TripleClose;
    } ورنہ اگر (TemplateSuffix.starts_with(Open)) {
      قسم_3 = لیبل::قسم_3::معمول;
      لیبل_Open = Open;
      لیبل_Close = Close;
    } ورنہ {
      // Not at a tag, continue scanning.
      ++Cursor;
      جاری;
    }

    // Found a tag, first add the preceding text.
    اگر (Cursor > متن_آغاز)
      علامات.emplace_عقب(Template.قاش(متن_آغاز, Cursor));

    // Find the closing tag.
    size_t انتہا_Pos = Template.ڈھونڈو(لیبل_Close, Cursor + لیبل_Open.سائز_2());
    اگر (انتہا_Pos == StringRef::npos) {
      // No closing tag, the rest is text.
      علامات.emplace_عقب(Template.substr(Cursor));
      متن_آغاز = Cursor = Template.سائز_2();
      توڑو;
    }

    // Extract tag content and full match.
    size_t Content_آغاز = Cursor + لیبل_Open.سائز_2();
    StringRef Content = Template.substr(Content_آغاز, انتہا_Pos - Content_آغاز);
    StringRef بھرا_مطابقت =
        Template.substr(Cursor, (انتہا_Pos + لیبل_Close.سائز_2()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << بھرا_مطابقت << ", Content: " << Content
                      << ", Kind: " << لیبل_قسم_مقصد_String(قسم_3) << "\n");
    اگر (قسم_3 == لیبل::قسم_3::Triple) {
      علامات.emplace_عقب(بھرا_مطابقت, Ctx.Saver.save("&" + Content), '&', Ctx);
    } ورنہ { // Normal Tag
      StringRef Interpolated = Content;
      اگر (!Interpolated.trim().starts_with("=")) {
        حرف Front = Interpolated.خالی_خانے() ? ' ' : Interpolated.trim().front();
        علامات.emplace_عقب(بھرا_مطابقت, Interpolated, Front, Ctx);
      } ورنہ { // Set Delimiter
        علامات.emplace_عقب(بھرا_مطابقت, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](حرف C) { واپس C == '='; });
        DelimSpec = DelimSpec.trim();

        خودکار [NewOpen, NewClose] = DelimSpec.بانٹو(' ');
        LLVM_DEBUG(dbgs() << "[Set Delimiter] NewOpen: " << NewOpen
                          << ", NewClose: " << NewClose << "\n");
        Open = NewOpen;
        Close = NewClose;
      }
    }

    // Move past the tag for the next iteration.
    Cursor += بھرا_مطابقت.سائز_2();
    متن_آغاز = Cursor;
  }

  // Add any remaining text after the last tag.
  اگر (متن_آغاز < Template.سائز_2())
    علامات.emplace_عقب(Template.substr(متن_آغاز));

  // Fix up white spaces for standalone tags.
  size_t آخری_Idx = علامات.سائز_2() - 1;
  برائے (size_t Idx = 0, انتہا_2 = علامات.سائز_2(); Idx < انتہا_2; ++Idx) {
    Token &CurrentToken = علامات[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    اگر (!requires_Clean_اوپر(CurrentType))
      جاری;

    منطقی Has_متن_Behind = has_متن_Behind(Idx, علامات);
    منطقی Has_متن_Ahead = has_متن_Ahead(Idx, علامات);

    اگر ((!Has_متن_Ahead && !Has_متن_Behind) || (!Has_متن_Ahead && Idx == 0))
      stripTokenAhead(علامات, Idx);

    اگر ((!Has_متن_Behind && !Has_متن_Ahead) || (!Has_متن_Behind && Idx == آخری_Idx))
      stripTokenBefore(علامات, Idx, CurrentToken, CurrentType);
  }
  واپس علامات;
}

// Custom stream to escape strings.
جماعت Escape_String_دھارا : عوامی Mustache_Output_دھارا {
عوامی:
  صریح Escape_String_دھارا(llvm::raw_ostream &Wrapped_دھارا,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.کلیدیں().begin(), Escape.کلیدیں().انتہا()),
        Wrapped_دھارا(Wrapped_دھارا) {
    سیٹ_Unbuffered();
  }

محفوظ:
  خالی write_impl(ثابت حرف *Ptr, size_t سائز) override {
    StringRef ڈیٹا_2(Ptr, سائز);
    size_t آغاز = 0;
    جب (آغاز < سائز) {
      // Find the next character that needs to be escaped.
      size_t اگلا_2 = ڈیٹا_2.ڈھونڈو_پہلا_کا(EscapeChars.str(), آغاز);

      // If no escapable characters are found, write the rest of the string.
      اگر (اگلا_2 == StringRef::npos) {
        Wrapped_دھارا << ڈیٹا_2.substr(آغاز);
        واپس;
      }

      // Write the chunk of text before the escapable character.
      اگر (اگلا_2 > آغاز)
        Wrapped_دھارا << ڈیٹا_2.substr(آغاز, اگلا_2 - آغاز);

      // Look up and write the escaped version of the character.
      Wrapped_دھارا << Escape[ڈیٹا_2[اگلا_2]];
      آغاز = اگلا_2 + 1;
    }
  }

  uint64_t current_pos() ثابت override { واپس Wrapped_دھارا.tell(); }

نجی:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &Wrapped_دھارا;
};

// Custom stream to add indentation used to for rendering partials.
جماعت جوڑو_Indentation_String_دھارا : عوامی Mustache_Output_دھارا {
عوامی:
  صریح جوڑو_Indentation_String_دھارا(raw_ostream &Wrapped_دھارا,
                                      size_t Indentation)
      : Indentation(Indentation), Wrapped_دھارا(Wrapped_دھارا),
        NeedsIndent(سچ), IsSuspended(جھوٹ) {
    سیٹ_Unbuffered();
  }

  خالی suspendIndentation() override { IsSuspended = سچ; }
  خالی resumeIndentation() override { IsSuspended = جھوٹ; }

محفوظ:
  خالی write_impl(ثابت حرف *Ptr, size_t سائز) override {
    llvm::StringRef ڈیٹا_2(Ptr, سائز);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    برائے (حرف C : ڈیٹا_2) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      اگر (NeedsIndent && C != '\n') {
        Wrapped_دھارا << Indent;
        NeedsIndent = جھوٹ;
      }
      Wrapped_دھارا << C;
      اگر (C == '\n' && !IsSuspended)
        NeedsIndent = سچ;
    }
  }

  uint64_t current_pos() ثابت override { واپس Wrapped_دھارا.tell(); }

نجی:
  size_t Indentation;
  raw_ostream &Wrapped_دھارا;
  منطقی NeedsIndent;
  منطقی IsSuspended;
};

جماعت Parser {
عوامی:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

نجی:
  خالی parseMustache(ASTNode *والد_2);
  خالی parseSection(ASTNode *والد_2, ASTNode::Type Ty, ثابت Accessor &A);

  MustacheContext &Ctx;
  Small_سمتیہ<Token> علامات;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

خالی Parser::parseSection(ASTNode *والد_2, ASTNode::Type Ty,
                          ثابت Accessor &A) {
  AstPtr Current_گرہ = create_گرہ(Ctx, Ty, A, والد_2);
  size_t آغاز = CurrentPtr;
  parseMustache(Current_گرہ);
  ثابت size_t انتہا_2 = CurrentPtr - 1;

  size_t Raw_Body_سائز = 0;
  برائے (size_t I = آغاز; I < انتہا_2; ++I)
    Raw_Body_سائز += علامات[I].RawBody.سائز_2();

  SmallString<128> RawBody;
  RawBody.reserve(Raw_Body_سائز);
  برائے (std::size_t I = آغاز; I < انتہا_2; ++I)
    RawBody += علامات[I].RawBody;

  Current_گرہ->سیٹ_Raw_Body(Ctx.Saver.save(StringRef(RawBody)));
  والد_2->جوڑو_بچہ(Current_گرہ);
}

AstPtr Parser::parse() {
  علامات = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr جڑ_گرہ = create_جڑ_گرہ(Ctx);
  parseMustache(جڑ_گرہ);
  واپس جڑ_گرہ;
}

خالی Parser::parseMustache(ASTNode *والد_2) {

  جب (CurrentPtr < علامات.سائز_2()) {
    Token CurrentToken = علامات[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_گرہ;

    انتخاب (CurrentToken.getType()) {
    حالت Token::Type::متن_2: {
      Current_گرہ = create_متن_گرہ(Ctx, CurrentToken.TokenBody, والد_2);
      والد_2->جوڑو_بچہ(Current_گرہ);
      توڑو;
    }
    حالت Token::Type::Variable: {
      Current_گرہ = create_گرہ(Ctx, ASTNode::Variable, A, والد_2);
      والد_2->جوڑو_بچہ(Current_گرہ);
      توڑو;
    }
    حالت Token::Type::UnescapeVariable: {
      Current_گرہ = create_گرہ(Ctx, ASTNode::UnescapeVariable, A, والد_2);
      والد_2->جوڑو_بچہ(Current_گرہ);
      توڑو;
    }
    حالت Token::Type::Partial: {
      Current_گرہ = create_گرہ(Ctx, ASTNode::Partial, A, والد_2);
      Current_گرہ->سیٹ_Indentation(CurrentToken.getIndentation());
      والد_2->جوڑو_بچہ(Current_گرہ);
      توڑو;
    }
    حالت Token::Type::SectionOpen: {
      parseSection(والد_2, ASTNode::Section, A);
      توڑو;
    }
    حالت Token::Type::InvertSectionOpen: {
      parseSection(والد_2, ASTNode::InvertSection, A);
      توڑو;
    }
    حالت Token::Type::Comment:
    حالت Token::Type::سیٹ_Delimiter:
      توڑو;
    حالت Token::Type::SectionClose:
      واپس;
    }
  }
}
ساکن خالی مقصد_Mustache_String(ثابت json::قدر &ڈیٹا_2, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << json_قسم_مقصد_String(ڈیٹا_2.قسم_2()) << ", Data: " << ڈیٹا_2
                    << "\n");
  انتخاب (ڈیٹا_2.قسم_2()) {
  حالت json::قدر::Null:
    واپس;
  حالت json::قدر::نمبر: {
    خودکار عدد = *ڈیٹا_2.get_As_نمبر();
    std::ostringstream SS;
    SS << عدد;
    OS << SS.str();
    واپس;
  }
  حالت json::قدر::String: {
    OS << *ڈیٹا_2.getAsString();
    واپس;
  }

  حالت json::قدر::Array: {
    خودکار Arr = *ڈیٹا_2.getAsArray();
    اگر (Arr.خالی_خانے())
      واپس;
    [[fallthrough]];
  }
  حالت json::قدر::Object:
  حالت json::قدر::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.قدر_2(ڈیٹا_2);
    توڑو;
  }
  }
}

خالی ASTNode::render_جڑ(ثابت json::قدر &CurrentCtx,
                         Mustache_Output_دھارا &OS) {
  render_بچہ(CurrentCtx, OS);
}

خالی ASTNode::render_متن(Mustache_Output_دھارا &OS) { OS << Body; }

خالی ASTNode::renderPartial(ثابت json::قدر &CurrentCtx,
                            Mustache_Output_دھارا &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_قدر[0]
                    << ", Indentation:" << Indentation << "\n");
  خودکار Partial = Ctx.Partials.ڈھونڈو(Accessor_قدر[0]);
  اگر (Partial != Ctx.Partials.انتہا())
    renderPartial(CurrentCtx, OS, Partial->get_قدر());
}

خالی ASTNode::renderVariable(ثابت json::قدر &CurrentCtx,
                             Mustache_Output_دھارا &OS) {
  خودکار Lambda = Ctx.Lambdas.ڈھونڈو(Accessor_قدر[0]);
  اگر (Lambda != Ctx.Lambdas.انتہا()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_قدر());
  } ورنہ اگر (ثابت json::قدر *ContextPtr = ڈھونڈو_Context()) {
    Escape_String_دھارا ES(OS, Ctx.Escapes);
    مقصد_Mustache_String(*ContextPtr, ES);
  }
}

خالی ASTNode::renderUnescapeVariable(ثابت json::قدر &CurrentCtx,
                                     Mustache_Output_دھارا &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_قدر[0]
                    << "\n");
  خودکار Lambda = Ctx.Lambdas.ڈھونڈو(Accessor_قدر[0]);
  اگر (Lambda != Ctx.Lambdas.انتہا()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_قدر());
  } ورنہ اگر (ثابت json::قدر *ContextPtr = ڈھونڈو_Context()) {
    OS.suspendIndentation();
    مقصد_Mustache_String(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

خالی ASTNode::renderSection(ثابت json::قدر &CurrentCtx,
                            Mustache_Output_دھارا &OS) {
  خودکار SectionLambda = Ctx.SectionLambdas.ڈھونڈو(Accessor_قدر[0]);
  اگر (SectionLambda != Ctx.SectionLambdas.انتہا()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_قدر());
    واپس;
  }

  ثابت json::قدر *ContextPtr = ڈھونڈو_Context();
  اگر (isContextFalsey(ContextPtr))
    واپس;

  اگر (ثابت json::Array *Arr = ContextPtr->getAsArray()) {
    برائے (ثابت json::قدر &V : *Arr)
      render_بچہ(V, OS);
    واپس;
  }
  render_بچہ(*ContextPtr, OS);
}

خالی ASTNode::renderInvertSection(ثابت json::قدر &CurrentCtx,
                                  Mustache_Output_دھارا &OS) {
  منطقی IsLambda = Ctx.SectionLambdas.شامل_ہے(Accessor_قدر[0]);
  ثابت json::قدر *ContextPtr = ڈھونڈو_Context();
  اگر (isContextFalsey(ContextPtr) && !IsLambda) {
    render_بچہ(CurrentCtx, OS);
  }
}

خالی ASTNode::render(ثابت llvm::json::قدر &ڈیٹا_2, Mustache_Output_دھارا &OS) {
  اگر (Ty != جڑ_2 && Ty != متن_2 && Accessor_قدر.خالی_خانے())
    واپس;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  والد_Context = &ڈیٹا_2;

  انتخاب (Ty) {
  حالت جڑ_2:
    render_جڑ(ڈیٹا_2, OS);
    واپس;
  حالت متن_2:
    render_متن(OS);
    واپس;
  حالت Partial:
    renderPartial(ڈیٹا_2, OS);
    واپس;
  حالت Variable:
    renderVariable(ڈیٹا_2, OS);
    واپس;
  حالت UnescapeVariable:
    renderUnescapeVariable(ڈیٹا_2, OS);
    واپس;
  حالت Section:
    renderSection(ڈیٹا_2, OS);
    واپس;
  حالت InvertSection:
    renderInvertSection(ڈیٹا_2, OS);
    واپس;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ثابت json::قدر *ASTNode::ڈھونڈو_Context() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  اگر (Accessor_قدر.خالی_خانے())
    واپس خالی_اشاریہ;
  اگر (Accessor_قدر[0] == ".")
    واپس والد_Context;

  ثابت json::Object *CurrentContext = والد_Context->getAsObject();
  StringRef CurrentAccessor = Accessor_قدر[0];
  ASTNode *Current_والد = والد_2;

  جب (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    اگر (Current_والد->Ty != جڑ_2) {
      CurrentContext = Current_والد->والد_Context->getAsObject();
      Current_والد = Current_والد->والد_2;
      جاری;
    }
    واپس خالی_اشاریہ;
  }
  ثابت json::قدر *Context = خالی_اشاریہ;
  برائے (خودکار [Idx, Acc] : enumerate(Accessor_قدر)) {
    ثابت json::قدر *Current_قدر = CurrentContext->get(Acc);
    اگر (!Current_قدر)
      واپس خالی_اشاریہ;
    اگر (Idx < Accessor_قدر.سائز_2() - 1) {
      CurrentContext = Current_قدر->getAsObject();
      اگر (!CurrentContext)
        واپس خالی_اشاریہ;
    } ورنہ {
      Context = Current_قدر;
    }
  }
  واپس Context;
}

خالی ASTNode::render_بچہ(ثابت json::قدر &Contexts,
                          Mustache_Output_دھارا &OS) {
  برائے (ASTNode &بچہ : Children)
    بچہ.render(Contexts, OS);
}

خالی ASTNode::renderPartial(ثابت json::قدر &Contexts,
                            Mustache_Output_دھارا &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  جوڑو_Indentation_String_دھارا IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

خالی ASTNode::renderLambdas(ثابت llvm::json::قدر &Contexts,
                            Mustache_Output_دھارا &OS, Lambda &L) {
  json::قدر Lambda_نتیجہ = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  مقصد_Mustache_String(Lambda_نتیجہ, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_گرہ = P.parse();

  Escape_String_دھارا ES(OS, Ctx.Escapes);
  اگر (Ty == Variable) {
    Lambda_گرہ->render(Contexts, ES);
    واپس;
  }
  Lambda_گرہ->render(Contexts, OS);
}

خالی ASTNode::renderSectionLambdas(ثابت llvm::json::قدر &Contexts,
                                   Mustache_Output_دھارا &OS, SectionLambda &L) {
  json::قدر Return = L(RawBody.str());
  اگر (isFalsey(Return))
    واپس;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  مقصد_Mustache_String(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_گرہ = P.parse();
  Lambda_گرہ->render(Contexts, OS);
}

خالی Template::render(ثابت llvm::json::قدر &ڈیٹا_2, llvm::raw_ostream &OS) {
  Raw_Mustache_Output_دھارا MOS(OS);
  درخت->render(ڈیٹا_2, MOS);
}

خالی Template::رجسٹر_Partial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_درخت = P.parse();
  Ctx.Partials.داخل_کرو(std::make_pair(Name, Partial_درخت));
}

خالی Template::رجسٹر_Lambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

خالی Template::رجسٹر_Lambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

خالی Template::overrideEscapeCharacters(EscapeMap E) {
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
  Other.درخت = خالی_اشاریہ;
}

Template::~Template() = طےشدہ;

} // namespace llvm::mustache

#تعریف_منسوخ DEBUG_TYPE
