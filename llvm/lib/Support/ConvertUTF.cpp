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

#ضمن "llvm/Support/ConvertUTF.h"
#جے_تعریف_شدہ CVTUTF_DEBUG
#ضمن <stdio.h>
#جے_انت
#ضمن <assert.h>

/*
 * This code extensively uses fall-through switches.
 * Keep the compiler from warning about that.
 */
#إذا defined(__clang__) && defined(__has_warning)
# إذا __has_warning("-Wimplicit-fallthrough")
#  define ConvertUTF_DISABLE_WARNINGS \
    _Pragma("clang diagnostic push")  \
    _Pragma("clang diagnostic ignored \"-Wimplicit-fallthrough\"")
#  define ConvertUTF_RESTORE_WARNINGS \
    _Pragma("clang diagnostic pop")
# endif
#وإلاإذا defined(__GNUC__)
# define ConvertUTF_DISABLE_WARNINGS \
   _Pragma("GCC diagnostic push")    \
   _Pragma("GCC diagnostic ignored \"-Wimplicit-fallthrough\"")
# define ConvertUTF_RESTORE_WARNINGS \
   _Pragma("GCC diagnostic pop")
#جے_انت
#جے_بنا_تعریف ConvertUTF_DISABLE_WARNINGS
# define ConvertUTF_DISABLE_WARNINGS
#جے_انت
#جے_بنا_تعریف ConvertUTF_RESTORE_WARNINGS
# define ConvertUTF_RESTORE_WARNINGS
#جے_انت

ConvertUTF_DISABLE_WARNINGS

نطاق llvm {

ساكن ثابت صحيح half_کھسکاؤ  = 10; /* used for shifting by 10 bits */

ساكن ثابت UTF32 half_ادھار = 0x0010000UL;
ساكن ثابت UTF32 half_نقاب = 0x3FFUL;

#تعریف_کرو UNI_SUR_بلند_شروع  (UTF32)0xD800
#تعریف_کرو UNI_SUR_بلند_سرا    (UTF32)0xDBFF
#تعریف_کرو UNI_SUR_کم_شروع   (UTF32)0xDC00
#تعریف_کرو UNI_SUR_کم_سرا     (UTF32)0xDFFF

/* --------------------------------------------------------------------- */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
ساكن ثابت حرف trailing_بائٹاں_For_UTF8[256] = {
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
ساكن ثابت UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
                     0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
ساكن ثابت UTF8 پہلا_Byte_Mark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */


/* --------------------------------------------------------------------- */

Conversion_نتیجہ ConvertUTF32toUTF16 (
        ثابت UTF32** سروت_شروع, ثابت UTF32* سروت_سرا,
        UTF16** نشانہ_شروع, UTF16* نشانہ_سرا, ConversionFlags flags) {
    Conversion_نتیجہ نتیجہ_2 = conversionOK;
    ثابت UTF32* سروت_2 = *سروت_شروع;
    UTF16* نشانہ = *نشانہ_شروع;
    طالما (سروت_2 < سروت_سرا) {
        UTF32 ch;
        إذا (نشانہ >= نشانہ_سرا) {
            نتیجہ_2 = نشانہ_Exhausted; اكسر;
        }
        ch = *سروت_2++;
        إذا (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
            إذا (ch >= UNI_SUR_بلند_شروع && ch <= UNI_SUR_کم_سرا) {
                إذا (flags == strictConversion) {
                    --سروت_2; /* return to the illegal value itself */
                    نتیجہ_2 = سروت_Illegal;
                    اكسر;
                } وإلا {
                    *نشانہ++ = UNI_REPLACEMENT_CHAR;
                }
            } وإلا {
                *نشانہ++ = (UTF16)ch; /* normal case */
            }
        } وإلا إذا (ch > UNI_MAX_LEGAL_UTF32) {
            إذا (flags == strictConversion) {
                نتیجہ_2 = سروت_Illegal;
            } وإلا {
                *نشانہ++ = UNI_REPLACEMENT_CHAR;
            }
        } وإلا {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            إذا (نشانہ + 1 >= نشانہ_سرا) {
                --سروت_2; /* Back up source pointer! */
                نتیجہ_2 = نشانہ_Exhausted; اكسر;
            }
            ch -= half_ادھار;
            *نشانہ++ = (UTF16)((ch >> half_کھسکاؤ) + UNI_SUR_بلند_شروع);
            *نشانہ++ = (UTF16)((ch & half_نقاب) + UNI_SUR_کم_شروع);
        }
    }
    *سروت_شروع = سروت_2;
    *نشانہ_شروع = نشانہ;
    أرجع نتیجہ_2;
}

/* --------------------------------------------------------------------- */

Conversion_نتیجہ ConvertUTF16toUTF32 (
        ثابت UTF16** سروت_شروع, ثابت UTF16* سروت_سرا,
        UTF32** نشانہ_شروع, UTF32* نشانہ_سرا, ConversionFlags flags) {
    Conversion_نتیجہ نتیجہ_2 = conversionOK;
    ثابت UTF16* سروت_2 = *سروت_شروع;
    UTF32* نشانہ = *نشانہ_شروع;
    UTF32 ch, ch2;
    طالما (سروت_2 < سروت_سرا) {
        ثابت UTF16* old_سروت = سروت_2; /*  In case we have to back up because of target overflow. */
        ch = *سروت_2++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        إذا (ch >= UNI_SUR_بلند_شروع && ch <= UNI_SUR_بلند_سرا) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            إذا (سروت_2 < سروت_سرا) {
                ch2 = *سروت_2;
                /* If it's a low surrogate, convert to UTF32. */
                إذا (ch2 >= UNI_SUR_کم_شروع && ch2 <= UNI_SUR_کم_سرا) {
                    ch = ((ch - UNI_SUR_بلند_شروع) << half_کھسکاؤ)
                        + (ch2 - UNI_SUR_کم_شروع) + half_ادھار;
                    ++سروت_2;
                } وإلا إذا (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --سروت_2; /* return to the illegal value itself */
                    نتیجہ_2 = سروت_Illegal;
                    اكسر;
                }
            } وإلا { /* We don't have the 16 bits following the high surrogate. */
                --سروت_2; /* return to the high surrogate */
                نتیجہ_2 = سروت_Exhausted;
                اكسر;
            }
        } وإلا إذا (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            إذا (ch >= UNI_SUR_کم_شروع && ch <= UNI_SUR_کم_سرا) {
                --سروت_2; /* return to the illegal value itself */
                نتیجہ_2 = سروت_Illegal;
                اكسر;
            }
        }
        إذا (نشانہ >= نشانہ_سرا) {
            سروت_2 = old_سروت; /* Back up source pointer! */
            نتیجہ_2 = نشانہ_Exhausted; اكسر;
        }
        *نشانہ++ = ch;
    }
    *سروت_شروع = سروت_2;
    *نشانہ_شروع = نشانہ;
#جے_تعریف_شدہ CVTUTF_DEBUG
إذا (نتیجہ_2 == سروت_Illegal) {
    fprintf(stderr, "ConvertUTF16toUTF32 illegal seq 0x%04x,%04x\n", ch, ch2);
    fflush(stderr);
}
#جے_انت
    أرجع نتیجہ_2;
}
Conversion_نتیجہ ConvertUTF16toUTF8 (
        ثابت UTF16** سروت_شروع, ثابت UTF16* سروت_سرا,
        UTF8** نشانہ_شروع, UTF8* نشانہ_سرا, ConversionFlags flags) {
    Conversion_نتیجہ نتیجہ_2 = conversionOK;
    ثابت UTF16* سروت_2 = *سروت_شروع;
    UTF8* نشانہ = *نشانہ_شروع;
    طالما (سروت_2 < سروت_سرا) {
        UTF32 ch;
        بنانشانی چھوٹا بائٹاں_منزل_Write = 0;
        ثابت UTF32 byte_نقاب = 0xBF;
        ثابت UTF32 byteMark = 0x80;
        ثابت UTF16* old_سروت = سروت_2; /* In case we have to back up because of target overflow. */
        ch = *سروت_2++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        إذا (ch >= UNI_SUR_بلند_شروع && ch <= UNI_SUR_بلند_سرا) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            إذا (سروت_2 < سروت_سرا) {
                UTF32 ch2 = *سروت_2;
                /* If it's a low surrogate, convert to UTF32. */
                إذا (ch2 >= UNI_SUR_کم_شروع && ch2 <= UNI_SUR_کم_سرا) {
                    ch = ((ch - UNI_SUR_بلند_شروع) << half_کھسکاؤ)
                        + (ch2 - UNI_SUR_کم_شروع) + half_ادھار;
                    ++سروت_2;
                } وإلا إذا (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --سروت_2; /* return to the illegal value itself */
                    نتیجہ_2 = سروت_Illegal;
                    اكسر;
                }
            } وإلا { /* We don't have the 16 bits following the high surrogate. */
                --سروت_2; /* return to the high surrogate */
                نتیجہ_2 = سروت_Exhausted;
                اكسر;
            }
        } وإلا إذا (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            إذا (ch >= UNI_SUR_کم_شروع && ch <= UNI_SUR_کم_سرا) {
                --سروت_2; /* return to the illegal value itself */
                نتیجہ_2 = سروت_Illegal;
                اكسر;
            }
        }
        /* Figure out how many bytes the result will require */
        إذا (ch < (UTF32)0x80) {      بائٹاں_منزل_Write = 1;
        } وإلا إذا (ch < (UTF32)0x800) {     بائٹاں_منزل_Write = 2;
        } وإلا إذا (ch < (UTF32)0x10000) {   بائٹاں_منزل_Write = 3;
        } وإلا إذا (ch < (UTF32)0x110000) {  بائٹاں_منزل_Write = 4;
        } وإلا {                            بائٹاں_منزل_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
        }

        نشانہ += بائٹاں_منزل_Write;
        إذا (نشانہ > نشانہ_سرا) {
            سروت_2 = old_سروت; /* Back up source pointer! */
            نشانہ -= بائٹاں_منزل_Write; نتیجہ_2 = نشانہ_Exhausted; اكسر;
        }
        بدل (بائٹاں_منزل_Write) { /* note: everything falls through. */
            حالة 4: *--نشانہ = (UTF8)((ch | byteMark) & byte_نقاب); ch >>= 6;
            حالة 3: *--نشانہ = (UTF8)((ch | byteMark) & byte_نقاب); ch >>= 6;
            حالة 2: *--نشانہ = (UTF8)((ch | byteMark) & byte_نقاب); ch >>= 6;
            حالة 1: *--نشانہ =  (UTF8)(ch | پہلا_Byte_Mark[بائٹاں_منزل_Write]);
        }
        نشانہ += بائٹاں_منزل_Write;
    }
    *سروت_شروع = سروت_2;
    *نشانہ_شروع = نشانہ;
    أرجع نتیجہ_2;
}

/* --------------------------------------------------------------------- */

Conversion_نتیجہ ConvertUTF32toUTF8 (
        ثابت UTF32** سروت_شروع, ثابت UTF32* سروت_سرا,
        UTF8** نشانہ_شروع, UTF8* نشانہ_سرا, ConversionFlags flags) {
    Conversion_نتیجہ نتیجہ_2 = conversionOK;
    ثابت UTF32* سروت_2 = *سروت_شروع;
    UTF8* نشانہ = *نشانہ_شروع;
    طالما (سروت_2 < سروت_سرا) {
        UTF32 ch;
        بنانشانی چھوٹا بائٹاں_منزل_Write = 0;
        ثابت UTF32 byte_نقاب = 0xBF;
        ثابت UTF32 byteMark = 0x80;
        ch = *سروت_2++;
        إذا (flags == strictConversion ) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            إذا (ch >= UNI_SUR_بلند_شروع && ch <= UNI_SUR_کم_سرا) {
                --سروت_2; /* return to the illegal value itself */
                نتیجہ_2 = سروت_Illegal;
                اكسر;
            }
        }
        /*
         * Figure out how many bytes the result will require. Turn any
         * illegally large UTF32 things (> Plane 17) into replacement chars.
         */
        إذا (ch < (UTF32)0x80) {      بائٹاں_منزل_Write = 1;
        } وإلا إذا (ch < (UTF32)0x800) {     بائٹاں_منزل_Write = 2;
        } وإلا إذا (ch < (UTF32)0x10000) {   بائٹاں_منزل_Write = 3;
        } وإلا إذا (ch <= UNI_MAX_LEGAL_UTF32) {  بائٹاں_منزل_Write = 4;
        } وإلا {                            بائٹاں_منزل_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
                                            نتیجہ_2 = سروت_Illegal;
        }

        نشانہ += بائٹاں_منزل_Write;
        إذا (نشانہ > نشانہ_سرا) {
            --سروت_2; /* Back up source pointer! */
            نشانہ -= بائٹاں_منزل_Write; نتیجہ_2 = نشانہ_Exhausted; اكسر;
        }
        بدل (بائٹاں_منزل_Write) { /* note: everything falls through. */
            حالة 4: *--نشانہ = (UTF8)((ch | byteMark) & byte_نقاب); ch >>= 6;
            حالة 3: *--نشانہ = (UTF8)((ch | byteMark) & byte_نقاب); ch >>= 6;
            حالة 2: *--نشانہ = (UTF8)((ch | byteMark) & byte_نقاب); ch >>= 6;
            حالة 1: *--نشانہ = (UTF8) (ch | پہلا_Byte_Mark[بائٹاں_منزل_Write]);
        }
        نشانہ += بائٹاں_منزل_Write;
    }
    *سروت_شروع = سروت_2;
    *نشانہ_شروع = نشانہ;
    أرجع نتیجہ_2;
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

ساكن Boolean isLegalUTF8(ثابت UTF8 *سروت_2, صحيح لمائی_2) {
    UTF8 a;
    ثابت UTF8 *srcptr = سروت_2+لمائی_2;
    بدل (لمائی_2) {
    افتراضي: أرجع خطأ;
        /* Everything else falls through when "true"... */
    حالة 4: إذا ((a = (*--srcptr)) < 0x80 || a > 0xBF) أرجع خطأ;
    حالة 3: إذا ((a = (*--srcptr)) < 0x80 || a > 0xBF) أرجع خطأ;
    حالة 2: إذا ((a = (*--srcptr)) < 0x80 || a > 0xBF) أرجع خطأ;

        بدل (*سروت_2) {
            /* no fall-through in this inner switch */
            حالة 0xE0: إذا (a < 0xA0) أرجع خطأ; اكسر;
            حالة 0xED: إذا (a > 0x9F) أرجع خطأ; اكسر;
            حالة 0xF0: إذا (a < 0x90) أرجع خطأ; اكسر;
            حالة 0xF4: إذا (a > 0x8F) أرجع خطأ; اكسر;
            افتراضي:   إذا (a < 0x80) أرجع خطأ;
        }

    حالة 1: إذا (*سروت_2 >= 0x80 && *سروت_2 < 0xC2) أرجع خطأ;
    }
    إذا (*سروت_2 > 0xF4) أرجع خطأ;
    أرجع سچ;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 sequence is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8Sequence(ثابت UTF8 *سروت_2, ثابت UTF8 *سروت_سرا) {
    صحيح لمائی_2 = trailing_بائٹاں_For_UTF8[*سروت_2]+1;
    إذا (لمائی_2 > سروت_سرا - سروت_2) {
        أرجع خطأ;
    }
    أرجع isLegalUTF8(سروت_2, لمائی_2);
}

/*
 * Exported function to return the size of the first utf-8 code unit sequence,
 * Or 0 if the sequence is not valid;
 */
بنانشانی get_UTF8_Sequence_ناپ(ثابت UTF8 *سروت_2, ثابت UTF8 *سروت_سرا) {
  صحيح لمائی_2 = trailing_بائٹاں_For_UTF8[*سروت_2] + 1;
  أرجع (لمائی_2 <= سروت_سرا - سروت_2 && isLegalUTF8(سروت_2, لمائی_2)) ? لمائی_2
                                                                       : 0;
}

/* --------------------------------------------------------------------- */

بنانشانی لبھو_Maximal_Subpart_دا_Ill_Formed_UTF8_Sequence(ثابت UTF8 *سروت_2,
                                                   ثابت UTF8 *سروت_سرا) {
  UTF8 b1, b2, b3;

  أكد(!isLegalUTF8Sequence(سروت_2, سروت_سرا));

  /*
   * Unicode 6.3.0, D93b:
   *
   *   Maximal subpart of an ill-formed subsequence: The longest code unit
   *   subsequence starting at an unconvertible offset that is either:
   *   a. the initial subsequence of a well-formed code unit sequence, or
   *   b. a subsequence of length one.
   */

  إذا (سروت_2 == سروت_سرا)
    أرجع 0;

  /*
   * Perform case analysis.  See Unicode 6.3.0, Table 3-7. Well-Formed UTF-8
   * Byte Sequences.
   */

  b1 = *سروت_2;
  ++سروت_2;
  إذا (b1 >= 0xC2 && b1 <= 0xDF) {
    /*
     * First byte is valid, but we know that this code unit sequence is
     * invalid, so the maximal subpart has to end after the first byte.
     */
    أرجع 1;
  }

  إذا (سروت_2 == سروت_سرا)
    أرجع 1;

  b2 = *سروت_2;
  ++سروت_2;

  إذا (b1 == 0xE0) {
    أرجع (b2 >= 0xA0 && b2 <= 0xBF) ? 2 : 1;
  }
  إذا (b1 >= 0xE1 && b1 <= 0xEC) {
    أرجع (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  إذا (b1 == 0xED) {
    أرجع (b2 >= 0x80 && b2 <= 0x9F) ? 2 : 1;
  }
  إذا (b1 >= 0xEE && b1 <= 0xEF) {
    أرجع (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  إذا (b1 == 0xF0) {
    إذا (b2 >= 0x90 && b2 <= 0xBF) {
      إذا (سروت_2 == سروت_سرا)
        أرجع 2;

      b3 = *سروت_2;
      أرجع (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    أرجع 1;
  }
  إذا (b1 >= 0xF1 && b1 <= 0xF3) {
    إذا (b2 >= 0x80 && b2 <= 0xBF) {
      إذا (سروت_2 == سروت_سرا)
        أرجع 2;

      b3 = *سروت_2;
      أرجع (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    أرجع 1;
  }
  إذا (b1 == 0xF4) {
    إذا (b2 >= 0x80 && b2 <= 0x8F) {
      إذا (سروت_2 == سروت_سرا)
        أرجع 2;

      b3 = *سروت_2;
      أرجع (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    أرجع 1;
  }

  أكد((b1 >= 0x80 && b1 <= 0xC1) || b1 >= 0xF5);
  /*
   * There are no valid sequences that start with these bytes.  Maximal subpart
   * is defined to have length 1 in these cases.
   */
  أرجع 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return the total number of bytes in a codepoint
 * represented in UTF-8, given the value of the first byte.
 */
بنانشانی get_عدد_بائٹاں_For_UTF8(UTF8 پہلا_2) {
  أرجع trailing_بائٹاں_For_UTF8[پہلا_2] + 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 string is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8String(ثابت UTF8 **سروت_2, ثابت UTF8 *سروت_سرا) {
    طالما (*سروت_2 != سروت_سرا) {
        صحيح لمائی_2 = trailing_بائٹاں_For_UTF8[**سروت_2] + 1;
        إذا (لمائی_2 > سروت_سرا - *سروت_2 || !isLegalUTF8(*سروت_2, لمائی_2))
            أرجع خطأ;
        *سروت_2 += لمائی_2;
    }
    أرجع سچ;
}

/* --------------------------------------------------------------------- */

Conversion_نتیجہ ConvertUTF8toUTF16 (
        ثابت UTF8** سروت_شروع, ثابت UTF8* سروت_سرا,
        UTF16** نشانہ_شروع, UTF16* نشانہ_سرا, ConversionFlags flags) {
    Conversion_نتیجہ نتیجہ_2 = conversionOK;
    ثابت UTF8* سروت_2 = *سروت_شروع;
    UTF16* نشانہ = *نشانہ_شروع;
    طالما (سروت_2 < سروت_سرا) {
        UTF32 ch = 0;
        بنانشانی چھوٹا extra_بائٹاں_منزل_Read = trailing_بائٹاں_For_UTF8[*سروت_2];
        إذا (extra_بائٹاں_منزل_Read >= سروت_سرا - سروت_2) {
            نتیجہ_2 = سروت_Exhausted; اكسر;
        }
        /* Do this check whether lenient or strict */
        إذا (!isLegalUTF8(سروت_2, extra_بائٹاں_منزل_Read+1)) {
            نتیجہ_2 = سروت_Illegal;
            اكسر;
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        بدل (extra_بائٹاں_منزل_Read) {
            حالة 5: ch += *سروت_2++; ch <<= 6; /* remember, illegal UTF-8 */
            حالة 4: ch += *سروت_2++; ch <<= 6; /* remember, illegal UTF-8 */
            حالة 3: ch += *سروت_2++; ch <<= 6;
            حالة 2: ch += *سروت_2++; ch <<= 6;
            حالة 1: ch += *سروت_2++; ch <<= 6;
            حالة 0: ch += *سروت_2++;
        }
        ch -= offsetsFromUTF8[extra_بائٹاں_منزل_Read];

        إذا (نشانہ >= نشانہ_سرا) {
            سروت_2 -= (extra_بائٹاں_منزل_Read+1); /* Back up source pointer! */
            نتیجہ_2 = نشانہ_Exhausted; اكسر;
        }
        إذا (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32 */
            إذا (ch >= UNI_SUR_بلند_شروع && ch <= UNI_SUR_کم_سرا) {
                إذا (flags == strictConversion) {
                    سروت_2 -= (extra_بائٹاں_منزل_Read+1); /* return to the illegal value itself */
                    نتیجہ_2 = سروت_Illegal;
                    اكسر;
                } وإلا {
                    *نشانہ++ = UNI_REPLACEMENT_CHAR;
                }
            } وإلا {
                *نشانہ++ = (UTF16)ch; /* normal case */
            }
        } وإلا إذا (ch > UNI_MAX_UTF16) {
            إذا (flags == strictConversion) {
                نتیجہ_2 = سروت_Illegal;
                سروت_2 -= (extra_بائٹاں_منزل_Read+1); /* return to the start */
                اكسر; /* Bail out; shouldn't continue */
            } وإلا {
                *نشانہ++ = UNI_REPLACEMENT_CHAR;
            }
        } وإلا {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            إذا (نشانہ + 1 >= نشانہ_سرا) {
                سروت_2 -= (extra_بائٹاں_منزل_Read+1); /* Back up source pointer! */
                نتیجہ_2 = نشانہ_Exhausted; اكسر;
            }
            ch -= half_ادھار;
            *نشانہ++ = (UTF16)((ch >> half_کھسکاؤ) + UNI_SUR_بلند_شروع);
            *نشانہ++ = (UTF16)((ch & half_نقاب) + UNI_SUR_کم_شروع);
        }
    }
    *سروت_شروع = سروت_2;
    *نشانہ_شروع = نشانہ;
    أرجع نتیجہ_2;
}

/* --------------------------------------------------------------------- */

ساكن Conversion_نتیجہ ConvertUTF8toUTF32Impl(
        ثابت UTF8** سروت_شروع, ثابت UTF8* سروت_سرا,
        UTF32** نشانہ_شروع, UTF32* نشانہ_سرا, ConversionFlags flags,
        Boolean داخلہ_Is_Partial) {
    Conversion_نتیجہ نتیجہ_2 = conversionOK;
    ثابت UTF8* سروت_2 = *سروت_شروع;
    UTF32* نشانہ = *نشانہ_شروع;
    طالما (سروت_2 < سروت_سرا) {
        UTF32 ch = 0;
        بنانشانی چھوٹا extra_بائٹاں_منزل_Read = trailing_بائٹاں_For_UTF8[*سروت_2];
        إذا (extra_بائٹاں_منزل_Read >= سروت_سرا - سروت_2) {
            إذا (flags == strictConversion || داخلہ_Is_Partial) {
                نتیجہ_2 = سروت_Exhausted;
                اكسر;
            } وإلا {
                نتیجہ_2 = سروت_Illegal;

                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                سروت_2 += لبھو_Maximal_Subpart_دا_Ill_Formed_UTF8_Sequence(سروت_2,
                                                                    سروت_سرا);
                *نشانہ++ = UNI_REPLACEMENT_CHAR;
                استمر;
            }
        }
        إذا (نشانہ >= نشانہ_سرا) {
            نتیجہ_2 = نشانہ_Exhausted; اكسر;
        }

        /* Do this check whether lenient or strict */
        إذا (!isLegalUTF8(سروت_2, extra_بائٹاں_منزل_Read+1)) {
            نتیجہ_2 = سروت_Illegal;
            إذا (flags == strictConversion) {
                /* Abort conversion. */
                اكسر;
            } وإلا {
                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                سروت_2 += لبھو_Maximal_Subpart_دا_Ill_Formed_UTF8_Sequence(سروت_2,
                                                                    سروت_سرا);
                *نشانہ++ = UNI_REPLACEMENT_CHAR;
                استمر;
            }
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        بدل (extra_بائٹاں_منزل_Read) {
            حالة 5: ch += *سروت_2++; ch <<= 6;
            حالة 4: ch += *سروت_2++; ch <<= 6;
            حالة 3: ch += *سروت_2++; ch <<= 6;
            حالة 2: ch += *سروت_2++; ch <<= 6;
            حالة 1: ch += *سروت_2++; ch <<= 6;
            حالة 0: ch += *سروت_2++;
        }
        ch -= offsetsFromUTF8[extra_بائٹاں_منزل_Read];

        إذا (ch <= UNI_MAX_LEGAL_UTF32) {
            /*
             * UTF-16 surrogate values are illegal in UTF-32, and anything
             * over Plane 17 (> 0x10FFFF) is illegal.
             */
            إذا (ch >= UNI_SUR_بلند_شروع && ch <= UNI_SUR_کم_سرا) {
                إذا (flags == strictConversion) {
                    سروت_2 -= (extra_بائٹاں_منزل_Read+1); /* return to the illegal value itself */
                    نتیجہ_2 = سروت_Illegal;
                    اكسر;
                } وإلا {
                    *نشانہ++ = UNI_REPLACEMENT_CHAR;
                }
            } وإلا {
                *نشانہ++ = ch;
            }
        } وإلا { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
            نتیجہ_2 = سروت_Illegal;
            *نشانہ++ = UNI_REPLACEMENT_CHAR;
        }
    }
    *سروت_شروع = سروت_2;
    *نشانہ_شروع = نشانہ;
    أرجع نتیجہ_2;
}

Conversion_نتیجہ ConvertUTF8toUTF32Partial(ثابت UTF8 **سروت_شروع,
                                           ثابت UTF8 *سروت_سرا,
                                           UTF32 **نشانہ_شروع,
                                           UTF32 *نشانہ_سرا,
                                           ConversionFlags flags) {
  أرجع ConvertUTF8toUTF32Impl(سروت_شروع, سروت_سرا, نشانہ_شروع, نشانہ_سرا,
                                flags, /*InputIsPartial=*/سچ);
}

Conversion_نتیجہ ConvertUTF8toUTF32(ثابت UTF8 **سروت_شروع,
                                    ثابت UTF8 *سروت_سرا, UTF32 **نشانہ_شروع,
                                    UTF32 *نشانہ_سرا, ConversionFlags flags) {
  أرجع ConvertUTF8toUTF32Impl(سروت_شروع, سروت_سرا, نشانہ_شروع, نشانہ_سرا,
                                flags, /*InputIsPartial=*/خطأ);
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
