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
#که_تعریف_شوی CVTUTF_DEBUG
#ضمن <stdio.h>
#د_که_پای
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
#د_که_پای
#که_نه_تعریف_شوی ConvertUTF_DISABLE_WARNINGS
# define ConvertUTF_DISABLE_WARNINGS
#د_که_پای
#که_نه_تعریف_شوی ConvertUTF_RESTORE_WARNINGS
# define ConvertUTF_RESTORE_WARNINGS
#د_که_پای

ConvertUTF_DISABLE_WARNINGS

نطاق llvm {

ساكن ثابت صحيح half_ښويدنه  = 10; /* used for shifting by 10 bits */

ساكن ثابت UTF32 half_بنسټ = 0x0010000UL;
ساكن ثابت UTF32 half_ماسک = 0x3FFUL;

#تعریف_کړه UNI_SUR_لوړ_پیل  (UTF32)0xD800
#تعریف_کړه UNI_SUR_لوړ_پای_ټکی    (UTF32)0xDBFF
#تعریف_کړه UNI_SUR_ټیټ_پیل   (UTF32)0xDC00
#تعریف_کړه UNI_SUR_ټیټ_پای_ټکی     (UTF32)0xDFFF

/* --------------------------------------------------------------------- */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
ساكن ثابت حرف trailing_بایټونه_For_UTF8[256] = {
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
ساكن ثابت UTF8 لومړی_Byte_Mark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */


/* --------------------------------------------------------------------- */

Conversion_پایله ConvertUTF32toUTF16 (
        ثابت UTF32** سرچینه_پیل, ثابت UTF32* سرچینه_پای_ټکی,
        UTF16** موخه_پیل, UTF16* موخه_پای_ټکی, ConversionFlags flags) {
    Conversion_پایله پایله_2 = conversionOK;
    ثابت UTF32* سرچینه_2 = *سرچینه_پیل;
    UTF16* موخه = *موخه_پیل;
    طالما (سرچینه_2 < سرچینه_پای_ټکی) {
        UTF32 ch;
        إذا (موخه >= موخه_پای_ټکی) {
            پایله_2 = موخه_Exhausted; اكسر;
        }
        ch = *سرچینه_2++;
        إذا (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
            إذا (ch >= UNI_SUR_لوړ_پیل && ch <= UNI_SUR_ټیټ_پای_ټکی) {
                إذا (flags == strictConversion) {
                    --سرچینه_2; /* return to the illegal value itself */
                    پایله_2 = سرچینه_Illegal;
                    اكسر;
                } وإلا {
                    *موخه++ = UNI_REPLACEMENT_CHAR;
                }
            } وإلا {
                *موخه++ = (UTF16)ch; /* normal case */
            }
        } وإلا إذا (ch > UNI_MAX_LEGAL_UTF32) {
            إذا (flags == strictConversion) {
                پایله_2 = سرچینه_Illegal;
            } وإلا {
                *موخه++ = UNI_REPLACEMENT_CHAR;
            }
        } وإلا {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            إذا (موخه + 1 >= موخه_پای_ټکی) {
                --سرچینه_2; /* Back up source pointer! */
                پایله_2 = موخه_Exhausted; اكسر;
            }
            ch -= half_بنسټ;
            *موخه++ = (UTF16)((ch >> half_ښويدنه) + UNI_SUR_لوړ_پیل);
            *موخه++ = (UTF16)((ch & half_ماسک) + UNI_SUR_ټیټ_پیل);
        }
    }
    *سرچینه_پیل = سرچینه_2;
    *موخه_پیل = موخه;
    أرجع پایله_2;
}

/* --------------------------------------------------------------------- */

Conversion_پایله ConvertUTF16toUTF32 (
        ثابت UTF16** سرچینه_پیل, ثابت UTF16* سرچینه_پای_ټکی,
        UTF32** موخه_پیل, UTF32* موخه_پای_ټکی, ConversionFlags flags) {
    Conversion_پایله پایله_2 = conversionOK;
    ثابت UTF16* سرچینه_2 = *سرچینه_پیل;
    UTF32* موخه = *موخه_پیل;
    UTF32 ch, ch2;
    طالما (سرچینه_2 < سرچینه_پای_ټکی) {
        ثابت UTF16* old_سرچینه = سرچینه_2; /*  In case we have to back up because of target overflow. */
        ch = *سرچینه_2++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        إذا (ch >= UNI_SUR_لوړ_پیل && ch <= UNI_SUR_لوړ_پای_ټکی) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            إذا (سرچینه_2 < سرچینه_پای_ټکی) {
                ch2 = *سرچینه_2;
                /* If it's a low surrogate, convert to UTF32. */
                إذا (ch2 >= UNI_SUR_ټیټ_پیل && ch2 <= UNI_SUR_ټیټ_پای_ټکی) {
                    ch = ((ch - UNI_SUR_لوړ_پیل) << half_ښويدنه)
                        + (ch2 - UNI_SUR_ټیټ_پیل) + half_بنسټ;
                    ++سرچینه_2;
                } وإلا إذا (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --سرچینه_2; /* return to the illegal value itself */
                    پایله_2 = سرچینه_Illegal;
                    اكسر;
                }
            } وإلا { /* We don't have the 16 bits following the high surrogate. */
                --سرچینه_2; /* return to the high surrogate */
                پایله_2 = سرچینه_Exhausted;
                اكسر;
            }
        } وإلا إذا (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            إذا (ch >= UNI_SUR_ټیټ_پیل && ch <= UNI_SUR_ټیټ_پای_ټکی) {
                --سرچینه_2; /* return to the illegal value itself */
                پایله_2 = سرچینه_Illegal;
                اكسر;
            }
        }
        إذا (موخه >= موخه_پای_ټکی) {
            سرچینه_2 = old_سرچینه; /* Back up source pointer! */
            پایله_2 = موخه_Exhausted; اكسر;
        }
        *موخه++ = ch;
    }
    *سرچینه_پیل = سرچینه_2;
    *موخه_پیل = موخه;
#که_تعریف_شوی CVTUTF_DEBUG
إذا (پایله_2 == سرچینه_Illegal) {
    fprintf(stderr, "ConvertUTF16toUTF32 illegal seq 0x%04x,%04x\n", ch, ch2);
    fflush(stderr);
}
#د_که_پای
    أرجع پایله_2;
}
Conversion_پایله ConvertUTF16toUTF8 (
        ثابت UTF16** سرچینه_پیل, ثابت UTF16* سرچینه_پای_ټکی,
        UTF8** موخه_پیل, UTF8* موخه_پای_ټکی, ConversionFlags flags) {
    Conversion_پایله پایله_2 = conversionOK;
    ثابت UTF16* سرچینه_2 = *سرچینه_پیل;
    UTF8* موخه = *موخه_پیل;
    طالما (سرچینه_2 < سرچینه_پای_ټکی) {
        UTF32 ch;
        بینښه لنډ بایټونه_منزل_Write = 0;
        ثابت UTF32 byte_ماسک = 0xBF;
        ثابت UTF32 byteMark = 0x80;
        ثابت UTF16* old_سرچینه = سرچینه_2; /* In case we have to back up because of target overflow. */
        ch = *سرچینه_2++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        إذا (ch >= UNI_SUR_لوړ_پیل && ch <= UNI_SUR_لوړ_پای_ټکی) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            إذا (سرچینه_2 < سرچینه_پای_ټکی) {
                UTF32 ch2 = *سرچینه_2;
                /* If it's a low surrogate, convert to UTF32. */
                إذا (ch2 >= UNI_SUR_ټیټ_پیل && ch2 <= UNI_SUR_ټیټ_پای_ټکی) {
                    ch = ((ch - UNI_SUR_لوړ_پیل) << half_ښويدنه)
                        + (ch2 - UNI_SUR_ټیټ_پیل) + half_بنسټ;
                    ++سرچینه_2;
                } وإلا إذا (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --سرچینه_2; /* return to the illegal value itself */
                    پایله_2 = سرچینه_Illegal;
                    اكسر;
                }
            } وإلا { /* We don't have the 16 bits following the high surrogate. */
                --سرچینه_2; /* return to the high surrogate */
                پایله_2 = سرچینه_Exhausted;
                اكسر;
            }
        } وإلا إذا (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            إذا (ch >= UNI_SUR_ټیټ_پیل && ch <= UNI_SUR_ټیټ_پای_ټکی) {
                --سرچینه_2; /* return to the illegal value itself */
                پایله_2 = سرچینه_Illegal;
                اكسر;
            }
        }
        /* Figure out how many bytes the result will require */
        إذا (ch < (UTF32)0x80) {      بایټونه_منزل_Write = 1;
        } وإلا إذا (ch < (UTF32)0x800) {     بایټونه_منزل_Write = 2;
        } وإلا إذا (ch < (UTF32)0x10000) {   بایټونه_منزل_Write = 3;
        } وإلا إذا (ch < (UTF32)0x110000) {  بایټونه_منزل_Write = 4;
        } وإلا {                            بایټونه_منزل_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
        }

        موخه += بایټونه_منزل_Write;
        إذا (موخه > موخه_پای_ټکی) {
            سرچینه_2 = old_سرچینه; /* Back up source pointer! */
            موخه -= بایټونه_منزل_Write; پایله_2 = موخه_Exhausted; اكسر;
        }
        بدل (بایټونه_منزل_Write) { /* note: everything falls through. */
            حالة 4: *--موخه = (UTF8)((ch | byteMark) & byte_ماسک); ch >>= 6;
            حالة 3: *--موخه = (UTF8)((ch | byteMark) & byte_ماسک); ch >>= 6;
            حالة 2: *--موخه = (UTF8)((ch | byteMark) & byte_ماسک); ch >>= 6;
            حالة 1: *--موخه =  (UTF8)(ch | لومړی_Byte_Mark[بایټونه_منزل_Write]);
        }
        موخه += بایټونه_منزل_Write;
    }
    *سرچینه_پیل = سرچینه_2;
    *موخه_پیل = موخه;
    أرجع پایله_2;
}

/* --------------------------------------------------------------------- */

Conversion_پایله ConvertUTF32toUTF8 (
        ثابت UTF32** سرچینه_پیل, ثابت UTF32* سرچینه_پای_ټکی,
        UTF8** موخه_پیل, UTF8* موخه_پای_ټکی, ConversionFlags flags) {
    Conversion_پایله پایله_2 = conversionOK;
    ثابت UTF32* سرچینه_2 = *سرچینه_پیل;
    UTF8* موخه = *موخه_پیل;
    طالما (سرچینه_2 < سرچینه_پای_ټکی) {
        UTF32 ch;
        بینښه لنډ بایټونه_منزل_Write = 0;
        ثابت UTF32 byte_ماسک = 0xBF;
        ثابت UTF32 byteMark = 0x80;
        ch = *سرچینه_2++;
        إذا (flags == strictConversion ) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            إذا (ch >= UNI_SUR_لوړ_پیل && ch <= UNI_SUR_ټیټ_پای_ټکی) {
                --سرچینه_2; /* return to the illegal value itself */
                پایله_2 = سرچینه_Illegal;
                اكسر;
            }
        }
        /*
         * Figure out how many bytes the result will require. Turn any
         * illegally large UTF32 things (> Plane 17) into replacement chars.
         */
        إذا (ch < (UTF32)0x80) {      بایټونه_منزل_Write = 1;
        } وإلا إذا (ch < (UTF32)0x800) {     بایټونه_منزل_Write = 2;
        } وإلا إذا (ch < (UTF32)0x10000) {   بایټونه_منزل_Write = 3;
        } وإلا إذا (ch <= UNI_MAX_LEGAL_UTF32) {  بایټونه_منزل_Write = 4;
        } وإلا {                            بایټونه_منزل_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
                                            پایله_2 = سرچینه_Illegal;
        }

        موخه += بایټونه_منزل_Write;
        إذا (موخه > موخه_پای_ټکی) {
            --سرچینه_2; /* Back up source pointer! */
            موخه -= بایټونه_منزل_Write; پایله_2 = موخه_Exhausted; اكسر;
        }
        بدل (بایټونه_منزل_Write) { /* note: everything falls through. */
            حالة 4: *--موخه = (UTF8)((ch | byteMark) & byte_ماسک); ch >>= 6;
            حالة 3: *--موخه = (UTF8)((ch | byteMark) & byte_ماسک); ch >>= 6;
            حالة 2: *--موخه = (UTF8)((ch | byteMark) & byte_ماسک); ch >>= 6;
            حالة 1: *--موخه = (UTF8) (ch | لومړی_Byte_Mark[بایټونه_منزل_Write]);
        }
        موخه += بایټونه_منزل_Write;
    }
    *سرچینه_پیل = سرچینه_2;
    *موخه_پیل = موخه;
    أرجع پایله_2;
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

ساكن Boolean isLegalUTF8(ثابت UTF8 *سرچینه_2, صحيح اوږدتیا_2) {
    UTF8 a;
    ثابت UTF8 *srcptr = سرچینه_2+اوږدتیا_2;
    بدل (اوږدتیا_2) {
    افتراضي: أرجع خطأ;
        /* Everything else falls through when "true"... */
    حالة 4: إذا ((a = (*--srcptr)) < 0x80 || a > 0xBF) أرجع خطأ;
    حالة 3: إذا ((a = (*--srcptr)) < 0x80 || a > 0xBF) أرجع خطأ;
    حالة 2: إذا ((a = (*--srcptr)) < 0x80 || a > 0xBF) أرجع خطأ;

        بدل (*سرچینه_2) {
            /* no fall-through in this inner switch */
            حالة 0xE0: إذا (a < 0xA0) أرجع خطأ; اكسر;
            حالة 0xED: إذا (a > 0x9F) أرجع خطأ; اكسر;
            حالة 0xF0: إذا (a < 0x90) أرجع خطأ; اكسر;
            حالة 0xF4: إذا (a > 0x8F) أرجع خطأ; اكسر;
            افتراضي:   إذا (a < 0x80) أرجع خطأ;
        }

    حالة 1: إذا (*سرچینه_2 >= 0x80 && *سرچینه_2 < 0xC2) أرجع خطأ;
    }
    إذا (*سرچینه_2 > 0xF4) أرجع خطأ;
    أرجع ریښتیا;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 sequence is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8Sequence(ثابت UTF8 *سرچینه_2, ثابت UTF8 *سرچینه_پای_ټکی) {
    صحيح اوږدتیا_2 = trailing_بایټونه_For_UTF8[*سرچینه_2]+1;
    إذا (اوږدتیا_2 > سرچینه_پای_ټکی - سرچینه_2) {
        أرجع خطأ;
    }
    أرجع isLegalUTF8(سرچینه_2, اوږدتیا_2);
}

/*
 * Exported function to return the size of the first utf-8 code unit sequence,
 * Or 0 if the sequence is not valid;
 */
بینښه get_UTF8_Sequence_اندازه(ثابت UTF8 *سرچینه_2, ثابت UTF8 *سرچینه_پای_ټکی) {
  صحيح اوږدتیا_2 = trailing_بایټونه_For_UTF8[*سرچینه_2] + 1;
  أرجع (اوږدتیا_2 <= سرچینه_پای_ټکی - سرچینه_2 && isLegalUTF8(سرچینه_2, اوږدتیا_2)) ? اوږدتیا_2
                                                                       : 0;
}

/* --------------------------------------------------------------------- */

بینښه پیدا_کړه_Maximal_Subpart_د_Ill_Formed_UTF8_Sequence(ثابت UTF8 *سرچینه_2,
                                                   ثابت UTF8 *سرچینه_پای_ټکی) {
  UTF8 b1, b2, b3;

  أكد(!isLegalUTF8Sequence(سرچینه_2, سرچینه_پای_ټکی));

  /*
   * Unicode 6.3.0, D93b:
   *
   *   Maximal subpart of an ill-formed subsequence: The longest code unit
   *   subsequence starting at an unconvertible offset that is either:
   *   a. the initial subsequence of a well-formed code unit sequence, or
   *   b. a subsequence of length one.
   */

  إذا (سرچینه_2 == سرچینه_پای_ټکی)
    أرجع 0;

  /*
   * Perform case analysis.  See Unicode 6.3.0, Table 3-7. Well-Formed UTF-8
   * Byte Sequences.
   */

  b1 = *سرچینه_2;
  ++سرچینه_2;
  إذا (b1 >= 0xC2 && b1 <= 0xDF) {
    /*
     * First byte is valid, but we know that this code unit sequence is
     * invalid, so the maximal subpart has to end after the first byte.
     */
    أرجع 1;
  }

  إذا (سرچینه_2 == سرچینه_پای_ټکی)
    أرجع 1;

  b2 = *سرچینه_2;
  ++سرچینه_2;

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
      إذا (سرچینه_2 == سرچینه_پای_ټکی)
        أرجع 2;

      b3 = *سرچینه_2;
      أرجع (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    أرجع 1;
  }
  إذا (b1 >= 0xF1 && b1 <= 0xF3) {
    إذا (b2 >= 0x80 && b2 <= 0xBF) {
      إذا (سرچینه_2 == سرچینه_پای_ټکی)
        أرجع 2;

      b3 = *سرچینه_2;
      أرجع (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    أرجع 1;
  }
  إذا (b1 == 0xF4) {
    إذا (b2 >= 0x80 && b2 <= 0x8F) {
      إذا (سرچینه_2 == سرچینه_پای_ټکی)
        أرجع 2;

      b3 = *سرچینه_2;
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
بینښه get_عدد_بایټونه_For_UTF8(UTF8 لومړی_2) {
  أرجع trailing_بایټونه_For_UTF8[لومړی_2] + 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 string is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8String(ثابت UTF8 **سرچینه_2, ثابت UTF8 *سرچینه_پای_ټکی) {
    طالما (*سرچینه_2 != سرچینه_پای_ټکی) {
        صحيح اوږدتیا_2 = trailing_بایټونه_For_UTF8[**سرچینه_2] + 1;
        إذا (اوږدتیا_2 > سرچینه_پای_ټکی - *سرچینه_2 || !isLegalUTF8(*سرچینه_2, اوږدتیا_2))
            أرجع خطأ;
        *سرچینه_2 += اوږدتیا_2;
    }
    أرجع ریښتیا;
}

/* --------------------------------------------------------------------- */

Conversion_پایله ConvertUTF8toUTF16 (
        ثابت UTF8** سرچینه_پیل, ثابت UTF8* سرچینه_پای_ټکی,
        UTF16** موخه_پیل, UTF16* موخه_پای_ټکی, ConversionFlags flags) {
    Conversion_پایله پایله_2 = conversionOK;
    ثابت UTF8* سرچینه_2 = *سرچینه_پیل;
    UTF16* موخه = *موخه_پیل;
    طالما (سرچینه_2 < سرچینه_پای_ټکی) {
        UTF32 ch = 0;
        بینښه لنډ extra_بایټونه_منزل_Read = trailing_بایټونه_For_UTF8[*سرچینه_2];
        إذا (extra_بایټونه_منزل_Read >= سرچینه_پای_ټکی - سرچینه_2) {
            پایله_2 = سرچینه_Exhausted; اكسر;
        }
        /* Do this check whether lenient or strict */
        إذا (!isLegalUTF8(سرچینه_2, extra_بایټونه_منزل_Read+1)) {
            پایله_2 = سرچینه_Illegal;
            اكسر;
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        بدل (extra_بایټونه_منزل_Read) {
            حالة 5: ch += *سرچینه_2++; ch <<= 6; /* remember, illegal UTF-8 */
            حالة 4: ch += *سرچینه_2++; ch <<= 6; /* remember, illegal UTF-8 */
            حالة 3: ch += *سرچینه_2++; ch <<= 6;
            حالة 2: ch += *سرچینه_2++; ch <<= 6;
            حالة 1: ch += *سرچینه_2++; ch <<= 6;
            حالة 0: ch += *سرچینه_2++;
        }
        ch -= offsetsFromUTF8[extra_بایټونه_منزل_Read];

        إذا (موخه >= موخه_پای_ټکی) {
            سرچینه_2 -= (extra_بایټونه_منزل_Read+1); /* Back up source pointer! */
            پایله_2 = موخه_Exhausted; اكسر;
        }
        إذا (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32 */
            إذا (ch >= UNI_SUR_لوړ_پیل && ch <= UNI_SUR_ټیټ_پای_ټکی) {
                إذا (flags == strictConversion) {
                    سرچینه_2 -= (extra_بایټونه_منزل_Read+1); /* return to the illegal value itself */
                    پایله_2 = سرچینه_Illegal;
                    اكسر;
                } وإلا {
                    *موخه++ = UNI_REPLACEMENT_CHAR;
                }
            } وإلا {
                *موخه++ = (UTF16)ch; /* normal case */
            }
        } وإلا إذا (ch > UNI_MAX_UTF16) {
            إذا (flags == strictConversion) {
                پایله_2 = سرچینه_Illegal;
                سرچینه_2 -= (extra_بایټونه_منزل_Read+1); /* return to the start */
                اكسر; /* Bail out; shouldn't continue */
            } وإلا {
                *موخه++ = UNI_REPLACEMENT_CHAR;
            }
        } وإلا {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            إذا (موخه + 1 >= موخه_پای_ټکی) {
                سرچینه_2 -= (extra_بایټونه_منزل_Read+1); /* Back up source pointer! */
                پایله_2 = موخه_Exhausted; اكسر;
            }
            ch -= half_بنسټ;
            *موخه++ = (UTF16)((ch >> half_ښويدنه) + UNI_SUR_لوړ_پیل);
            *موخه++ = (UTF16)((ch & half_ماسک) + UNI_SUR_ټیټ_پیل);
        }
    }
    *سرچینه_پیل = سرچینه_2;
    *موخه_پیل = موخه;
    أرجع پایله_2;
}

/* --------------------------------------------------------------------- */

ساكن Conversion_پایله ConvertUTF8toUTF32Impl(
        ثابت UTF8** سرچینه_پیل, ثابت UTF8* سرچینه_پای_ټکی,
        UTF32** موخه_پیل, UTF32* موخه_پای_ټکی, ConversionFlags flags,
        Boolean ننوت_Is_Partial) {
    Conversion_پایله پایله_2 = conversionOK;
    ثابت UTF8* سرچینه_2 = *سرچینه_پیل;
    UTF32* موخه = *موخه_پیل;
    طالما (سرچینه_2 < سرچینه_پای_ټکی) {
        UTF32 ch = 0;
        بینښه لنډ extra_بایټونه_منزل_Read = trailing_بایټونه_For_UTF8[*سرچینه_2];
        إذا (extra_بایټونه_منزل_Read >= سرچینه_پای_ټکی - سرچینه_2) {
            إذا (flags == strictConversion || ننوت_Is_Partial) {
                پایله_2 = سرچینه_Exhausted;
                اكسر;
            } وإلا {
                پایله_2 = سرچینه_Illegal;

                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                سرچینه_2 += پیدا_کړه_Maximal_Subpart_د_Ill_Formed_UTF8_Sequence(سرچینه_2,
                                                                    سرچینه_پای_ټکی);
                *موخه++ = UNI_REPLACEMENT_CHAR;
                استمر;
            }
        }
        إذا (موخه >= موخه_پای_ټکی) {
            پایله_2 = موخه_Exhausted; اكسر;
        }

        /* Do this check whether lenient or strict */
        إذا (!isLegalUTF8(سرچینه_2, extra_بایټونه_منزل_Read+1)) {
            پایله_2 = سرچینه_Illegal;
            إذا (flags == strictConversion) {
                /* Abort conversion. */
                اكسر;
            } وإلا {
                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                سرچینه_2 += پیدا_کړه_Maximal_Subpart_د_Ill_Formed_UTF8_Sequence(سرچینه_2,
                                                                    سرچینه_پای_ټکی);
                *موخه++ = UNI_REPLACEMENT_CHAR;
                استمر;
            }
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        بدل (extra_بایټونه_منزل_Read) {
            حالة 5: ch += *سرچینه_2++; ch <<= 6;
            حالة 4: ch += *سرچینه_2++; ch <<= 6;
            حالة 3: ch += *سرچینه_2++; ch <<= 6;
            حالة 2: ch += *سرچینه_2++; ch <<= 6;
            حالة 1: ch += *سرچینه_2++; ch <<= 6;
            حالة 0: ch += *سرچینه_2++;
        }
        ch -= offsetsFromUTF8[extra_بایټونه_منزل_Read];

        إذا (ch <= UNI_MAX_LEGAL_UTF32) {
            /*
             * UTF-16 surrogate values are illegal in UTF-32, and anything
             * over Plane 17 (> 0x10FFFF) is illegal.
             */
            إذا (ch >= UNI_SUR_لوړ_پیل && ch <= UNI_SUR_ټیټ_پای_ټکی) {
                إذا (flags == strictConversion) {
                    سرچینه_2 -= (extra_بایټونه_منزل_Read+1); /* return to the illegal value itself */
                    پایله_2 = سرچینه_Illegal;
                    اكسر;
                } وإلا {
                    *موخه++ = UNI_REPLACEMENT_CHAR;
                }
            } وإلا {
                *موخه++ = ch;
            }
        } وإلا { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
            پایله_2 = سرچینه_Illegal;
            *موخه++ = UNI_REPLACEMENT_CHAR;
        }
    }
    *سرچینه_پیل = سرچینه_2;
    *موخه_پیل = موخه;
    أرجع پایله_2;
}

Conversion_پایله ConvertUTF8toUTF32Partial(ثابت UTF8 **سرچینه_پیل,
                                           ثابت UTF8 *سرچینه_پای_ټکی,
                                           UTF32 **موخه_پیل,
                                           UTF32 *موخه_پای_ټکی,
                                           ConversionFlags flags) {
  أرجع ConvertUTF8toUTF32Impl(سرچینه_پیل, سرچینه_پای_ټکی, موخه_پیل, موخه_پای_ټکی,
                                flags, /*InputIsPartial=*/ریښتیا);
}

Conversion_پایله ConvertUTF8toUTF32(ثابت UTF8 **سرچینه_پیل,
                                    ثابت UTF8 *سرچینه_پای_ټکی, UTF32 **موخه_پیل,
                                    UTF32 *موخه_پای_ټکی, ConversionFlags flags) {
  أرجع ConvertUTF8toUTF32Impl(سرچینه_پیل, سرچینه_پای_ټکی, موخه_پیل, موخه_پای_ټکی,
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
