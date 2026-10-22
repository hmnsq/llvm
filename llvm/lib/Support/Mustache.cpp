//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#అన్తర్భావ "llvm/Support/Mustache.h"
#అన్తర్భావ "llvm/ADT/SmallVector.h"
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <sstream>

#నిర్వచించు DEBUG_TYPE "mustache"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::mustache;

నామాకాశ {

ఉపయోగించి Accessor = ArrayRef<StringRef>;

స్థిర తార్కిక isFalsey(స్థిరరాశి json::విలువ &V) {
  తిరిగి V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().విలువ_2()) ||
         (V.getAsArray() && V.getAsArray()->ఖాళీలు());
}

స్థిర తార్కిక isContextFalsey(స్థిరరాశి json::విలువ *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  ఒకవేళ (!V)
    తిరిగి నిజం;
  తిరిగి isFalsey(*V);
}

స్థిర శూన్య విభజించు_And_Trim(StringRef Str, Small_సదిశ_Impl<StringRef> &టోకెన్లు) {
  size_t CurrentPos = 0;
  అయితే (CurrentPos < Str.కొలత_2()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.కనుగొను('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    ఒకవేళ (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.కొలత_2();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.ముక్క(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t ప్రారంభం = Part.కనుగొను_మొదటి_not_యొక్క(" \t\r\n");
    ఒకవేళ (ప్రారంభం != StringRef::npos) {
      size_t చివర_2 = Part.కనుగొను_చివరి_not_యొక్క(" \t\r\n");
      టోకెన్లు.నెట్టు_వెనుక(Part.ముక్క(ప్రారంభం, చివర_2 + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

స్థిర Accessor విభజించు_Mustache_String(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  Small_సదిశ<StringRef> టోకెన్లు;
  ఒకవేళ (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    టోకెన్లు.నెట్టు_వెనుక(".");
  } లేకపోతే {
    విభజించు_And_Trim(Str, టోకెన్లు);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *Arena_టోకెన్లు = Ctx.Allocator.Allocate<StringRef>(టోకెన్లు.కొలత_2());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(టోకెన్లు, Arena_టోకెన్లు);
  // Return an ArrayRef pointing to the stable arena memory.
  తిరిగి ArrayRef<StringRef>(Arena_టోకెన్లు, టోకెన్లు.కొలత_2());
}
} // namespace

నామాకాశ llvm::mustache {

తరగతి Mustache_Output_ప్రవాహం : బహిరంగ raw_ostream {
బహిరంగ:
  Mustache_Output_ప్రవాహం() = అప్రమేయ;
  ~Mustache_Output_ప్రవాహం() override = అప్రమేయ;

  మాయా శూన్య suspendIndentation() {}
  మాయా శూన్య resumeIndentation() {}

అంతరంగ:
  శూన్య anchor() override;
};

శూన్య Mustache_Output_ప్రవాహం::anchor() {}

తరగతి Raw_Mustache_Output_ప్రవాహం : బహిరంగ Mustache_Output_ప్రవాహం {
బహిరంగ:
  Raw_Mustache_Output_ప్రవాహం(raw_ostream &OS) : OS(OS) { సమితి_Unbuffered(); }

అంతరంగ:
  raw_ostream &OS;

  శూన్య write_impl(స్థిరరాశి అక్షరం *Ptr, size_t కొలత) override {
    OS.లిఖ(Ptr, కొలత);
  }
  uint64_t current_pos() స్థిరరాశి override { తిరిగి OS.tell(); }
};

తరగతి Token {
బహిరంగ:
  గణన తరగతి Type {
    పాఠ్యం_2,
    Variable,
    Partial,
    SectionOpen,
    SectionClose,
    InvertSectionOpen,
    UnescapeVariable,
    Comment,
    సమితి_Delimiter,
  };

  Token(StringRef Str)
      : TokenType(Type::పాఠ్యం_2), RawBody(Str), TokenBody(RawBody),
        Accessor_విలువ({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, అక్షరం Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    ఒకవేళ (TokenType == Type::Comment)
      తిరిగి;
    StringRef AccessorStr(ఇది->TokenBody);
    ఒకవేళ (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_విలువ = విభజించు_Mustache_String(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() స్థిరరాశి { తిరిగి Accessor_విలువ; }

  Type getType() స్థిరరాశి { తిరిగి TokenType; }

  శూన్య సమితి_Indentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() స్థిరరాశి { తిరిగి Indentation; }

  స్థిర Type getTokenType(అక్షరం Identifier) {
    ఎంపిక (Identifier) {
    స్థితి '#':
      తిరిగి Type::SectionOpen;
    స్థితి '/':
      తిరిగి Type::SectionClose;
    స్థితి '^':
      తిరిగి Type::InvertSectionOpen;
    స్థితి '!':
      తిరిగి Type::Comment;
    స్థితి '>':
      తిరిగి Type::Partial;
    స్థితి '&':
      తిరిగి Type::UnescapeVariable;
    స్థితి '=':
      తిరిగి Type::సమితి_Delimiter;
    అప్రమేయ:
      తిరిగి Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_విలువ;
  size_t Indentation;
};

ఉపయోగించి EscapeMap = DenseMap<అక్షరం, std::string>;

తరగతి ASTNode : బహిరంగ ilist_కణుపు<ASTNode> {
బహిరంగ:
  గణన Type {
    మూలం_2,
    పాఠ్యం_2,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::మూలం_2), మాతృ_2(శూన్యసూచిక), మాతృ_Context(శూన్యసూచిక) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *మాతృ_2)
      : Ctx(Ctx), Ty(Type::పాఠ్యం_2), Body(Body), మాతృ_2(మాతృ_2),
        మాతృ_Context(శూన్యసూచిక) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *మాతృ_2)
      : Ctx(Ctx), Ty(Ty), మాతృ_2(మాతృ_2), Accessor_విలువ(Accessor),
        మాతృ_Context(శూన్యసూచిక) {}

  శూన్య కూడు_బిడ్డ(AstPtr బిడ్డ) { Children.నెట్టు_వెనుక(బిడ్డ); };

  శూన్య సమితి_Raw_Body(StringRef NewBody) { RawBody = NewBody; };

  శూన్య సమితి_Indentation(size_t NewIndentation) { Indentation = NewIndentation; };

  శూన్య render(స్థిరరాశి llvm::json::విలువ &దత్తాంశం_2, Mustache_Output_ప్రవాహం &OS);

అంతరంగ:
  శూన్య renderLambdas(స్థిరరాశి llvm::json::విలువ &Contexts,
                     Mustache_Output_ప్రవాహం &OS, Lambda &L);

  శూన్య renderSectionLambdas(స్థిరరాశి llvm::json::విలువ &Contexts,
                            Mustache_Output_ప్రవాహం &OS, SectionLambda &L);

  శూన్య renderPartial(స్థిరరాశి llvm::json::విలువ &Contexts,
                     Mustache_Output_ప్రవాహం &OS, ASTNode *Partial);

  శూన్య render_బిడ్డ(స్థిరరాశి llvm::json::విలువ &Context, Mustache_Output_ప్రవాహం &OS);

  స్థిరరాశి llvm::json::విలువ *కనుగొను_Context();

  శూన్య render_మూలం(స్థిరరాశి json::విలువ &CurrentCtx, Mustache_Output_ప్రవాహం &OS);
  శూన్య render_పాఠ్యం(Mustache_Output_ప్రవాహం &OS);
  శూన్య renderPartial(స్థిరరాశి json::విలువ &CurrentCtx, Mustache_Output_ప్రవాహం &OS);
  శూన్య renderVariable(స్థిరరాశి json::విలువ &CurrentCtx, Mustache_Output_ప్రవాహం &OS);
  శూన్య renderUnescapeVariable(స్థిరరాశి json::విలువ &CurrentCtx,
                              Mustache_Output_ప్రవాహం &OS);
  శూన్య renderSection(స్థిరరాశి json::విలువ &CurrentCtx, Mustache_Output_ప్రవాహం &OS);
  శూన్య renderInvertSection(స్థిరరాశి json::విలువ &CurrentCtx,
                           Mustache_Output_ప్రవాహం &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *మాతృ_2;
  ASTNodeList Children;
  స్థిరరాశి ArrayRef<StringRef> Accessor_విలువ;
  స్థిరరాశి llvm::json::విలువ *మాతృ_Context;
};

// A wrapper for arena allocator for ASTNodes
స్థిర AstPtr create_మూలం_కణుపు(MustacheContext &Ctx) {
  తిరిగి కొత్త (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

స్థిర AstPtr create_కణుపు(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *మాతృ_2) {
  తిరిగి కొత్త (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, మాతృ_2);
}

స్థిర AstPtr create_పాఠ్యం_కణుపు(MustacheContext &Ctx, StringRef Body,
                             ASTNode *మాతృ_2) {
  తిరిగి కొత్త (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, మాతృ_2);
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
స్థిర తార్కిక has_పాఠ్యం_Behind(size_t Idx, స్థిరరాశి ArrayRef<Token> &టోకెన్లు) {
  ఒకవేళ (Idx == 0)
    తిరిగి నిజం;

  size_t PrevIdx = Idx - 1;
  ఒకవేళ (టోకెన్లు[PrevIdx].getType() != Token::Type::పాఠ్యం_2)
    తిరిగి నిజం;

  స్థిరరాశి Token &PrevToken = టోకెన్లు[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  తిరిగి !TokenBody.చివర్లు_with("\n") && !(TokenBody.ఖాళీలు() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
స్థిర తార్కిక has_పాఠ్యం_Ahead(size_t Idx, స్థిరరాశి ArrayRef<Token> &టోకెన్లు) {
  ఒకవేళ (Idx >= టోకెన్లు.కొలత_2() - 1)
    తిరిగి నిజం;

  size_t తదుపరి_Idx = Idx + 1;
  ఒకవేళ (టోకెన్లు[తదుపరి_Idx].getType() != Token::Type::పాఠ్యం_2)
    తిరిగి నిజం;

  స్థిరరాశి Token &తదుపరి_Token = టోకెన్లు[తదుపరి_Idx];
  StringRef TokenBody = StringRef(తదుపరి_Token.RawBody).ltrim(" ");
  తిరిగి !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

స్థిర తార్కిక requires_Clean_పైకి(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  తిరిగి T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::సమితి_Delimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
స్థిర శూన్య stripTokenAhead(Small_సదిశ_Impl<Token> &టోకెన్లు, size_t Idx) {
  Token &తదుపరి_Token = టోకెన్లు[Idx + 1];
  StringRef తదుపరి_Token_Body = తదుపరి_Token.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  ఒకవేళ (తదుపరి_Token_Body.starts_with("\r\n"))
    తదుపరి_Token.TokenBody = తదుపరి_Token_Body.substr(2);
  లేకపోతే ఒకవేళ (తదుపరి_Token_Body.starts_with("\n"))
    తదుపరి_Token.TokenBody = తదుపరి_Token_Body.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
శూన్య stripTokenBefore(Small_సదిశ_Impl<Token> &టోకెన్లు, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = టోకెన్లు[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.కొలత_2() - Unindented.కొలత_2();
  PrevToken.TokenBody = Unindented;
  CurrentToken.సమితి_Indentation(Indentation);
}

నిర్మాణం ట్యాగ్ {
  గణన తరగతి రకం_3 {
    None,
    సాధారణ, // {{...}}
    Triple, // {{{...}}}
  };

  రకం_3 ట్యాగ్_రకం = రకం_3::None;
  StringRef Content;   // The content between the delimiters.
  StringRef నిండిన_సరిపోలిక; // The entire tag, including delimiters.
  size_t ప్రారంభం_Position = StringRef::npos;
};

[[maybe_వాడని]] స్థిర స్థిరరాశి అక్షరం *ట్యాగ్_రకం_గమ్యం_String(ట్యాగ్::రకం_3 K) {
  ఎంపిక (K) {
  స్థితి ట్యాగ్::రకం_3::None:
    తిరిగి "None";
  స్థితి ట్యాగ్::రకం_3::సాధారణ:
    తిరిగి "Normal";
  స్థితి ట్యాగ్::రకం_3::Triple:
    తిరిగి "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_వాడని]] స్థిర స్థిరరాశి అక్షరం *json_రకం_గమ్యం_String(json::విలువ::రకం_3 K) {
  ఎంపిక (K) {
  స్థితి json::విలువ::రకం_3::Null:
    తిరిగి "JSON_KIND_NULL";
  స్థితి json::విలువ::రకం_3::Boolean:
    తిరిగి "JSON_KIND_BOOLEAN";
  స్థితి json::విలువ::రకం_3::సంఖ్య:
    తిరిగి "JSON_KIND_NUMBER";
  స్థితి json::విలువ::రకం_3::String:
    తిరిగి "JSON_KIND_STRING";
  స్థితి json::విలువ::రకం_3::Array:
    తిరిగి "JSON_KIND_ARRAY";
  స్థితి json::విలువ::రకం_3::Object:
    తిరిగి "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
స్థిర Small_సదిశ<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  Small_సదిశ<Token> టోకెన్లు;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t పాఠ్యం_ప్రారంభం = 0;

  స్థిరరాశి StringLiteral TripleOpen("{{{");
  స్థిరరాశి StringLiteral TripleClose("}}}");

  అయితే (Cursor < Template.కొలత_2()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef ట్యాగ్_Open, ట్యాగ్_Close;
    ట్యాగ్::రకం_3 రకం_3;

    // Determine which tag we've encountered.
    ఒకవేళ (TemplateSuffix.starts_with(TripleOpen)) {
      రకం_3 = ట్యాగ్::రకం_3::Triple;
      ట్యాగ్_Open = TripleOpen;
      ట్యాగ్_Close = TripleClose;
    } లేకపోతే ఒకవేళ (TemplateSuffix.starts_with(Open)) {
      రకం_3 = ట్యాగ్::రకం_3::సాధారణ;
      ట్యాగ్_Open = Open;
      ట్యాగ్_Close = Close;
    } లేకపోతే {
      // Not at a tag, continue scanning.
      ++Cursor;
      కొనసాగు;
    }

    // Found a tag, first add the preceding text.
    ఒకవేళ (Cursor > పాఠ్యం_ప్రారంభం)
      టోకెన్లు.emplace_వెనుక(Template.ముక్క(పాఠ్యం_ప్రారంభం, Cursor));

    // Find the closing tag.
    size_t చివర_Pos = Template.కనుగొను(ట్యాగ్_Close, Cursor + ట్యాగ్_Open.కొలత_2());
    ఒకవేళ (చివర_Pos == StringRef::npos) {
      // No closing tag, the rest is text.
      టోకెన్లు.emplace_వెనుక(Template.substr(Cursor));
      పాఠ్యం_ప్రారంభం = Cursor = Template.కొలత_2();
      ఆపు;
    }

    // Extract tag content and full match.
    size_t Content_ప్రారంభం = Cursor + ట్యాగ్_Open.కొలత_2();
    StringRef Content = Template.substr(Content_ప్రారంభం, చివర_Pos - Content_ప్రారంభం);
    StringRef నిండిన_సరిపోలిక =
        Template.substr(Cursor, (చివర_Pos + ట్యాగ్_Close.కొలత_2()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << నిండిన_సరిపోలిక << ", Content: " << Content
                      << ", Kind: " << ట్యాగ్_రకం_గమ్యం_String(రకం_3) << "\n");
    ఒకవేళ (రకం_3 == ట్యాగ్::రకం_3::Triple) {
      టోకెన్లు.emplace_వెనుక(నిండిన_సరిపోలిక, Ctx.Saver.save("&" + Content), '&', Ctx);
    } లేకపోతే { // Normal Tag
      StringRef Interpolated = Content;
      ఒకవేళ (!Interpolated.trim().starts_with("=")) {
        అక్షరం Front = Interpolated.ఖాళీలు() ? ' ' : Interpolated.trim().front();
        టోకెన్లు.emplace_వెనుక(నిండిన_సరిపోలిక, Interpolated, Front, Ctx);
      } లేకపోతే { // Set Delimiter
        టోకెన్లు.emplace_వెనుక(నిండిన_సరిపోలిక, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](అక్షరం C) { తిరిగి C == '='; });
        DelimSpec = DelimSpec.trim();

        స్వయంచాలక [NewOpen, NewClose] = DelimSpec.విభజించు(' ');
        LLVM_DEBUG(dbgs() << "[Set Delimiter] NewOpen: " << NewOpen
                          << ", NewClose: " << NewClose << "\n");
        Open = NewOpen;
        Close = NewClose;
      }
    }

    // Move past the tag for the next iteration.
    Cursor += నిండిన_సరిపోలిక.కొలత_2();
    పాఠ్యం_ప్రారంభం = Cursor;
  }

  // Add any remaining text after the last tag.
  ఒకవేళ (పాఠ్యం_ప్రారంభం < Template.కొలత_2())
    టోకెన్లు.emplace_వెనుక(Template.substr(పాఠ్యం_ప్రారంభం));

  // Fix up white spaces for standalone tags.
  size_t చివరి_Idx = టోకెన్లు.కొలత_2() - 1;
  కొరకు (size_t Idx = 0, చివర_2 = టోకెన్లు.కొలత_2(); Idx < చివర_2; ++Idx) {
    Token &CurrentToken = టోకెన్లు[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    ఒకవేళ (!requires_Clean_పైకి(CurrentType))
      కొనసాగు;

    తార్కిక Has_పాఠ్యం_Behind = has_పాఠ్యం_Behind(Idx, టోకెన్లు);
    తార్కిక Has_పాఠ్యం_Ahead = has_పాఠ్యం_Ahead(Idx, టోకెన్లు);

    ఒకవేళ ((!Has_పాఠ్యం_Ahead && !Has_పాఠ్యం_Behind) || (!Has_పాఠ్యం_Ahead && Idx == 0))
      stripTokenAhead(టోకెన్లు, Idx);

    ఒకవేళ ((!Has_పాఠ్యం_Behind && !Has_పాఠ్యం_Ahead) || (!Has_పాఠ్యం_Behind && Idx == చివరి_Idx))
      stripTokenBefore(టోకెన్లు, Idx, CurrentToken, CurrentType);
  }
  తిరిగి టోకెన్లు;
}

// Custom stream to escape strings.
తరగతి Escape_String_ప్రవాహం : బహిరంగ Mustache_Output_ప్రవాహం {
బహిరంగ:
  స్పష్టమైన Escape_String_ప్రవాహం(llvm::raw_ostream &Wrapped_ప్రవాహం,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.కీలకాలు().begin(), Escape.కీలకాలు().చివర()),
        Wrapped_ప్రవాహం(Wrapped_ప్రవాహం) {
    సమితి_Unbuffered();
  }

రక్షిత:
  శూన్య write_impl(స్థిరరాశి అక్షరం *Ptr, size_t కొలత) override {
    StringRef దత్తాంశం_2(Ptr, కొలత);
    size_t ప్రారంభం = 0;
    అయితే (ప్రారంభం < కొలత) {
      // Find the next character that needs to be escaped.
      size_t తదుపరి_2 = దత్తాంశం_2.కనుగొను_మొదటి_యొక్క(EscapeChars.str(), ప్రారంభం);

      // If no escapable characters are found, write the rest of the string.
      ఒకవేళ (తదుపరి_2 == StringRef::npos) {
        Wrapped_ప్రవాహం << దత్తాంశం_2.substr(ప్రారంభం);
        తిరిగి;
      }

      // Write the chunk of text before the escapable character.
      ఒకవేళ (తదుపరి_2 > ప్రారంభం)
        Wrapped_ప్రవాహం << దత్తాంశం_2.substr(ప్రారంభం, తదుపరి_2 - ప్రారంభం);

      // Look up and write the escaped version of the character.
      Wrapped_ప్రవాహం << Escape[దత్తాంశం_2[తదుపరి_2]];
      ప్రారంభం = తదుపరి_2 + 1;
    }
  }

  uint64_t current_pos() స్థిరరాశి override { తిరిగి Wrapped_ప్రవాహం.tell(); }

అంతరంగ:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &Wrapped_ప్రవాహం;
};

// Custom stream to add indentation used to for rendering partials.
తరగతి కూడు_Indentation_String_ప్రవాహం : బహిరంగ Mustache_Output_ప్రవాహం {
బహిరంగ:
  స్పష్టమైన కూడు_Indentation_String_ప్రవాహం(raw_ostream &Wrapped_ప్రవాహం,
                                      size_t Indentation)
      : Indentation(Indentation), Wrapped_ప్రవాహం(Wrapped_ప్రవాహం),
        NeedsIndent(నిజం), IsSuspended(అబద్ధం) {
    సమితి_Unbuffered();
  }

  శూన్య suspendIndentation() override { IsSuspended = నిజం; }
  శూన్య resumeIndentation() override { IsSuspended = అబద్ధం; }

రక్షిత:
  శూన్య write_impl(స్థిరరాశి అక్షరం *Ptr, size_t కొలత) override {
    llvm::StringRef దత్తాంశం_2(Ptr, కొలత);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    కొరకు (అక్షరం C : దత్తాంశం_2) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      ఒకవేళ (NeedsIndent && C != '\n') {
        Wrapped_ప్రవాహం << Indent;
        NeedsIndent = అబద్ధం;
      }
      Wrapped_ప్రవాహం << C;
      ఒకవేళ (C == '\n' && !IsSuspended)
        NeedsIndent = నిజం;
    }
  }

  uint64_t current_pos() స్థిరరాశి override { తిరిగి Wrapped_ప్రవాహం.tell(); }

అంతరంగ:
  size_t Indentation;
  raw_ostream &Wrapped_ప్రవాహం;
  తార్కిక NeedsIndent;
  తార్కిక IsSuspended;
};

తరగతి Parser {
బహిరంగ:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

అంతరంగ:
  శూన్య parseMustache(ASTNode *మాతృ_2);
  శూన్య parseSection(ASTNode *మాతృ_2, ASTNode::Type Ty, స్థిరరాశి Accessor &A);

  MustacheContext &Ctx;
  Small_సదిశ<Token> టోకెన్లు;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

శూన్య Parser::parseSection(ASTNode *మాతృ_2, ASTNode::Type Ty,
                          స్థిరరాశి Accessor &A) {
  AstPtr Current_కణుపు = create_కణుపు(Ctx, Ty, A, మాతృ_2);
  size_t ప్రారంభం = CurrentPtr;
  parseMustache(Current_కణుపు);
  స్థిరరాశి size_t చివర_2 = CurrentPtr - 1;

  size_t Raw_Body_కొలత = 0;
  కొరకు (size_t I = ప్రారంభం; I < చివర_2; ++I)
    Raw_Body_కొలత += టోకెన్లు[I].RawBody.కొలత_2();

  SmallString<128> RawBody;
  RawBody.reserve(Raw_Body_కొలత);
  కొరకు (std::size_t I = ప్రారంభం; I < చివర_2; ++I)
    RawBody += టోకెన్లు[I].RawBody;

  Current_కణుపు->సమితి_Raw_Body(Ctx.Saver.save(StringRef(RawBody)));
  మాతృ_2->కూడు_బిడ్డ(Current_కణుపు);
}

AstPtr Parser::parse() {
  టోకెన్లు = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr మూలం_కణుపు = create_మూలం_కణుపు(Ctx);
  parseMustache(మూలం_కణుపు);
  తిరిగి మూలం_కణుపు;
}

శూన్య Parser::parseMustache(ASTNode *మాతృ_2) {

  అయితే (CurrentPtr < టోకెన్లు.కొలత_2()) {
    Token CurrentToken = టోకెన్లు[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_కణుపు;

    ఎంపిక (CurrentToken.getType()) {
    స్థితి Token::Type::పాఠ్యం_2: {
      Current_కణుపు = create_పాఠ్యం_కణుపు(Ctx, CurrentToken.TokenBody, మాతృ_2);
      మాతృ_2->కూడు_బిడ్డ(Current_కణుపు);
      ఆపు;
    }
    స్థితి Token::Type::Variable: {
      Current_కణుపు = create_కణుపు(Ctx, ASTNode::Variable, A, మాతృ_2);
      మాతృ_2->కూడు_బిడ్డ(Current_కణుపు);
      ఆపు;
    }
    స్థితి Token::Type::UnescapeVariable: {
      Current_కణుపు = create_కణుపు(Ctx, ASTNode::UnescapeVariable, A, మాతృ_2);
      మాతృ_2->కూడు_బిడ్డ(Current_కణుపు);
      ఆపు;
    }
    స్థితి Token::Type::Partial: {
      Current_కణుపు = create_కణుపు(Ctx, ASTNode::Partial, A, మాతృ_2);
      Current_కణుపు->సమితి_Indentation(CurrentToken.getIndentation());
      మాతృ_2->కూడు_బిడ్డ(Current_కణుపు);
      ఆపు;
    }
    స్థితి Token::Type::SectionOpen: {
      parseSection(మాతృ_2, ASTNode::Section, A);
      ఆపు;
    }
    స్థితి Token::Type::InvertSectionOpen: {
      parseSection(మాతృ_2, ASTNode::InvertSection, A);
      ఆపు;
    }
    స్థితి Token::Type::Comment:
    స్థితి Token::Type::సమితి_Delimiter:
      ఆపు;
    స్థితి Token::Type::SectionClose:
      తిరిగి;
    }
  }
}
స్థిర శూన్య గమ్యం_Mustache_String(స్థిరరాశి json::విలువ &దత్తాంశం_2, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << json_రకం_గమ్యం_String(దత్తాంశం_2.రకం_2()) << ", Data: " << దత్తాంశం_2
                    << "\n");
  ఎంపిక (దత్తాంశం_2.రకం_2()) {
  స్థితి json::విలువ::Null:
    తిరిగి;
  స్థితి json::విలువ::సంఖ్య: {
    స్వయంచాలక అంకె = *దత్తాంశం_2.get_As_సంఖ్య();
    std::ostringstream SS;
    SS << అంకె;
    OS << SS.str();
    తిరిగి;
  }
  స్థితి json::విలువ::String: {
    OS << *దత్తాంశం_2.getAsString();
    తిరిగి;
  }

  స్థితి json::విలువ::Array: {
    స్వయంచాలక Arr = *దత్తాంశం_2.getAsArray();
    ఒకవేళ (Arr.ఖాళీలు())
      తిరిగి;
    [[fallthrough]];
  }
  స్థితి json::విలువ::Object:
  స్థితి json::విలువ::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.విలువ_2(దత్తాంశం_2);
    ఆపు;
  }
  }
}

శూన్య ASTNode::render_మూలం(స్థిరరాశి json::విలువ &CurrentCtx,
                         Mustache_Output_ప్రవాహం &OS) {
  render_బిడ్డ(CurrentCtx, OS);
}

శూన్య ASTNode::render_పాఠ్యం(Mustache_Output_ప్రవాహం &OS) { OS << Body; }

శూన్య ASTNode::renderPartial(స్థిరరాశి json::విలువ &CurrentCtx,
                            Mustache_Output_ప్రవాహం &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_విలువ[0]
                    << ", Indentation:" << Indentation << "\n");
  స్వయంచాలక Partial = Ctx.Partials.కనుగొను(Accessor_విలువ[0]);
  ఒకవేళ (Partial != Ctx.Partials.చివర())
    renderPartial(CurrentCtx, OS, Partial->get_విలువ());
}

శూన్య ASTNode::renderVariable(స్థిరరాశి json::విలువ &CurrentCtx,
                             Mustache_Output_ప్రవాహం &OS) {
  స్వయంచాలక Lambda = Ctx.Lambdas.కనుగొను(Accessor_విలువ[0]);
  ఒకవేళ (Lambda != Ctx.Lambdas.చివర()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_విలువ());
  } లేకపోతే ఒకవేళ (స్థిరరాశి json::విలువ *ContextPtr = కనుగొను_Context()) {
    Escape_String_ప్రవాహం ES(OS, Ctx.Escapes);
    గమ్యం_Mustache_String(*ContextPtr, ES);
  }
}

శూన్య ASTNode::renderUnescapeVariable(స్థిరరాశి json::విలువ &CurrentCtx,
                                     Mustache_Output_ప్రవాహం &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_విలువ[0]
                    << "\n");
  స్వయంచాలక Lambda = Ctx.Lambdas.కనుగొను(Accessor_విలువ[0]);
  ఒకవేళ (Lambda != Ctx.Lambdas.చివర()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_విలువ());
  } లేకపోతే ఒకవేళ (స్థిరరాశి json::విలువ *ContextPtr = కనుగొను_Context()) {
    OS.suspendIndentation();
    గమ్యం_Mustache_String(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

శూన్య ASTNode::renderSection(స్థిరరాశి json::విలువ &CurrentCtx,
                            Mustache_Output_ప్రవాహం &OS) {
  స్వయంచాలక SectionLambda = Ctx.SectionLambdas.కనుగొను(Accessor_విలువ[0]);
  ఒకవేళ (SectionLambda != Ctx.SectionLambdas.చివర()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_విలువ());
    తిరిగి;
  }

  స్థిరరాశి json::విలువ *ContextPtr = కనుగొను_Context();
  ఒకవేళ (isContextFalsey(ContextPtr))
    తిరిగి;

  ఒకవేళ (స్థిరరాశి json::Array *Arr = ContextPtr->getAsArray()) {
    కొరకు (స్థిరరాశి json::విలువ &V : *Arr)
      render_బిడ్డ(V, OS);
    తిరిగి;
  }
  render_బిడ్డ(*ContextPtr, OS);
}

శూన్య ASTNode::renderInvertSection(స్థిరరాశి json::విలువ &CurrentCtx,
                                  Mustache_Output_ప్రవాహం &OS) {
  తార్కిక IsLambda = Ctx.SectionLambdas.కలిగియుంది(Accessor_విలువ[0]);
  స్థిరరాశి json::విలువ *ContextPtr = కనుగొను_Context();
  ఒకవేళ (isContextFalsey(ContextPtr) && !IsLambda) {
    render_బిడ్డ(CurrentCtx, OS);
  }
}

శూన్య ASTNode::render(స్థిరరాశి llvm::json::విలువ &దత్తాంశం_2, Mustache_Output_ప్రవాహం &OS) {
  ఒకవేళ (Ty != మూలం_2 && Ty != పాఠ్యం_2 && Accessor_విలువ.ఖాళీలు())
    తిరిగి;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  మాతృ_Context = &దత్తాంశం_2;

  ఎంపిక (Ty) {
  స్థితి మూలం_2:
    render_మూలం(దత్తాంశం_2, OS);
    తిరిగి;
  స్థితి పాఠ్యం_2:
    render_పాఠ్యం(OS);
    తిరిగి;
  స్థితి Partial:
    renderPartial(దత్తాంశం_2, OS);
    తిరిగి;
  స్థితి Variable:
    renderVariable(దత్తాంశం_2, OS);
    తిరిగి;
  స్థితి UnescapeVariable:
    renderUnescapeVariable(దత్తాంశం_2, OS);
    తిరిగి;
  స్థితి Section:
    renderSection(దత్తాంశం_2, OS);
    తిరిగి;
  స్థితి InvertSection:
    renderInvertSection(దత్తాంశం_2, OS);
    తిరిగి;
  }
  llvm_unreachable("Invalid ASTNode type");
}

స్థిరరాశి json::విలువ *ASTNode::కనుగొను_Context() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  ఒకవేళ (Accessor_విలువ.ఖాళీలు())
    తిరిగి శూన్యసూచిక;
  ఒకవేళ (Accessor_విలువ[0] == ".")
    తిరిగి మాతృ_Context;

  స్థిరరాశి json::Object *CurrentContext = మాతృ_Context->getAsObject();
  StringRef CurrentAccessor = Accessor_విలువ[0];
  ASTNode *Current_మాతృ = మాతృ_2;

  అయితే (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    ఒకవేళ (Current_మాతృ->Ty != మూలం_2) {
      CurrentContext = Current_మాతృ->మాతృ_Context->getAsObject();
      Current_మాతృ = Current_మాతృ->మాతృ_2;
      కొనసాగు;
    }
    తిరిగి శూన్యసూచిక;
  }
  స్థిరరాశి json::విలువ *Context = శూన్యసూచిక;
  కొరకు (స్వయంచాలక [Idx, Acc] : enumerate(Accessor_విలువ)) {
    స్థిరరాశి json::విలువ *Current_విలువ = CurrentContext->get(Acc);
    ఒకవేళ (!Current_విలువ)
      తిరిగి శూన్యసూచిక;
    ఒకవేళ (Idx < Accessor_విలువ.కొలత_2() - 1) {
      CurrentContext = Current_విలువ->getAsObject();
      ఒకవేళ (!CurrentContext)
        తిరిగి శూన్యసూచిక;
    } లేకపోతే {
      Context = Current_విలువ;
    }
  }
  తిరిగి Context;
}

శూన్య ASTNode::render_బిడ్డ(స్థిరరాశి json::విలువ &Contexts,
                          Mustache_Output_ప్రవాహం &OS) {
  కొరకు (ASTNode &బిడ్డ : Children)
    బిడ్డ.render(Contexts, OS);
}

శూన్య ASTNode::renderPartial(స్థిరరాశి json::విలువ &Contexts,
                            Mustache_Output_ప్రవాహం &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  కూడు_Indentation_String_ప్రవాహం IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

శూన్య ASTNode::renderLambdas(స్థిరరాశి llvm::json::విలువ &Contexts,
                            Mustache_Output_ప్రవాహం &OS, Lambda &L) {
  json::విలువ Lambda_ఫలితం = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  గమ్యం_Mustache_String(Lambda_ఫలితం, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_కణుపు = P.parse();

  Escape_String_ప్రవాహం ES(OS, Ctx.Escapes);
  ఒకవేళ (Ty == Variable) {
    Lambda_కణుపు->render(Contexts, ES);
    తిరిగి;
  }
  Lambda_కణుపు->render(Contexts, OS);
}

శూన్య ASTNode::renderSectionLambdas(స్థిరరాశి llvm::json::విలువ &Contexts,
                                   Mustache_Output_ప్రవాహం &OS, SectionLambda &L) {
  json::విలువ Return = L(RawBody.str());
  ఒకవేళ (isFalsey(Return))
    తిరిగి;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  గమ్యం_Mustache_String(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_కణుపు = P.parse();
  Lambda_కణుపు->render(Contexts, OS);
}

శూన్య Template::render(స్థిరరాశి llvm::json::విలువ &దత్తాంశం_2, llvm::raw_ostream &OS) {
  Raw_Mustache_Output_ప్రవాహం MOS(OS);
  వృక్షం->render(దత్తాంశం_2, MOS);
}

శూన్య Template::రిజిస్టరు_Partial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_వృక్షం = P.parse();
  Ctx.Partials.చొప్పించు(std::make_pair(Name, Partial_వృక్షం));
}

శూన్య Template::రిజిస్టరు_Lambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

శూన్య Template::రిజిస్టరు_Lambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

శూన్య Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  వృక్షం = P.parse();
  // The default behavior is to escape html entities.
  స్థిరరాశి EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), వృక్షం(Other.వృక్షం) {
  Other.వృక్షం = శూన్యసూచిక;
}

Template::~Template() = అప్రమేయ;

} // namespace llvm::mustache

#నిర్వచనం_రద్దు DEBUG_TYPE
