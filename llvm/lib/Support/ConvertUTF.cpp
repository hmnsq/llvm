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
#اگر_تعریف_شده CVTUTF_DEBUG
#ضمن <stdio.h>
#پایان_اگر
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
#پایان_اگر
#اگر_تعریف_نشده ConvertUTF_DISABLE_WARNINGS
# define ConvertUTF_DISABLE_WARNINGS
#پایان_اگر
#اگر_تعریف_نشده ConvertUTF_RESTORE_WARNINGS
# define ConvertUTF_RESTORE_WARNINGS
#پایان_اگر

ConvertUTF_DISABLE_WARNINGS

نطاق llvm {

ساكن ثابت صحيح half_لغزش  = 10; /* used for shifting by 10 bits */

ساكن ثابت UTF32 half_پایه = 0x0010000UL;
ساكن ثابت UTF32 half_ماسک = 0x3FFUL;

#تعریف_کن UNI_SUR_بالا_شروع  (UTF32)0xD800
#تعریف_کن UNI_SUR_بالا_انتها    (UTF32)0xDBFF
#تعریف_کن UNI_SUR_کم_شروع   (UTF32)0xDC00
#تعریف_کن UNI_SUR_کم_انتها     (UTF32)0xDFFF

/* --------------------------------------------------------------------- */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
ساكن ثابت حرف trailing_بایتها_For_UTF8[256] = {
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
ساكن ثابت UTF8 اولی_Byte_Mark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */


/* --------------------------------------------------------------------- */

Conversion_نتیجه ConvertUTF32toUTF16 (
        ثابت UTF32** منبع_شروع, ثابت UTF32* منبع_انتها,
        UTF16** هدف_شروع, UTF16* هدف_انتها, ConversionFlags flags) {
    Conversion_نتیجه نتیجه_2 = conversionOK;
    ثابت UTF32* منبع_2 = *منبع_شروع;
    UTF16* هدف = *هدف_شروع;
    طالما (منبع_2 < منبع_انتها) {
        UTF32 ch;
        إذا (هدف >= هدف_انتها) {
            نتیجه_2 = هدف_Exhausted; اكسر;
        }
        ch = *منبع_2++;
        إذا (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
            إذا (ch >= UNI_SUR_بالا_شروع && ch <= UNI_SUR_کم_انتها) {
                إذا (flags == strictConversion) {
                    --منبع_2; /* return to the illegal value itself */
                    نتیجه_2 = منبع_Illegal;
                    اكسر;
                } وإلا {
                    *هدف++ = UNI_REPLACEMENT_CHAR;
                }
            } وإلا {
                *هدف++ = (UTF16)ch; /* normal case */
            }
        } وإلا إذا (ch > UNI_MAX_LEGAL_UTF32) {
            إذا (flags == strictConversion) {
                نتیجه_2 = منبع_Illegal;
            } وإلا {
                *هدف++ = UNI_REPLACEMENT_CHAR;
            }
        } وإلا {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            إذا (هدف + 1 >= هدف_انتها) {
                --منبع_2; /* Back up source pointer! */
                نتیجه_2 = هدف_Exhausted; اكسر;
            }
            ch -= half_پایه;
            *هدف++ = (UTF16)((ch >> half_لغزش) + UNI_SUR_بالا_شروع);
            *هدف++ = (UTF16)((ch & half_ماسک) + UNI_SUR_کم_شروع);
        }
    }
    *منبع_شروع = منبع_2;
    *هدف_شروع = هدف;
    أرجع نتیجه_2;
}

/* --------------------------------------------------------------------- */

Conversion_نتیجه ConvertUTF16toUTF32 (
        ثابت UTF16** منبع_شروع, ثابت UTF16* منبع_انتها,
        UTF32** هدف_شروع, UTF32* هدف_انتها, ConversionFlags flags) {
    Conversion_نتیجه نتیجه_2 = conversionOK;
    ثابت UTF16* منبع_2 = *منبع_شروع;
    UTF32* هدف = *هدف_شروع;
    UTF32 ch, ch2;
    طالما (منبع_2 < منبع_انتها) {
        ثابت UTF16* old_منبع = منبع_2; /*  In case we have to back up because of target overflow. */
        ch = *منبع_2++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        إذا (ch >= UNI_SUR_بالا_شروع && ch <= UNI_SUR_بالا_انتها) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            إذا (منبع_2 < منبع_انتها) {
                ch2 = *منبع_2;
                /* If it's a low surrogate, convert to UTF32. */
                إذا (ch2 >= UNI_SUR_کم_شروع && ch2 <= UNI_SUR_کم_انتها) {
                    ch = ((ch - UNI_SUR_بالا_شروع) << half_لغزش)
                        + (ch2 - UNI_SUR_کم_شروع) + half_پایه;
                    ++منبع_2;
                } وإلا إذا (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --منبع_2; /* return to the illegal value itself */
                    نتیجه_2 = منبع_Illegal;
                    اكسر;
                }
            } وإلا { /* We don't have the 16 bits following the high surrogate. */
                --منبع_2; /* return to the high surrogate */
                نتیجه_2 = منبع_Exhausted;
                اكسر;
            }
        } وإلا إذا (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            إذا (ch >= UNI_SUR_کم_شروع && ch <= UNI_SUR_کم_انتها) {
                --منبع_2; /* return to the illegal value itself */
                نتیجه_2 = منبع_Illegal;
                اكسر;
            }
        }
        إذا (هدف >= هدف_انتها) {
            منبع_2 = old_منبع; /* Back up source pointer! */
            نتیجه_2 = هدف_Exhausted; اكسر;
        }
        *هدف++ = ch;
    }
    *منبع_شروع = منبع_2;
    *هدف_شروع = هدف;
#اگر_تعریف_شده CVTUTF_DEBUG
إذا (نتیجه_2 == منبع_Illegal) {
    fprintf(stderr, "ConvertUTF16toUTF32 illegal seq 0x%04x,%04x\n", ch, ch2);
    fflush(stderr);
}
#پایان_اگر
    أرجع نتیجه_2;
}
Conversion_نتیجه ConvertUTF16toUTF8 (
        ثابت UTF16** منبع_شروع, ثابت UTF16* منبع_انتها,
        UTF8** هدف_شروع, UTF8* هدف_انتها, ConversionFlags flags) {
    Conversion_نتیجه نتیجه_2 = conversionOK;
    ثابت UTF16* منبع_2 = *منبع_شروع;
    UTF8* هدف = *هدف_شروع;
    طالما (منبع_2 < منبع_انتها) {
        UTF32 ch;
        بیعلامت کوتاه بایتها_مقصد_Write = 0;
        ثابت UTF32 byte_ماسک = 0xBF;
        ثابت UTF32 byteMark = 0x80;
        ثابت UTF16* old_منبع = منبع_2; /* In case we have to back up because of target overflow. */
        ch = *منبع_2++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        إذا (ch >= UNI_SUR_بالا_شروع && ch <= UNI_SUR_بالا_انتها) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            إذا (منبع_2 < منبع_انتها) {
                UTF32 ch2 = *منبع_2;
                /* If it's a low surrogate, convert to UTF32. */
                إذا (ch2 >= UNI_SUR_کم_شروع && ch2 <= UNI_SUR_کم_انتها) {
                    ch = ((ch - UNI_SUR_بالا_شروع) << half_لغزش)
                        + (ch2 - UNI_SUR_کم_شروع) + half_پایه;
                    ++منبع_2;
                } وإلا إذا (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --منبع_2; /* return to the illegal value itself */
                    نتیجه_2 = منبع_Illegal;
                    اكسر;
                }
            } وإلا { /* We don't have the 16 bits following the high surrogate. */
                --منبع_2; /* return to the high surrogate */
                نتیجه_2 = منبع_Exhausted;
                اكسر;
            }
        } وإلا إذا (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            إذا (ch >= UNI_SUR_کم_شروع && ch <= UNI_SUR_کم_انتها) {
                --منبع_2; /* return to the illegal value itself */
                نتیجه_2 = منبع_Illegal;
                اكسر;
            }
        }
        /* Figure out how many bytes the result will require */
        إذا (ch < (UTF32)0x80) {      بایتها_مقصد_Write = 1;
        } وإلا إذا (ch < (UTF32)0x800) {     بایتها_مقصد_Write = 2;
        } وإلا إذا (ch < (UTF32)0x10000) {   بایتها_مقصد_Write = 3;
        } وإلا إذا (ch < (UTF32)0x110000) {  بایتها_مقصد_Write = 4;
        } وإلا {                            بایتها_مقصد_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
        }

        هدف += بایتها_مقصد_Write;
        إذا (هدف > هدف_انتها) {
            منبع_2 = old_منبع; /* Back up source pointer! */
            هدف -= بایتها_مقصد_Write; نتیجه_2 = هدف_Exhausted; اكسر;
        }
        بدل (بایتها_مقصد_Write) { /* note: everything falls through. */
            حالة 4: *--هدف = (UTF8)((ch | byteMark) & byte_ماسک); ch >>= 6;
            حالة 3: *--هدف = (UTF8)((ch | byteMark) & byte_ماسک); ch >>= 6;
            حالة 2: *--هدف = (UTF8)((ch | byteMark) & byte_ماسک); ch >>= 6;
            حالة 1: *--هدف =  (UTF8)(ch | اولی_Byte_Mark[بایتها_مقصد_Write]);
        }
        هدف += بایتها_مقصد_Write;
    }
    *منبع_شروع = منبع_2;
    *هدف_شروع = هدف;
    أرجع نتیجه_2;
}

/* --------------------------------------------------------------------- */

Conversion_نتیجه ConvertUTF32toUTF8 (
        ثابت UTF32** منبع_شروع, ثابت UTF32* منبع_انتها,
        UTF8** هدف_شروع, UTF8* هدف_انتها, ConversionFlags flags) {
    Conversion_نتیجه نتیجه_2 = conversionOK;
    ثابت UTF32* منبع_2 = *منبع_شروع;
    UTF8* هدف = *هدف_شروع;
    طالما (منبع_2 < منبع_انتها) {
        UTF32 ch;
        بیعلامت کوتاه بایتها_مقصد_Write = 0;
        ثابت UTF32 byte_ماسک = 0xBF;
        ثابت UTF32 byteMark = 0x80;
        ch = *منبع_2++;
        إذا (flags == strictConversion ) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            إذا (ch >= UNI_SUR_بالا_شروع && ch <= UNI_SUR_کم_انتها) {
                --منبع_2; /* return to the illegal value itself */
                نتیجه_2 = منبع_Illegal;
                اكسر;
            }
        }
        /*
         * Figure out how many bytes the result will require. Turn any
         * illegally large UTF32 things (> Plane 17) into replacement chars.
         */
        إذا (ch < (UTF32)0x80) {      بایتها_مقصد_Write = 1;
        } وإلا إذا (ch < (UTF32)0x800) {     بایتها_مقصد_Write = 2;
        } وإلا إذا (ch < (UTF32)0x10000) {   بایتها_مقصد_Write = 3;
        } وإلا إذا (ch <= UNI_MAX_LEGAL_UTF32) {  بایتها_مقصد_Write = 4;
        } وإلا {                            بایتها_مقصد_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
                                            نتیجه_2 = منبع_Illegal;
        }

        هدف += بایتها_مقصد_Write;
        إذا (هدف > هدف_انتها) {
            --منبع_2; /* Back up source pointer! */
            هدف -= بایتها_مقصد_Write; نتیجه_2 = هدف_Exhausted; اكسر;
        }
        بدل (بایتها_مقصد_Write) { /* note: everything falls through. */
            حالة 4: *--هدف = (UTF8)((ch | byteMark) & byte_ماسک); ch >>= 6;
            حالة 3: *--هدف = (UTF8)((ch | byteMark) & byte_ماسک); ch >>= 6;
            حالة 2: *--هدف = (UTF8)((ch | byteMark) & byte_ماسک); ch >>= 6;
            حالة 1: *--هدف = (UTF8) (ch | اولی_Byte_Mark[بایتها_مقصد_Write]);
        }
        هدف += بایتها_مقصد_Write;
    }
    *منبع_شروع = منبع_2;
    *هدف_شروع = هدف;
    أرجع نتیجه_2;
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

ساكن Boolean isLegalUTF8(ثابت UTF8 *منبع_2, صحيح درازی_2) {
    UTF8 a;
    ثابت UTF8 *srcptr = منبع_2+درازی_2;
    بدل (درازی_2) {
    افتراضي: أرجع خطأ;
        /* Everything else falls through when "true"... */
    حالة 4: إذا ((a = (*--srcptr)) < 0x80 || a > 0xBF) أرجع خطأ;
    حالة 3: إذا ((a = (*--srcptr)) < 0x80 || a > 0xBF) أرجع خطأ;
    حالة 2: إذا ((a = (*--srcptr)) < 0x80 || a > 0xBF) أرجع خطأ;

        بدل (*منبع_2) {
            /* no fall-through in this inner switch */
            حالة 0xE0: إذا (a < 0xA0) أرجع خطأ; اكسر;
            حالة 0xED: إذا (a > 0x9F) أرجع خطأ; اكسر;
            حالة 0xF0: إذا (a < 0x90) أرجع خطأ; اكسر;
            حالة 0xF4: إذا (a > 0x8F) أرجع خطأ; اكسر;
            افتراضي:   إذا (a < 0x80) أرجع خطأ;
        }

    حالة 1: إذا (*منبع_2 >= 0x80 && *منبع_2 < 0xC2) أرجع خطأ;
    }
    إذا (*منبع_2 > 0xF4) أرجع خطأ;
    أرجع درست;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 sequence is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8Sequence(ثابت UTF8 *منبع_2, ثابت UTF8 *منبع_انتها) {
    صحيح درازی_2 = trailing_بایتها_For_UTF8[*منبع_2]+1;
    إذا (درازی_2 > منبع_انتها - منبع_2) {
        أرجع خطأ;
    }
    أرجع isLegalUTF8(منبع_2, درازی_2);
}

/*
 * Exported function to return the size of the first utf-8 code unit sequence,
 * Or 0 if the sequence is not valid;
 */
بیعلامت get_UTF8_Sequence_اندازه(ثابت UTF8 *منبع_2, ثابت UTF8 *منبع_انتها) {
  صحيح درازی_2 = trailing_بایتها_For_UTF8[*منبع_2] + 1;
  أرجع (درازی_2 <= منبع_انتها - منبع_2 && isLegalUTF8(منبع_2, درازی_2)) ? درازی_2
                                                                       : 0;
}

/* --------------------------------------------------------------------- */

بیعلامت پیدا_کن_Maximal_Subpart_از_Ill_Formed_UTF8_Sequence(ثابت UTF8 *منبع_2,
                                                   ثابت UTF8 *منبع_انتها) {
  UTF8 b1, b2, b3;

  أكد(!isLegalUTF8Sequence(منبع_2, منبع_انتها));

  /*
   * Unicode 6.3.0, D93b:
   *
   *   Maximal subpart of an ill-formed subsequence: The longest code unit
   *   subsequence starting at an unconvertible offset that is either:
   *   a. the initial subsequence of a well-formed code unit sequence, or
   *   b. a subsequence of length one.
   */

  إذا (منبع_2 == منبع_انتها)
    أرجع 0;

  /*
   * Perform case analysis.  See Unicode 6.3.0, Table 3-7. Well-Formed UTF-8
   * Byte Sequences.
   */

  b1 = *منبع_2;
  ++منبع_2;
  إذا (b1 >= 0xC2 && b1 <= 0xDF) {
    /*
     * First byte is valid, but we know that this code unit sequence is
     * invalid, so the maximal subpart has to end after the first byte.
     */
    أرجع 1;
  }

  إذا (منبع_2 == منبع_انتها)
    أرجع 1;

  b2 = *منبع_2;
  ++منبع_2;

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
      إذا (منبع_2 == منبع_انتها)
        أرجع 2;

      b3 = *منبع_2;
      أرجع (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    أرجع 1;
  }
  إذا (b1 >= 0xF1 && b1 <= 0xF3) {
    إذا (b2 >= 0x80 && b2 <= 0xBF) {
      إذا (منبع_2 == منبع_انتها)
        أرجع 2;

      b3 = *منبع_2;
      أرجع (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    أرجع 1;
  }
  إذا (b1 == 0xF4) {
    إذا (b2 >= 0x80 && b2 <= 0x8F) {
      إذا (منبع_2 == منبع_انتها)
        أرجع 2;

      b3 = *منبع_2;
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
بیعلامت get_رقم_بایتها_For_UTF8(UTF8 اولی_2) {
  أرجع trailing_بایتها_For_UTF8[اولی_2] + 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 string is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8String(ثابت UTF8 **منبع_2, ثابت UTF8 *منبع_انتها) {
    طالما (*منبع_2 != منبع_انتها) {
        صحيح درازی_2 = trailing_بایتها_For_UTF8[**منبع_2] + 1;
        إذا (درازی_2 > منبع_انتها - *منبع_2 || !isLegalUTF8(*منبع_2, درازی_2))
            أرجع خطأ;
        *منبع_2 += درازی_2;
    }
    أرجع درست;
}

/* --------------------------------------------------------------------- */

Conversion_نتیجه ConvertUTF8toUTF16 (
        ثابت UTF8** منبع_شروع, ثابت UTF8* منبع_انتها,
        UTF16** هدف_شروع, UTF16* هدف_انتها, ConversionFlags flags) {
    Conversion_نتیجه نتیجه_2 = conversionOK;
    ثابت UTF8* منبع_2 = *منبع_شروع;
    UTF16* هدف = *هدف_شروع;
    طالما (منبع_2 < منبع_انتها) {
        UTF32 ch = 0;
        بیعلامت کوتاه extra_بایتها_مقصد_Read = trailing_بایتها_For_UTF8[*منبع_2];
        إذا (extra_بایتها_مقصد_Read >= منبع_انتها - منبع_2) {
            نتیجه_2 = منبع_Exhausted; اكسر;
        }
        /* Do this check whether lenient or strict */
        إذا (!isLegalUTF8(منبع_2, extra_بایتها_مقصد_Read+1)) {
            نتیجه_2 = منبع_Illegal;
            اكسر;
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        بدل (extra_بایتها_مقصد_Read) {
            حالة 5: ch += *منبع_2++; ch <<= 6; /* remember, illegal UTF-8 */
            حالة 4: ch += *منبع_2++; ch <<= 6; /* remember, illegal UTF-8 */
            حالة 3: ch += *منبع_2++; ch <<= 6;
            حالة 2: ch += *منبع_2++; ch <<= 6;
            حالة 1: ch += *منبع_2++; ch <<= 6;
            حالة 0: ch += *منبع_2++;
        }
        ch -= offsetsFromUTF8[extra_بایتها_مقصد_Read];

        إذا (هدف >= هدف_انتها) {
            منبع_2 -= (extra_بایتها_مقصد_Read+1); /* Back up source pointer! */
            نتیجه_2 = هدف_Exhausted; اكسر;
        }
        إذا (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32 */
            إذا (ch >= UNI_SUR_بالا_شروع && ch <= UNI_SUR_کم_انتها) {
                إذا (flags == strictConversion) {
                    منبع_2 -= (extra_بایتها_مقصد_Read+1); /* return to the illegal value itself */
                    نتیجه_2 = منبع_Illegal;
                    اكسر;
                } وإلا {
                    *هدف++ = UNI_REPLACEMENT_CHAR;
                }
            } وإلا {
                *هدف++ = (UTF16)ch; /* normal case */
            }
        } وإلا إذا (ch > UNI_MAX_UTF16) {
            إذا (flags == strictConversion) {
                نتیجه_2 = منبع_Illegal;
                منبع_2 -= (extra_بایتها_مقصد_Read+1); /* return to the start */
                اكسر; /* Bail out; shouldn't continue */
            } وإلا {
                *هدف++ = UNI_REPLACEMENT_CHAR;
            }
        } وإلا {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            إذا (هدف + 1 >= هدف_انتها) {
                منبع_2 -= (extra_بایتها_مقصد_Read+1); /* Back up source pointer! */
                نتیجه_2 = هدف_Exhausted; اكسر;
            }
            ch -= half_پایه;
            *هدف++ = (UTF16)((ch >> half_لغزش) + UNI_SUR_بالا_شروع);
            *هدف++ = (UTF16)((ch & half_ماسک) + UNI_SUR_کم_شروع);
        }
    }
    *منبع_شروع = منبع_2;
    *هدف_شروع = هدف;
    أرجع نتیجه_2;
}

/* --------------------------------------------------------------------- */

ساكن Conversion_نتیجه ConvertUTF8toUTF32Impl(
        ثابت UTF8** منبع_شروع, ثابت UTF8* منبع_انتها,
        UTF32** هدف_شروع, UTF32* هدف_انتها, ConversionFlags flags,
        Boolean درون_داد_Is_Partial) {
    Conversion_نتیجه نتیجه_2 = conversionOK;
    ثابت UTF8* منبع_2 = *منبع_شروع;
    UTF32* هدف = *هدف_شروع;
    طالما (منبع_2 < منبع_انتها) {
        UTF32 ch = 0;
        بیعلامت کوتاه extra_بایتها_مقصد_Read = trailing_بایتها_For_UTF8[*منبع_2];
        إذا (extra_بایتها_مقصد_Read >= منبع_انتها - منبع_2) {
            إذا (flags == strictConversion || درون_داد_Is_Partial) {
                نتیجه_2 = منبع_Exhausted;
                اكسر;
            } وإلا {
                نتیجه_2 = منبع_Illegal;

                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                منبع_2 += پیدا_کن_Maximal_Subpart_از_Ill_Formed_UTF8_Sequence(منبع_2,
                                                                    منبع_انتها);
                *هدف++ = UNI_REPLACEMENT_CHAR;
                استمر;
            }
        }
        إذا (هدف >= هدف_انتها) {
            نتیجه_2 = هدف_Exhausted; اكسر;
        }

        /* Do this check whether lenient or strict */
        إذا (!isLegalUTF8(منبع_2, extra_بایتها_مقصد_Read+1)) {
            نتیجه_2 = منبع_Illegal;
            إذا (flags == strictConversion) {
                /* Abort conversion. */
                اكسر;
            } وإلا {
                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                منبع_2 += پیدا_کن_Maximal_Subpart_از_Ill_Formed_UTF8_Sequence(منبع_2,
                                                                    منبع_انتها);
                *هدف++ = UNI_REPLACEMENT_CHAR;
                استمر;
            }
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        بدل (extra_بایتها_مقصد_Read) {
            حالة 5: ch += *منبع_2++; ch <<= 6;
            حالة 4: ch += *منبع_2++; ch <<= 6;
            حالة 3: ch += *منبع_2++; ch <<= 6;
            حالة 2: ch += *منبع_2++; ch <<= 6;
            حالة 1: ch += *منبع_2++; ch <<= 6;
            حالة 0: ch += *منبع_2++;
        }
        ch -= offsetsFromUTF8[extra_بایتها_مقصد_Read];

        إذا (ch <= UNI_MAX_LEGAL_UTF32) {
            /*
             * UTF-16 surrogate values are illegal in UTF-32, and anything
             * over Plane 17 (> 0x10FFFF) is illegal.
             */
            إذا (ch >= UNI_SUR_بالا_شروع && ch <= UNI_SUR_کم_انتها) {
                إذا (flags == strictConversion) {
                    منبع_2 -= (extra_بایتها_مقصد_Read+1); /* return to the illegal value itself */
                    نتیجه_2 = منبع_Illegal;
                    اكسر;
                } وإلا {
                    *هدف++ = UNI_REPLACEMENT_CHAR;
                }
            } وإلا {
                *هدف++ = ch;
            }
        } وإلا { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
            نتیجه_2 = منبع_Illegal;
            *هدف++ = UNI_REPLACEMENT_CHAR;
        }
    }
    *منبع_شروع = منبع_2;
    *هدف_شروع = هدف;
    أرجع نتیجه_2;
}

Conversion_نتیجه ConvertUTF8toUTF32Partial(ثابت UTF8 **منبع_شروع,
                                           ثابت UTF8 *منبع_انتها,
                                           UTF32 **هدف_شروع,
                                           UTF32 *هدف_انتها,
                                           ConversionFlags flags) {
  أرجع ConvertUTF8toUTF32Impl(منبع_شروع, منبع_انتها, هدف_شروع, هدف_انتها,
                                flags, /*InputIsPartial=*/درست);
}

Conversion_نتیجه ConvertUTF8toUTF32(ثابت UTF8 **منبع_شروع,
                                    ثابت UTF8 *منبع_انتها, UTF32 **هدف_شروع,
                                    UTF32 *هدف_انتها, ConversionFlags flags) {
  أرجع ConvertUTF8toUTF32Impl(منبع_شروع, منبع_انتها, هدف_شروع, هدف_انتها,
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
