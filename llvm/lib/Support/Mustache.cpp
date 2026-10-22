//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ضمن "llvm/Support/Mustache.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <sstream>

#تعریف_کړه DEBUG_TYPE "mustache"

کارول نطاق llvm;
کارول نطاق llvm::mustache;

نطاق {

کارول Accessor = ArrayRef<StringRef>;

ساكن منطقي isFalsey(ثابت json::ارزښت &V) {
  أرجع V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().ارزښت_2()) ||
         (V.getAsArray() && V.getAsArray()->تشه());
}

ساكن منطقي isContextFalsey(ثابت json::ارزښت *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  إذا (!V)
    أرجع ریښتیا;
  أرجع isFalsey(*V);
}

ساكن فراغ وېشل_And_Trim(StringRef Str, Small_ویکټور_Impl<StringRef> &نښې) {
  size_t CurrentPos = 0;
  طالما (CurrentPos < Str.اندازه_2()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.پیدا_کړه('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    إذا (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.اندازه_2();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.ټوټه(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t پیل = Part.پیدا_کړه_لومړی_not_د(" \t\r\n");
    إذا (پیل != StringRef::npos) {
      size_t پای_ټکی_2 = Part.پیدا_کړه_وروستی_not_د(" \t\r\n");
      نښې.ټېل_شا(Part.ټوټه(پیل, پای_ټکی_2 + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

ساكن Accessor وېشل_Mustache_String(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  Small_ویکټور<StringRef> نښې;
  إذا (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    نښې.ټېل_شا(".");
  } وإلا {
    وېشل_And_Trim(Str, نښې);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *Arena_نښې = Ctx.Allocator.Allocate<StringRef>(نښې.اندازه_2());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(نښې, Arena_نښې);
  // Return an ArrayRef pointing to the stable arena memory.
  أرجع ArrayRef<StringRef>(Arena_نښې, نښې.اندازه_2());
}
} // namespace

نطاق llvm::mustache {

صنف Mustache_Output_بهیر : عام raw_ostream {
عام:
  Mustache_Output_بهیر() = افتراضي;
  ~Mustache_Output_بهیر() override = افتراضي;

  مجازي فراغ suspendIndentation() {}
  مجازي فراغ resumeIndentation() {}

خاص:
  فراغ anchor() override;
};

فراغ Mustache_Output_بهیر::anchor() {}

صنف Raw_Mustache_Output_بهیر : عام Mustache_Output_بهیر {
عام:
  Raw_Mustache_Output_بهیر(raw_ostream &OS) : OS(OS) { سېټ_Unbuffered(); }

خاص:
  raw_ostream &OS;

  فراغ write_impl(ثابت حرف *Ptr, size_t اندازه) override {
    OS.اكتب(Ptr, اندازه);
  }
  uint64_t current_pos() ثابت override { أرجع OS.tell(); }
};

صنف Token {
عام:
  تعداد صنف Type {
    متن_2,
    Variable,
    Partial,
    SectionOpen,
    SectionClose,
    InvertSectionOpen,
    UnescapeVariable,
    Comment,
    سېټ_Delimiter,
  };

  Token(StringRef Str)
      : TokenType(Type::متن_2), RawBody(Str), TokenBody(RawBody),
        Accessor_ارزښت({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, حرف Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    إذا (TokenType == Type::Comment)
      أرجع;
    StringRef AccessorStr(هذا->TokenBody);
    إذا (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_ارزښت = وېشل_Mustache_String(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ثابت { أرجع Accessor_ارزښت; }

  Type getType() ثابت { أرجع TokenType; }

  فراغ سېټ_Indentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() ثابت { أرجع Indentation; }

  ساكن Type getTokenType(حرف Identifier) {
    بدل (Identifier) {
    حالة '#':
      أرجع Type::SectionOpen;
    حالة '/':
      أرجع Type::SectionClose;
    حالة '^':
      أرجع Type::InvertSectionOpen;
    حالة '!':
      أرجع Type::Comment;
    حالة '>':
      أرجع Type::Partial;
    حالة '&':
      أرجع Type::UnescapeVariable;
    حالة '=':
      أرجع Type::سېټ_Delimiter;
    افتراضي:
      أرجع Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_ارزښت;
  size_t Indentation;
};

کارول EscapeMap = DenseMap<حرف, std::string>;

صنف ASTNode : عام ilist_غوټه<ASTNode> {
عام:
  تعداد Type {
    ریښه_2,
    متن_2,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::ریښه_2), پلار_2(تش_اشاره), پلار_Context(تش_اشاره) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *پلار_2)
      : Ctx(Ctx), Ty(Type::متن_2), Body(Body), پلار_2(پلار_2),
        پلار_Context(تش_اشاره) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *پلار_2)
      : Ctx(Ctx), Ty(Ty), پلار_2(پلار_2), Accessor_ارزښت(Accessor),
        پلار_Context(تش_اشاره) {}

  فراغ جمع_کړه_ماشوم(AstPtr ماشوم) { Children.ټېل_شا(ماشوم); };

  فراغ سېټ_Raw_Body(StringRef NewBody) { RawBody = NewBody; };

  فراغ سېټ_Indentation(size_t NewIndentation) { Indentation = NewIndentation; };

  فراغ render(ثابت llvm::json::ارزښت &معلومات_2, Mustache_Output_بهیر &OS);

خاص:
  فراغ renderLambdas(ثابت llvm::json::ارزښت &Contexts,
                     Mustache_Output_بهیر &OS, Lambda &L);

  فراغ renderSectionLambdas(ثابت llvm::json::ارزښت &Contexts,
                            Mustache_Output_بهیر &OS, SectionLambda &L);

  فراغ renderPartial(ثابت llvm::json::ارزښت &Contexts,
                     Mustache_Output_بهیر &OS, ASTNode *Partial);

  فراغ render_ماشوم(ثابت llvm::json::ارزښت &Context, Mustache_Output_بهیر &OS);

  ثابت llvm::json::ارزښت *پیدا_کړه_Context();

  فراغ render_ریښه(ثابت json::ارزښت &CurrentCtx, Mustache_Output_بهیر &OS);
  فراغ render_متن(Mustache_Output_بهیر &OS);
  فراغ renderPartial(ثابت json::ارزښت &CurrentCtx, Mustache_Output_بهیر &OS);
  فراغ renderVariable(ثابت json::ارزښت &CurrentCtx, Mustache_Output_بهیر &OS);
  فراغ renderUnescapeVariable(ثابت json::ارزښت &CurrentCtx,
                              Mustache_Output_بهیر &OS);
  فراغ renderSection(ثابت json::ارزښت &CurrentCtx, Mustache_Output_بهیر &OS);
  فراغ renderInvertSection(ثابت json::ارزښت &CurrentCtx,
                           Mustache_Output_بهیر &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *پلار_2;
  ASTNodeList Children;
  ثابت ArrayRef<StringRef> Accessor_ارزښت;
  ثابت llvm::json::ارزښت *پلار_Context;
};

// A wrapper for arena allocator for ASTNodes
ساكن AstPtr create_ریښه_غوټه(MustacheContext &Ctx) {
  أرجع جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

ساكن AstPtr create_غوټه(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *پلار_2) {
  أرجع جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, پلار_2);
}

ساكن AstPtr create_متن_غوټه(MustacheContext &Ctx, StringRef Body,
                             ASTNode *پلار_2) {
  أرجع جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, پلار_2);
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
ساكن منطقي has_متن_Behind(size_t Idx, ثابت ArrayRef<Token> &نښې) {
  إذا (Idx == 0)
    أرجع ریښتیا;

  size_t PrevIdx = Idx - 1;
  إذا (نښې[PrevIdx].getType() != Token::Type::متن_2)
    أرجع ریښتیا;

  ثابت Token &PrevToken = نښې[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  أرجع !TokenBody.پایونه_with("\n") && !(TokenBody.تشه() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
ساكن منطقي has_متن_Ahead(size_t Idx, ثابت ArrayRef<Token> &نښې) {
  إذا (Idx >= نښې.اندازه_2() - 1)
    أرجع ریښتیا;

  size_t بل_Idx = Idx + 1;
  إذا (نښې[بل_Idx].getType() != Token::Type::متن_2)
    أرجع ریښتیا;

  ثابت Token &بل_Token = نښې[بل_Idx];
  StringRef TokenBody = StringRef(بل_Token.RawBody).ltrim(" ");
  أرجع !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

ساكن منطقي requires_Clean_پورته(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  أرجع T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::سېټ_Delimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
ساكن فراغ stripTokenAhead(Small_ویکټور_Impl<Token> &نښې, size_t Idx) {
  Token &بل_Token = نښې[Idx + 1];
  StringRef بل_Token_Body = بل_Token.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  إذا (بل_Token_Body.starts_with("\r\n"))
    بل_Token.TokenBody = بل_Token_Body.substr(2);
  وإلا إذا (بل_Token_Body.starts_with("\n"))
    بل_Token.TokenBody = بل_Token_Body.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
فراغ stripTokenBefore(Small_ویکټور_Impl<Token> &نښې, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = نښې[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.اندازه_2() - Unindented.اندازه_2();
  PrevToken.TokenBody = Unindented;
  CurrentToken.سېټ_Indentation(Indentation);
}

بنية نښان {
  تعداد صنف ډول_2 {
    None,
    نورمال, // {{...}}
    Triple, // {{{...}}}
  };

  ډول_2 نښان_ډول = ډول_2::None;
  StringRef Content;   // The content between the delimiters.
  StringRef ډک_سمون; // The entire tag, including delimiters.
  size_t پیل_Position = StringRef::npos;
};

[[maybe_ناکارول_شوی]] ساكن ثابت حرف *نښان_ډول_منزل_String(نښان::ډول_2 K) {
  بدل (K) {
  حالة نښان::ډول_2::None:
    أرجع "None";
  حالة نښان::ډول_2::نورمال:
    أرجع "Normal";
  حالة نښان::ډول_2::Triple:
    أرجع "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_ناکارول_شوی]] ساكن ثابت حرف *json_ډول_منزل_String(json::ارزښت::ډول_2 K) {
  بدل (K) {
  حالة json::ارزښت::ډول_2::Null:
    أرجع "JSON_KIND_NULL";
  حالة json::ارزښت::ډول_2::Boolean:
    أرجع "JSON_KIND_BOOLEAN";
  حالة json::ارزښت::ډول_2::شمېره:
    أرجع "JSON_KIND_NUMBER";
  حالة json::ارزښت::ډول_2::String:
    أرجع "JSON_KIND_STRING";
  حالة json::ارزښت::ډول_2::Array:
    أرجع "JSON_KIND_ARRAY";
  حالة json::ارزښت::ډول_2::Object:
    أرجع "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
ساكن Small_ویکټور<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  Small_ویکټور<Token> نښې;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t متن_پیل = 0;

  ثابت StringLiteral TripleOpen("{{{");
  ثابت StringLiteral TripleClose("}}}");

  طالما (Cursor < Template.اندازه_2()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef نښان_Open, نښان_Close;
    نښان::ډول_2 ډول_2;

    // Determine which tag we've encountered.
    إذا (TemplateSuffix.starts_with(TripleOpen)) {
      ډول_2 = نښان::ډول_2::Triple;
      نښان_Open = TripleOpen;
      نښان_Close = TripleClose;
    } وإلا إذا (TemplateSuffix.starts_with(Open)) {
      ډول_2 = نښان::ډول_2::نورمال;
      نښان_Open = Open;
      نښان_Close = Close;
    } وإلا {
      // Not at a tag, continue scanning.
      ++Cursor;
      استمر;
    }

    // Found a tag, first add the preceding text.
    إذا (Cursor > متن_پیل)
      نښې.emplace_شا(Template.ټوټه(متن_پیل, Cursor));

    // Find the closing tag.
    size_t پای_ټکی_Pos = Template.پیدا_کړه(نښان_Close, Cursor + نښان_Open.اندازه_2());
    إذا (پای_ټکی_Pos == StringRef::npos) {
      // No closing tag, the rest is text.
      نښې.emplace_شا(Template.substr(Cursor));
      متن_پیل = Cursor = Template.اندازه_2();
      اكسر;
    }

    // Extract tag content and full match.
    size_t Content_پیل = Cursor + نښان_Open.اندازه_2();
    StringRef Content = Template.substr(Content_پیل, پای_ټکی_Pos - Content_پیل);
    StringRef ډک_سمون =
        Template.substr(Cursor, (پای_ټکی_Pos + نښان_Close.اندازه_2()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << ډک_سمون << ", Content: " << Content
                      << ", Kind: " << نښان_ډول_منزل_String(ډول_2) << "\n");
    إذا (ډول_2 == نښان::ډول_2::Triple) {
      نښې.emplace_شا(ډک_سمون, Ctx.Saver.save("&" + Content), '&', Ctx);
    } وإلا { // Normal Tag
      StringRef Interpolated = Content;
      إذا (!Interpolated.trim().starts_with("=")) {
        حرف Front = Interpolated.تشه() ? ' ' : Interpolated.trim().front();
        نښې.emplace_شا(ډک_سمون, Interpolated, Front, Ctx);
      } وإلا { // Set Delimiter
        نښې.emplace_شا(ډک_سمون, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](حرف C) { أرجع C == '='; });
        DelimSpec = DelimSpec.trim();

        خپلکاری [NewOpen, NewClose] = DelimSpec.وېشل(' ');
        LLVM_DEBUG(dbgs() << "[Set Delimiter] NewOpen: " << NewOpen
                          << ", NewClose: " << NewClose << "\n");
        Open = NewOpen;
        Close = NewClose;
      }
    }

    // Move past the tag for the next iteration.
    Cursor += ډک_سمون.اندازه_2();
    متن_پیل = Cursor;
  }

  // Add any remaining text after the last tag.
  إذا (متن_پیل < Template.اندازه_2())
    نښې.emplace_شا(Template.substr(متن_پیل));

  // Fix up white spaces for standalone tags.
  size_t وروستی_Idx = نښې.اندازه_2() - 1;
  لكل (size_t Idx = 0, پای_ټکی_2 = نښې.اندازه_2(); Idx < پای_ټکی_2; ++Idx) {
    Token &CurrentToken = نښې[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    إذا (!requires_Clean_پورته(CurrentType))
      استمر;

    منطقي Has_متن_Behind = has_متن_Behind(Idx, نښې);
    منطقي Has_متن_Ahead = has_متن_Ahead(Idx, نښې);

    إذا ((!Has_متن_Ahead && !Has_متن_Behind) || (!Has_متن_Ahead && Idx == 0))
      stripTokenAhead(نښې, Idx);

    إذا ((!Has_متن_Behind && !Has_متن_Ahead) || (!Has_متن_Behind && Idx == وروستی_Idx))
      stripTokenBefore(نښې, Idx, CurrentToken, CurrentType);
  }
  أرجع نښې;
}

// Custom stream to escape strings.
صنف Escape_String_بهیر : عام Mustache_Output_بهیر {
عام:
  څرګند Escape_String_بهیر(llvm::raw_ostream &Wrapped_بهیر,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.کلیګانې().begin(), Escape.کلیګانې().پای_ټکی()),
        Wrapped_بهیر(Wrapped_بهیر) {
    سېټ_Unbuffered();
  }

محمي:
  فراغ write_impl(ثابت حرف *Ptr, size_t اندازه) override {
    StringRef معلومات_2(Ptr, اندازه);
    size_t پیل = 0;
    طالما (پیل < اندازه) {
      // Find the next character that needs to be escaped.
      size_t بل_2 = معلومات_2.پیدا_کړه_لومړی_د(EscapeChars.str(), پیل);

      // If no escapable characters are found, write the rest of the string.
      إذا (بل_2 == StringRef::npos) {
        Wrapped_بهیر << معلومات_2.substr(پیل);
        أرجع;
      }

      // Write the chunk of text before the escapable character.
      إذا (بل_2 > پیل)
        Wrapped_بهیر << معلومات_2.substr(پیل, بل_2 - پیل);

      // Look up and write the escaped version of the character.
      Wrapped_بهیر << Escape[معلومات_2[بل_2]];
      پیل = بل_2 + 1;
    }
  }

  uint64_t current_pos() ثابت override { أرجع Wrapped_بهیر.tell(); }

خاص:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &Wrapped_بهیر;
};

// Custom stream to add indentation used to for rendering partials.
صنف جمع_کړه_Indentation_String_بهیر : عام Mustache_Output_بهیر {
عام:
  څرګند جمع_کړه_Indentation_String_بهیر(raw_ostream &Wrapped_بهیر,
                                      size_t Indentation)
      : Indentation(Indentation), Wrapped_بهیر(Wrapped_بهیر),
        NeedsIndent(ریښتیا), IsSuspended(خطأ) {
    سېټ_Unbuffered();
  }

  فراغ suspendIndentation() override { IsSuspended = ریښتیا; }
  فراغ resumeIndentation() override { IsSuspended = خطأ; }

محمي:
  فراغ write_impl(ثابت حرف *Ptr, size_t اندازه) override {
    llvm::StringRef معلومات_2(Ptr, اندازه);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    لكل (حرف C : معلومات_2) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      إذا (NeedsIndent && C != '\n') {
        Wrapped_بهیر << Indent;
        NeedsIndent = خطأ;
      }
      Wrapped_بهیر << C;
      إذا (C == '\n' && !IsSuspended)
        NeedsIndent = ریښتیا;
    }
  }

  uint64_t current_pos() ثابت override { أرجع Wrapped_بهیر.tell(); }

خاص:
  size_t Indentation;
  raw_ostream &Wrapped_بهیر;
  منطقي NeedsIndent;
  منطقي IsSuspended;
};

صنف Parser {
عام:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

خاص:
  فراغ parseMustache(ASTNode *پلار_2);
  فراغ parseSection(ASTNode *پلار_2, ASTNode::Type Ty, ثابت Accessor &A);

  MustacheContext &Ctx;
  Small_ویکټور<Token> نښې;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

فراغ Parser::parseSection(ASTNode *پلار_2, ASTNode::Type Ty,
                          ثابت Accessor &A) {
  AstPtr Current_غوټه = create_غوټه(Ctx, Ty, A, پلار_2);
  size_t پیل = CurrentPtr;
  parseMustache(Current_غوټه);
  ثابت size_t پای_ټکی_2 = CurrentPtr - 1;

  size_t Raw_Body_اندازه = 0;
  لكل (size_t I = پیل; I < پای_ټکی_2; ++I)
    Raw_Body_اندازه += نښې[I].RawBody.اندازه_2();

  SmallString<128> RawBody;
  RawBody.reserve(Raw_Body_اندازه);
  لكل (std::size_t I = پیل; I < پای_ټکی_2; ++I)
    RawBody += نښې[I].RawBody;

  Current_غوټه->سېټ_Raw_Body(Ctx.Saver.save(StringRef(RawBody)));
  پلار_2->جمع_کړه_ماشوم(Current_غوټه);
}

AstPtr Parser::parse() {
  نښې = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr ریښه_غوټه = create_ریښه_غوټه(Ctx);
  parseMustache(ریښه_غوټه);
  أرجع ریښه_غوټه;
}

فراغ Parser::parseMustache(ASTNode *پلار_2) {

  طالما (CurrentPtr < نښې.اندازه_2()) {
    Token CurrentToken = نښې[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_غوټه;

    بدل (CurrentToken.getType()) {
    حالة Token::Type::متن_2: {
      Current_غوټه = create_متن_غوټه(Ctx, CurrentToken.TokenBody, پلار_2);
      پلار_2->جمع_کړه_ماشوم(Current_غوټه);
      اكسر;
    }
    حالة Token::Type::Variable: {
      Current_غوټه = create_غوټه(Ctx, ASTNode::Variable, A, پلار_2);
      پلار_2->جمع_کړه_ماشوم(Current_غوټه);
      اكسر;
    }
    حالة Token::Type::UnescapeVariable: {
      Current_غوټه = create_غوټه(Ctx, ASTNode::UnescapeVariable, A, پلار_2);
      پلار_2->جمع_کړه_ماشوم(Current_غوټه);
      اكسر;
    }
    حالة Token::Type::Partial: {
      Current_غوټه = create_غوټه(Ctx, ASTNode::Partial, A, پلار_2);
      Current_غوټه->سېټ_Indentation(CurrentToken.getIndentation());
      پلار_2->جمع_کړه_ماشوم(Current_غوټه);
      اكسر;
    }
    حالة Token::Type::SectionOpen: {
      parseSection(پلار_2, ASTNode::Section, A);
      اكسر;
    }
    حالة Token::Type::InvertSectionOpen: {
      parseSection(پلار_2, ASTNode::InvertSection, A);
      اكسر;
    }
    حالة Token::Type::Comment:
    حالة Token::Type::سېټ_Delimiter:
      اكسر;
    حالة Token::Type::SectionClose:
      أرجع;
    }
  }
}
ساكن فراغ منزل_Mustache_String(ثابت json::ارزښت &معلومات_2, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << json_ډول_منزل_String(معلومات_2.ډول()) << ", Data: " << معلومات_2
                    << "\n");
  بدل (معلومات_2.ډول()) {
  حالة json::ارزښت::Null:
    أرجع;
  حالة json::ارزښت::شمېره: {
    خپلکاری عدد = *معلومات_2.get_As_شمېره();
    std::ostringstream SS;
    SS << عدد;
    OS << SS.str();
    أرجع;
  }
  حالة json::ارزښت::String: {
    OS << *معلومات_2.getAsString();
    أرجع;
  }

  حالة json::ارزښت::Array: {
    خپلکاری Arr = *معلومات_2.getAsArray();
    إذا (Arr.تشه())
      أرجع;
    [[fallthrough]];
  }
  حالة json::ارزښت::Object:
  حالة json::ارزښت::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.ارزښت_2(معلومات_2);
    اكسر;
  }
  }
}

فراغ ASTNode::render_ریښه(ثابت json::ارزښت &CurrentCtx,
                         Mustache_Output_بهیر &OS) {
  render_ماشوم(CurrentCtx, OS);
}

فراغ ASTNode::render_متن(Mustache_Output_بهیر &OS) { OS << Body; }

فراغ ASTNode::renderPartial(ثابت json::ارزښت &CurrentCtx,
                            Mustache_Output_بهیر &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_ارزښت[0]
                    << ", Indentation:" << Indentation << "\n");
  خپلکاری Partial = Ctx.Partials.پیدا_کړه(Accessor_ارزښت[0]);
  إذا (Partial != Ctx.Partials.پای_ټکی())
    renderPartial(CurrentCtx, OS, Partial->get_ارزښت());
}

فراغ ASTNode::renderVariable(ثابت json::ارزښت &CurrentCtx,
                             Mustache_Output_بهیر &OS) {
  خپلکاری Lambda = Ctx.Lambdas.پیدا_کړه(Accessor_ارزښت[0]);
  إذا (Lambda != Ctx.Lambdas.پای_ټکی()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_ارزښت());
  } وإلا إذا (ثابت json::ارزښت *ContextPtr = پیدا_کړه_Context()) {
    Escape_String_بهیر ES(OS, Ctx.Escapes);
    منزل_Mustache_String(*ContextPtr, ES);
  }
}

فراغ ASTNode::renderUnescapeVariable(ثابت json::ارزښت &CurrentCtx,
                                     Mustache_Output_بهیر &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_ارزښت[0]
                    << "\n");
  خپلکاری Lambda = Ctx.Lambdas.پیدا_کړه(Accessor_ارزښت[0]);
  إذا (Lambda != Ctx.Lambdas.پای_ټکی()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_ارزښت());
  } وإلا إذا (ثابت json::ارزښت *ContextPtr = پیدا_کړه_Context()) {
    OS.suspendIndentation();
    منزل_Mustache_String(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

فراغ ASTNode::renderSection(ثابت json::ارزښت &CurrentCtx,
                            Mustache_Output_بهیر &OS) {
  خپلکاری SectionLambda = Ctx.SectionLambdas.پیدا_کړه(Accessor_ارزښت[0]);
  إذا (SectionLambda != Ctx.SectionLambdas.پای_ټکی()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_ارزښت());
    أرجع;
  }

  ثابت json::ارزښت *ContextPtr = پیدا_کړه_Context();
  إذا (isContextFalsey(ContextPtr))
    أرجع;

  إذا (ثابت json::Array *Arr = ContextPtr->getAsArray()) {
    لكل (ثابت json::ارزښت &V : *Arr)
      render_ماشوم(V, OS);
    أرجع;
  }
  render_ماشوم(*ContextPtr, OS);
}

فراغ ASTNode::renderInvertSection(ثابت json::ارزښت &CurrentCtx,
                                  Mustache_Output_بهیر &OS) {
  منطقي IsLambda = Ctx.SectionLambdas.لري(Accessor_ارزښت[0]);
  ثابت json::ارزښت *ContextPtr = پیدا_کړه_Context();
  إذا (isContextFalsey(ContextPtr) && !IsLambda) {
    render_ماشوم(CurrentCtx, OS);
  }
}

فراغ ASTNode::render(ثابت llvm::json::ارزښت &معلومات_2, Mustache_Output_بهیر &OS) {
  إذا (Ty != ریښه_2 && Ty != متن_2 && Accessor_ارزښت.تشه())
    أرجع;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  پلار_Context = &معلومات_2;

  بدل (Ty) {
  حالة ریښه_2:
    render_ریښه(معلومات_2, OS);
    أرجع;
  حالة متن_2:
    render_متن(OS);
    أرجع;
  حالة Partial:
    renderPartial(معلومات_2, OS);
    أرجع;
  حالة Variable:
    renderVariable(معلومات_2, OS);
    أرجع;
  حالة UnescapeVariable:
    renderUnescapeVariable(معلومات_2, OS);
    أرجع;
  حالة Section:
    renderSection(معلومات_2, OS);
    أرجع;
  حالة InvertSection:
    renderInvertSection(معلومات_2, OS);
    أرجع;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ثابت json::ارزښت *ASTNode::پیدا_کړه_Context() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  إذا (Accessor_ارزښت.تشه())
    أرجع تش_اشاره;
  إذا (Accessor_ارزښت[0] == ".")
    أرجع پلار_Context;

  ثابت json::Object *CurrentContext = پلار_Context->getAsObject();
  StringRef CurrentAccessor = Accessor_ارزښت[0];
  ASTNode *Current_پلار = پلار_2;

  طالما (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    إذا (Current_پلار->Ty != ریښه_2) {
      CurrentContext = Current_پلار->پلار_Context->getAsObject();
      Current_پلار = Current_پلار->پلار_2;
      استمر;
    }
    أرجع تش_اشاره;
  }
  ثابت json::ارزښت *Context = تش_اشاره;
  لكل (خپلکاری [Idx, Acc] : enumerate(Accessor_ارزښت)) {
    ثابت json::ارزښت *Current_ارزښت = CurrentContext->get(Acc);
    إذا (!Current_ارزښت)
      أرجع تش_اشاره;
    إذا (Idx < Accessor_ارزښت.اندازه_2() - 1) {
      CurrentContext = Current_ارزښت->getAsObject();
      إذا (!CurrentContext)
        أرجع تش_اشاره;
    } وإلا {
      Context = Current_ارزښت;
    }
  }
  أرجع Context;
}

فراغ ASTNode::render_ماشوم(ثابت json::ارزښت &Contexts,
                          Mustache_Output_بهیر &OS) {
  لكل (ASTNode &ماشوم : Children)
    ماشوم.render(Contexts, OS);
}

فراغ ASTNode::renderPartial(ثابت json::ارزښت &Contexts,
                            Mustache_Output_بهیر &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  جمع_کړه_Indentation_String_بهیر IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

فراغ ASTNode::renderLambdas(ثابت llvm::json::ارزښت &Contexts,
                            Mustache_Output_بهیر &OS, Lambda &L) {
  json::ارزښت Lambda_پایله = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  منزل_Mustache_String(Lambda_پایله, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_غوټه = P.parse();

  Escape_String_بهیر ES(OS, Ctx.Escapes);
  إذا (Ty == Variable) {
    Lambda_غوټه->render(Contexts, ES);
    أرجع;
  }
  Lambda_غوټه->render(Contexts, OS);
}

فراغ ASTNode::renderSectionLambdas(ثابت llvm::json::ارزښت &Contexts,
                                   Mustache_Output_بهیر &OS, SectionLambda &L) {
  json::ارزښت Return = L(RawBody.str());
  إذا (isFalsey(Return))
    أرجع;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  منزل_Mustache_String(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_غوټه = P.parse();
  Lambda_غوټه->render(Contexts, OS);
}

فراغ Template::render(ثابت llvm::json::ارزښت &معلومات_2, llvm::raw_ostream &OS) {
  Raw_Mustache_Output_بهیر MOS(OS);
  ونه->render(معلومات_2, MOS);
}

فراغ Template::ثبتوونکی_Partial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_ونه = P.parse();
  Ctx.Partials.ورننباسه(std::make_pair(Name, Partial_ونه));
}

فراغ Template::ثبتوونکی_Lambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

فراغ Template::ثبتوونکی_Lambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

فراغ Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  ونه = P.parse();
  // The default behavior is to escape html entities.
  ثابت EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), ونه(Other.ونه) {
  Other.ونه = تش_اشاره;
}

Template::~Template() = افتراضي;

} // namespace llvm::mustache

#تعریف_لغوه DEBUG_TYPE
