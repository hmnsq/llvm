/*===--- ConvertUTF.c - Universal Character Names conversions ---------------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===------------------------------------------------------------------------=*/
/*
 * Copyright © 1991-2015 Unicode, Inc. All rights reserved.
 * Distributed under the Terms of Use in
 * http://www.unicode.org/copyright.html.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of the Unicode data files and any associated documentation
 * (the "Data Files") or Unicode software and any associated documentation
 * (the "Software") to deal in the Data Files or Software
 * without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, and/or sell copies of
 * the Data Files or Software, and to permit persons to whom the Data Files
 * or Software are furnished to do so, provided that
 * (a) this copyright and permission notice appear with all copies
 * of the Data Files or Software,
 * (b) this copyright and permission notice appear in associated
 * documentation, and
 * (c) there is clear notice in each modified Data File or in the Software
 * as well as in the documentation associated with the Data File(s) or
 * Software that the data or software has been modified.
 *
 * THE DATA FILES AND SOFTWARE ARE PROVIDED "AS IS", WITHOUT WARRANTY OF
 * ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS
 * NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL
 * DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THE DATA FILES OR SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder
 * shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in these Data Files or Software without prior
 * written authorization of the copyright holder.
 */

/* ---------------------------------------------------------------------

    Conversions between UTF32, UTF-16, and UTF-8. Source code file.
    Author: Mark E. Davis, 1994.
    Rev History: Rick McGowan, fixes & updates May 2001.
    Sept 2001: fixed const & error conditions per
        mods suggested by S. Parent & A. Lillich.
    June 2002: Tim Dodd added detection and handling of incomplete
        source sequences, enhanced error detection, added casts
        to eliminate compiler warnings.
    July 2003: slight mods to back out aggressive FFFE detection.
    Jan 2004: updated switches in from-UTF8 conversions.
    Oct 2004: updated to use UNI_MAX_LEGAL_UTF32 in UTF-32 conversions.

    See the header file "ConvertUTF.h" for complete documentation.

------------------------------------------------------------------------ */

#अन्तर्भाव "llvm/Support/ConvertUTF.h"
#यदि_परिभाषितम् CVTUTF_DEBUG
#अन्तर्भाव <stdio.h>
#यदि_अन्तः
#अन्तर्भाव <assert.h>

/*
 * This code extensively uses fall-through switches.
 * Keep the compiler from warning about that.
 */
#यदि defined(__clang__) && defined(__has_warning)
# यदि __has_warning("-Wimplicit-fallthrough")
#  define ConvertUTF_DISABLE_WARNINGS \
    _Pragma("clang diagnostic push")  \
    _Pragma("clang diagnostic ignored \"-Wimplicit-fallthrough\"")
#  define ConvertUTF_RESTORE_WARNINGS \
    _Pragma("clang diagnostic pop")
# endif
#नोचेद्यदि defined(__GNUC__)
# define ConvertUTF_DISABLE_WARNINGS \
   _Pragma("GCC diagnostic push")    \
   _Pragma("GCC diagnostic ignored \"-Wimplicit-fallthrough\"")
# define ConvertUTF_RESTORE_WARNINGS \
   _Pragma("GCC diagnostic pop")
#यदि_अन्तः
#यदि_अपरिभाषितम् ConvertUTF_DISABLE_WARNINGS
# define ConvertUTF_DISABLE_WARNINGS
#यदि_अन्तः
#यदि_अपरिभाषितम् ConvertUTF_RESTORE_WARNINGS
# define ConvertUTF_RESTORE_WARNINGS
#यदि_अन्तः

ConvertUTF_DISABLE_WARNINGS

नामाकाश llvm {

स्थिर ध्रुव पूर्णाङ्क half_सरणम्  = 10; /* used for shifting by 10 bits */

स्थिर ध्रुव UTF32 half_आधारः = 0x0010000UL;
स्थिर ध्रुव UTF32 half_आवरणम् = 0x3FFUL;

#परिभाषय UNI_SUR_उच्चः_START  (UTF32)0xD800
#परिभाषय UNI_SUR_उच्चः_अन्तः    (UTF32)0xDBFF
#परिभाषय UNI_SUR_नीचः_START   (UTF32)0xDC00
#परिभाषय UNI_SUR_नीचः_अन्तः     (UTF32)0xDFFF

/* --------------------------------------------------------------------- */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
स्थिर ध्रुव वर्ण trailing_अष्टकानि_For_UTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/*
 * Magic values subtracted from a buffer value during UTF8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence.
 */
स्थिर ध्रुव UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
                     0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
स्थिर ध्रुव UTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */


/* --------------------------------------------------------------------- */

Conversion_परिणाम ConvertUTF32toUTF16 (
        ध्रुव UTF32** sourceStart, ध्रुव UTF32* source_अन्तः,
        UTF16** targetStart, UTF16* target_अन्तः, ConversionFlags flags) {
    Conversion_परिणाम परिणाम_2 = conversionOK;
    ध्रुव UTF32* source = *sourceStart;
    UTF16* target = *targetStart;
    यावत् (source < source_अन्तः) {
        UTF32 ch;
        यदि (target >= target_अन्तः) {
            परिणाम_2 = targetExhausted; भङ्ग;
        }
        ch = *source++;
        यदि (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
            यदि (ch >= UNI_SUR_उच्चः_START && ch <= UNI_SUR_नीचः_अन्तः) {
                यदि (flags == strictConversion) {
                    --source; /* return to the illegal value itself */
                    परिणाम_2 = sourceIllegal;
                    भङ्ग;
                } अन्यथा {
                    *target++ = UNI_REPLACEMENT_CHAR;
                }
            } अन्यथा {
                *target++ = (UTF16)ch; /* normal case */
            }
        } अन्यथा यदि (ch > UNI_MAX_LEGAL_UTF32) {
            यदि (flags == strictConversion) {
                परिणाम_2 = sourceIllegal;
            } अन्यथा {
                *target++ = UNI_REPLACEMENT_CHAR;
            }
        } अन्यथा {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            यदि (target + 1 >= target_अन्तः) {
                --source; /* Back up source pointer! */
                परिणाम_2 = targetExhausted; भङ्ग;
            }
            ch -= half_आधारः;
            *target++ = (UTF16)((ch >> half_सरणम्) + UNI_SUR_उच्चः_START);
            *target++ = (UTF16)((ch & half_आवरणम्) + UNI_SUR_नीचः_START);
        }
    }
    *sourceStart = source;
    *targetStart = target;
    प्रत्यागम परिणाम_2;
}

/* --------------------------------------------------------------------- */

Conversion_परिणाम ConvertUTF16toUTF32 (
        ध्रुव UTF16** sourceStart, ध्रुव UTF16* source_अन्तः,
        UTF32** targetStart, UTF32* target_अन्तः, ConversionFlags flags) {
    Conversion_परिणाम परिणाम_2 = conversionOK;
    ध्रुव UTF16* source = *sourceStart;
    UTF32* target = *targetStart;
    UTF32 ch, ch2;
    यावत् (source < source_अन्तः) {
        ध्रुव UTF16* oldSource = source; /*  In case we have to back up because of target overflow. */
        ch = *source++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        यदि (ch >= UNI_SUR_उच्चः_START && ch <= UNI_SUR_उच्चः_अन्तः) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            यदि (source < source_अन्तः) {
                ch2 = *source;
                /* If it's a low surrogate, convert to UTF32. */
                यदि (ch2 >= UNI_SUR_नीचः_START && ch2 <= UNI_SUR_नीचः_अन्तः) {
                    ch = ((ch - UNI_SUR_उच्चः_START) << half_सरणम्)
                        + (ch2 - UNI_SUR_नीचः_START) + half_आधारः;
                    ++source;
                } अन्यथा यदि (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --source; /* return to the illegal value itself */
                    परिणाम_2 = sourceIllegal;
                    भङ्ग;
                }
            } अन्यथा { /* We don't have the 16 bits following the high surrogate. */
                --source; /* return to the high surrogate */
                परिणाम_2 = sourceExhausted;
                भङ्ग;
            }
        } अन्यथा यदि (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            यदि (ch >= UNI_SUR_नीचः_START && ch <= UNI_SUR_नीचः_अन्तः) {
                --source; /* return to the illegal value itself */
                परिणाम_2 = sourceIllegal;
                भङ्ग;
            }
        }
        यदि (target >= target_अन्तः) {
            source = oldSource; /* Back up source pointer! */
            परिणाम_2 = targetExhausted; भङ्ग;
        }
        *target++ = ch;
    }
    *sourceStart = source;
    *targetStart = target;
#यदि_परिभाषितम् CVTUTF_DEBUG
यदि (परिणाम_2 == sourceIllegal) {
    fprintf(stderr, "ConvertUTF16toUTF32 illegal seq 0x%04x,%04x\n", ch, ch2);
    fflush(stderr);
}
#यदि_अन्तः
    प्रत्यागम परिणाम_2;
}
Conversion_परिणाम ConvertUTF16toUTF8 (
        ध्रुव UTF16** sourceStart, ध्रुव UTF16* source_अन्तः,
        UTF8** targetStart, UTF8* target_अन्तः, ConversionFlags flags) {
    Conversion_परिणाम परिणाम_2 = conversionOK;
    ध्रुव UTF16* source = *sourceStart;
    UTF8* target = *targetStart;
    यावत् (source < source_अन्तः) {
        UTF32 ch;
        अचिह्नित ह्रस्व अष्टकानि_गन्तव्यम्_Write = 0;
        ध्रुव UTF32 byte_आवरणम् = 0xBF;
        ध्रुव UTF32 byteMark = 0x80;
        ध्रुव UTF16* oldSource = source; /* In case we have to back up because of target overflow. */
        ch = *source++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        यदि (ch >= UNI_SUR_उच्चः_START && ch <= UNI_SUR_उच्चः_अन्तः) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            यदि (source < source_अन्तः) {
                UTF32 ch2 = *source;
                /* If it's a low surrogate, convert to UTF32. */
                यदि (ch2 >= UNI_SUR_नीचः_START && ch2 <= UNI_SUR_नीचः_अन्तः) {
                    ch = ((ch - UNI_SUR_उच्चः_START) << half_सरणम्)
                        + (ch2 - UNI_SUR_नीचः_START) + half_आधारः;
                    ++source;
                } अन्यथा यदि (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --source; /* return to the illegal value itself */
                    परिणाम_2 = sourceIllegal;
                    भङ्ग;
                }
            } अन्यथा { /* We don't have the 16 bits following the high surrogate. */
                --source; /* return to the high surrogate */
                परिणाम_2 = sourceExhausted;
                भङ्ग;
            }
        } अन्यथा यदि (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            यदि (ch >= UNI_SUR_नीचः_START && ch <= UNI_SUR_नीचः_अन्तः) {
                --source; /* return to the illegal value itself */
                परिणाम_2 = sourceIllegal;
                भङ्ग;
            }
        }
        /* Figure out how many bytes the result will require */
        यदि (ch < (UTF32)0x80) {      अष्टकानि_गन्तव्यम्_Write = 1;
        } अन्यथा यदि (ch < (UTF32)0x800) {     अष्टकानि_गन्तव्यम्_Write = 2;
        } अन्यथा यदि (ch < (UTF32)0x10000) {   अष्टकानि_गन्तव्यम्_Write = 3;
        } अन्यथा यदि (ch < (UTF32)0x110000) {  अष्टकानि_गन्तव्यम्_Write = 4;
        } अन्यथा {                            अष्टकानि_गन्तव्यम्_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
        }

        target += अष्टकानि_गन्तव्यम्_Write;
        यदि (target > target_अन्तः) {
            source = oldSource; /* Back up source pointer! */
            target -= अष्टकानि_गन्तव्यम्_Write; परिणाम_2 = targetExhausted; भङ्ग;
        }
        विकल्प (अष्टकानि_गन्तव्यम्_Write) { /* note: everything falls through. */
            पक्ष 4: *--target = (UTF8)((ch | byteMark) & byte_आवरणम्); ch >>= 6;
            पक्ष 3: *--target = (UTF8)((ch | byteMark) & byte_आवरणम्); ch >>= 6;
            पक्ष 2: *--target = (UTF8)((ch | byteMark) & byte_आवरणम्); ch >>= 6;
            पक्ष 1: *--target =  (UTF8)(ch | firstByteMark[अष्टकानि_गन्तव्यम्_Write]);
        }
        target += अष्टकानि_गन्तव्यम्_Write;
    }
    *sourceStart = source;
    *targetStart = target;
    प्रत्यागम परिणाम_2;
}

/* --------------------------------------------------------------------- */

Conversion_परिणाम ConvertUTF32toUTF8 (
        ध्रुव UTF32** sourceStart, ध्रुव UTF32* source_अन्तः,
        UTF8** targetStart, UTF8* target_अन्तः, ConversionFlags flags) {
    Conversion_परिणाम परिणाम_2 = conversionOK;
    ध्रुव UTF32* source = *sourceStart;
    UTF8* target = *targetStart;
    यावत् (source < source_अन्तः) {
        UTF32 ch;
        अचिह्नित ह्रस्व अष्टकानि_गन्तव्यम्_Write = 0;
        ध्रुव UTF32 byte_आवरणम् = 0xBF;
        ध्रुव UTF32 byteMark = 0x80;
        ch = *source++;
        यदि (flags == strictConversion ) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            यदि (ch >= UNI_SUR_उच्चः_START && ch <= UNI_SUR_नीचः_अन्तः) {
                --source; /* return to the illegal value itself */
                परिणाम_2 = sourceIllegal;
                भङ्ग;
            }
        }
        /*
         * Figure out how many bytes the result will require. Turn any
         * illegally large UTF32 things (> Plane 17) into replacement chars.
         */
        यदि (ch < (UTF32)0x80) {      अष्टकानि_गन्तव्यम्_Write = 1;
        } अन्यथा यदि (ch < (UTF32)0x800) {     अष्टकानि_गन्तव्यम्_Write = 2;
        } अन्यथा यदि (ch < (UTF32)0x10000) {   अष्टकानि_गन्तव्यम्_Write = 3;
        } अन्यथा यदि (ch <= UNI_MAX_LEGAL_UTF32) {  अष्टकानि_गन्तव्यम्_Write = 4;
        } अन्यथा {                            अष्टकानि_गन्तव्यम्_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
                                            परिणाम_2 = sourceIllegal;
        }

        target += अष्टकानि_गन्तव्यम्_Write;
        यदि (target > target_अन्तः) {
            --source; /* Back up source pointer! */
            target -= अष्टकानि_गन्तव्यम्_Write; परिणाम_2 = targetExhausted; भङ्ग;
        }
        विकल्प (अष्टकानि_गन्तव्यम्_Write) { /* note: everything falls through. */
            पक्ष 4: *--target = (UTF8)((ch | byteMark) & byte_आवरणम्); ch >>= 6;
            पक्ष 3: *--target = (UTF8)((ch | byteMark) & byte_आवरणम्); ch >>= 6;
            पक्ष 2: *--target = (UTF8)((ch | byteMark) & byte_आवरणम्); ch >>= 6;
            पक्ष 1: *--target = (UTF8) (ch | firstByteMark[अष्टकानि_गन्तव्यम्_Write]);
        }
        target += अष्टकानि_गन्तव्यम्_Write;
    }
    *sourceStart = source;
    *targetStart = target;
    प्रत्यागम परिणाम_2;
}

/* --------------------------------------------------------------------- */

/*
 * Utility routine to tell whether a sequence of bytes is legal UTF-8.
 * This must be called with the length pre-determined by the first byte.
 * If not calling this from ConvertUTF8to*, then the length can be set by:
 *  length = trailingBytesForUTF8[*source]+1;
 * and the sequence is illegal right away if there aren't that many bytes
 * available.
 * If presented with a length > 4, this returns false.  The Unicode
 * definition of UTF-8 goes up to 4-byte sequences.
 */

स्थिर Boolean isLegalUTF8(ध्रुव UTF8 *source, पूर्णाङ्क दीर्घता_2) {
    UTF8 a;
    ध्रुव UTF8 *srcptr = source+दीर्घता_2;
    विकल्प (दीर्घता_2) {
    स्वभाव: प्रत्यागम असत्य;
        /* Everything else falls through when "true"... */
    पक्ष 4: यदि ((a = (*--srcptr)) < 0x80 || a > 0xBF) प्रत्यागम असत्य;
    पक्ष 3: यदि ((a = (*--srcptr)) < 0x80 || a > 0xBF) प्रत्यागम असत्य;
    पक्ष 2: यदि ((a = (*--srcptr)) < 0x80 || a > 0xBF) प्रत्यागम असत्य;

        विकल्प (*source) {
            /* no fall-through in this inner switch */
            पक्ष 0xE0: यदि (a < 0xA0) प्रत्यागम असत्य; भङ्ग;
            पक्ष 0xED: यदि (a > 0x9F) प्रत्यागम असत्य; भङ्ग;
            पक्ष 0xF0: यदि (a < 0x90) प्रत्यागम असत्य; भङ्ग;
            पक्ष 0xF4: यदि (a > 0x8F) प्रत्यागम असत्य; भङ्ग;
            स्वभाव:   यदि (a < 0x80) प्रत्यागम असत्य;
        }

    पक्ष 1: यदि (*source >= 0x80 && *source < 0xC2) प्रत्यागम असत्य;
    }
    यदि (*source > 0xF4) प्रत्यागम असत्य;
    प्रत्यागम सत्य;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 sequence is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8Sequence(ध्रुव UTF8 *source, ध्रुव UTF8 *source_अन्तः) {
    पूर्णाङ्क दीर्घता_2 = trailing_अष्टकानि_For_UTF8[*source]+1;
    यदि (दीर्घता_2 > source_अन्तः - source) {
        प्रत्यागम असत्य;
    }
    प्रत्यागम isLegalUTF8(source, दीर्घता_2);
}

/*
 * Exported function to return the size of the first utf-8 code unit sequence,
 * Or 0 if the sequence is not valid;
 */
अचिह्नित get_UTF8_Sequence_आकार(ध्रुव UTF8 *source, ध्रुव UTF8 *source_अन्तः) {
  पूर्णाङ्क दीर्घता_2 = trailing_अष्टकानि_For_UTF8[*source] + 1;
  प्रत्यागम (दीर्घता_2 <= source_अन्तः - source && isLegalUTF8(source, दीर्घता_2)) ? दीर्घता_2
                                                                       : 0;
}

/* --------------------------------------------------------------------- */

अचिह्नित find_Maximal_Subpart_सम्बन्धि_Ill_Formed_UTF8_Sequence(ध्रुव UTF8 *source,
                                                   ध्रुव UTF8 *source_अन्तः) {
  UTF8 b1, b2, b3;

  प्रतिज्ञा(!isLegalUTF8Sequence(source, source_अन्तः));

  /*
   * Unicode 6.3.0, D93b:
   *
   *   Maximal subpart of an ill-formed subsequence: The longest code unit
   *   subsequence starting at an unconvertible offset that is either:
   *   a. the initial subsequence of a well-formed code unit sequence, or
   *   b. a subsequence of length one.
   */

  यदि (source == source_अन्तः)
    प्रत्यागम 0;

  /*
   * Perform case analysis.  See Unicode 6.3.0, Table 3-7. Well-Formed UTF-8
   * Byte Sequences.
   */

  b1 = *source;
  ++source;
  यदि (b1 >= 0xC2 && b1 <= 0xDF) {
    /*
     * First byte is valid, but we know that this code unit sequence is
     * invalid, so the maximal subpart has to end after the first byte.
     */
    प्रत्यागम 1;
  }

  यदि (source == source_अन्तः)
    प्रत्यागम 1;

  b2 = *source;
  ++source;

  यदि (b1 == 0xE0) {
    प्रत्यागम (b2 >= 0xA0 && b2 <= 0xBF) ? 2 : 1;
  }
  यदि (b1 >= 0xE1 && b1 <= 0xEC) {
    प्रत्यागम (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  यदि (b1 == 0xED) {
    प्रत्यागम (b2 >= 0x80 && b2 <= 0x9F) ? 2 : 1;
  }
  यदि (b1 >= 0xEE && b1 <= 0xEF) {
    प्रत्यागम (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  यदि (b1 == 0xF0) {
    यदि (b2 >= 0x90 && b2 <= 0xBF) {
      यदि (source == source_अन्तः)
        प्रत्यागम 2;

      b3 = *source;
      प्रत्यागम (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    प्रत्यागम 1;
  }
  यदि (b1 >= 0xF1 && b1 <= 0xF3) {
    यदि (b2 >= 0x80 && b2 <= 0xBF) {
      यदि (source == source_अन्तः)
        प्रत्यागम 2;

      b3 = *source;
      प्रत्यागम (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    प्रत्यागम 1;
  }
  यदि (b1 == 0xF4) {
    यदि (b2 >= 0x80 && b2 <= 0x8F) {
      यदि (source == source_अन्तः)
        प्रत्यागम 2;

      b3 = *source;
      प्रत्यागम (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    प्रत्यागम 1;
  }

  प्रतिज्ञा((b1 >= 0x80 && b1 <= 0xC1) || b1 >= 0xF5);
  /*
   * There are no valid sequences that start with these bytes.  Maximal subpart
   * is defined to have length 1 in these cases.
   */
  प्रत्यागम 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return the total number of bytes in a codepoint
 * represented in UTF-8, given the value of the first byte.
 */
अचिह्नित get_अङ्कः_अष्टकानि_For_UTF8(UTF8 first) {
  प्रत्यागम trailing_अष्टकानि_For_UTF8[first] + 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 string is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8String(ध्रुव UTF8 **source, ध्रुव UTF8 *source_अन्तः) {
    यावत् (*source != source_अन्तः) {
        पूर्णाङ्क दीर्घता_2 = trailing_अष्टकानि_For_UTF8[**source] + 1;
        यदि (दीर्घता_2 > source_अन्तः - *source || !isLegalUTF8(*source, दीर्घता_2))
            प्रत्यागम असत्य;
        *source += दीर्घता_2;
    }
    प्रत्यागम सत्य;
}

/* --------------------------------------------------------------------- */

Conversion_परिणाम ConvertUTF8toUTF16 (
        ध्रुव UTF8** sourceStart, ध्रुव UTF8* source_अन्तः,
        UTF16** targetStart, UTF16* target_अन्तः, ConversionFlags flags) {
    Conversion_परिणाम परिणाम_2 = conversionOK;
    ध्रुव UTF8* source = *sourceStart;
    UTF16* target = *targetStart;
    यावत् (source < source_अन्तः) {
        UTF32 ch = 0;
        अचिह्नित ह्रस्व extra_अष्टकानि_गन्तव्यम्_Read = trailing_अष्टकानि_For_UTF8[*source];
        यदि (extra_अष्टकानि_गन्तव्यम्_Read >= source_अन्तः - source) {
            परिणाम_2 = sourceExhausted; भङ्ग;
        }
        /* Do this check whether lenient or strict */
        यदि (!isLegalUTF8(source, extra_अष्टकानि_गन्तव्यम्_Read+1)) {
            परिणाम_2 = sourceIllegal;
            भङ्ग;
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        विकल्प (extra_अष्टकानि_गन्तव्यम्_Read) {
            पक्ष 5: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
            पक्ष 4: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
            पक्ष 3: ch += *source++; ch <<= 6;
            पक्ष 2: ch += *source++; ch <<= 6;
            पक्ष 1: ch += *source++; ch <<= 6;
            पक्ष 0: ch += *source++;
        }
        ch -= offsetsFromUTF8[extra_अष्टकानि_गन्तव्यम्_Read];

        यदि (target >= target_अन्तः) {
            source -= (extra_अष्टकानि_गन्तव्यम्_Read+1); /* Back up source pointer! */
            परिणाम_2 = targetExhausted; भङ्ग;
        }
        यदि (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32 */
            यदि (ch >= UNI_SUR_उच्चः_START && ch <= UNI_SUR_नीचः_अन्तः) {
                यदि (flags == strictConversion) {
                    source -= (extra_अष्टकानि_गन्तव्यम्_Read+1); /* return to the illegal value itself */
                    परिणाम_2 = sourceIllegal;
                    भङ्ग;
                } अन्यथा {
                    *target++ = UNI_REPLACEMENT_CHAR;
                }
            } अन्यथा {
                *target++ = (UTF16)ch; /* normal case */
            }
        } अन्यथा यदि (ch > UNI_MAX_UTF16) {
            यदि (flags == strictConversion) {
                परिणाम_2 = sourceIllegal;
                source -= (extra_अष्टकानि_गन्तव्यम्_Read+1); /* return to the start */
                भङ्ग; /* Bail out; shouldn't continue */
            } अन्यथा {
                *target++ = UNI_REPLACEMENT_CHAR;
            }
        } अन्यथा {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            यदि (target + 1 >= target_अन्तः) {
                source -= (extra_अष्टकानि_गन्तव्यम्_Read+1); /* Back up source pointer! */
                परिणाम_2 = targetExhausted; भङ्ग;
            }
            ch -= half_आधारः;
            *target++ = (UTF16)((ch >> half_सरणम्) + UNI_SUR_उच्चः_START);
            *target++ = (UTF16)((ch & half_आवरणम्) + UNI_SUR_नीचः_START);
        }
    }
    *sourceStart = source;
    *targetStart = target;
    प्रत्यागम परिणाम_2;
}

/* --------------------------------------------------------------------- */

स्थिर Conversion_परिणाम ConvertUTF8toUTF32Impl(
        ध्रुव UTF8** sourceStart, ध्रुव UTF8* source_अन्तः,
        UTF32** targetStart, UTF32* target_अन्तः, ConversionFlags flags,
        Boolean निवेशः_Is_Partial) {
    Conversion_परिणाम परिणाम_2 = conversionOK;
    ध्रुव UTF8* source = *sourceStart;
    UTF32* target = *targetStart;
    यावत् (source < source_अन्तः) {
        UTF32 ch = 0;
        अचिह्नित ह्रस्व extra_अष्टकानि_गन्तव्यम्_Read = trailing_अष्टकानि_For_UTF8[*source];
        यदि (extra_अष्टकानि_गन्तव्यम्_Read >= source_अन्तः - source) {
            यदि (flags == strictConversion || निवेशः_Is_Partial) {
                परिणाम_2 = sourceExhausted;
                भङ्ग;
            } अन्यथा {
                परिणाम_2 = sourceIllegal;

                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                source += find_Maximal_Subpart_सम्बन्धि_Ill_Formed_UTF8_Sequence(source,
                                                                    source_अन्तः);
                *target++ = UNI_REPLACEMENT_CHAR;
                अनुवर्त;
            }
        }
        यदि (target >= target_अन्तः) {
            परिणाम_2 = targetExhausted; भङ्ग;
        }

        /* Do this check whether lenient or strict */
        यदि (!isLegalUTF8(source, extra_अष्टकानि_गन्तव्यम्_Read+1)) {
            परिणाम_2 = sourceIllegal;
            यदि (flags == strictConversion) {
                /* Abort conversion. */
                भङ्ग;
            } अन्यथा {
                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                source += find_Maximal_Subpart_सम्बन्धि_Ill_Formed_UTF8_Sequence(source,
                                                                    source_अन्तः);
                *target++ = UNI_REPLACEMENT_CHAR;
                अनुवर्त;
            }
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        विकल्प (extra_अष्टकानि_गन्तव्यम्_Read) {
            पक्ष 5: ch += *source++; ch <<= 6;
            पक्ष 4: ch += *source++; ch <<= 6;
            पक्ष 3: ch += *source++; ch <<= 6;
            पक्ष 2: ch += *source++; ch <<= 6;
            पक्ष 1: ch += *source++; ch <<= 6;
            पक्ष 0: ch += *source++;
        }
        ch -= offsetsFromUTF8[extra_अष्टकानि_गन्तव्यम्_Read];

        यदि (ch <= UNI_MAX_LEGAL_UTF32) {
            /*
             * UTF-16 surrogate values are illegal in UTF-32, and anything
             * over Plane 17 (> 0x10FFFF) is illegal.
             */
            यदि (ch >= UNI_SUR_उच्चः_START && ch <= UNI_SUR_नीचः_अन्तः) {
                यदि (flags == strictConversion) {
                    source -= (extra_अष्टकानि_गन्तव्यम्_Read+1); /* return to the illegal value itself */
                    परिणाम_2 = sourceIllegal;
                    भङ्ग;
                } अन्यथा {
                    *target++ = UNI_REPLACEMENT_CHAR;
                }
            } अन्यथा {
                *target++ = ch;
            }
        } अन्यथा { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
            परिणाम_2 = sourceIllegal;
            *target++ = UNI_REPLACEMENT_CHAR;
        }
    }
    *sourceStart = source;
    *targetStart = target;
    प्रत्यागम परिणाम_2;
}

Conversion_परिणाम ConvertUTF8toUTF32Partial(ध्रुव UTF8 **sourceStart,
                                           ध्रुव UTF8 *source_अन्तः,
                                           UTF32 **targetStart,
                                           UTF32 *target_अन्तः,
                                           ConversionFlags flags) {
  प्रत्यागम ConvertUTF8toUTF32Impl(sourceStart, source_अन्तः, targetStart, target_अन्तः,
                                flags, /*InputIsPartial=*/सत्य);
}

Conversion_परिणाम ConvertUTF8toUTF32(ध्रुव UTF8 **sourceStart,
                                    ध्रुव UTF8 *source_अन्तः, UTF32 **targetStart,
                                    UTF32 *target_अन्तः, ConversionFlags flags) {
  प्रत्यागम ConvertUTF8toUTF32Impl(sourceStart, source_अन्तः, targetStart, target_अन्तः,
                                flags, /*InputIsPartial=*/असत्य);
}

/* ---------------------------------------------------------------------

    Note A.
    The fall-through switches in UTF-8 reading code save a
    temp variable, some decrements & conditionals.  The switches
    are equivalent to the following loop:
        {
            int tmpBytesToRead = extraBytesToRead+1;
            do {
                ch += *source++;
                --tmpBytesToRead;
                if (tmpBytesToRead) ch <<= 6;
            } while (tmpBytesToRead > 0);
        }
    In UTF-8 writing code, the switches on "bytesToWrite" are
    similarly unrolled loops.

   --------------------------------------------------------------------- */

} // namespace llvm

ConvertUTF_RESTORE_WARNINGS
