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

#అన్తర్భావ "llvm/Support/ConvertUTF.h"
#నిర్వచితమైతే CVTUTF_DEBUG
#అన్తర్భావ <stdio.h>
#అయితే_ముగింపు
#అన్తర్భావ <assert.h>

/*
 * This code extensively uses fall-through switches.
 * Keep the compiler from warning about that.
 */
#ఒకవేళ defined(__clang__) && defined(__has_warning)
# ఒకవేళ __has_warning("-Wimplicit-fallthrough")
#  define ConvertUTF_DISABLE_WARNINGS \
    _Pragma("clang diagnostic push")  \
    _Pragma("clang diagnostic ignored \"-Wimplicit-fallthrough\"")
#  define ConvertUTF_RESTORE_WARNINGS \
    _Pragma("clang diagnostic pop")
# endif
#లేదావేళ defined(__GNUC__)
# define ConvertUTF_DISABLE_WARNINGS \
   _Pragma("GCC diagnostic push")    \
   _Pragma("GCC diagnostic ignored \"-Wimplicit-fallthrough\"")
# define ConvertUTF_RESTORE_WARNINGS \
   _Pragma("GCC diagnostic pop")
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ConvertUTF_DISABLE_WARNINGS
# define ConvertUTF_DISABLE_WARNINGS
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ConvertUTF_RESTORE_WARNINGS
# define ConvertUTF_RESTORE_WARNINGS
#అయితే_ముగింపు

ConvertUTF_DISABLE_WARNINGS

నామాకాశ llvm {

స్థిర స్థిరరాశి పూర్ణాంకం half_జరుపు  = 10; /* used for shifting by 10 bits */

స్థిర స్థిరరాశి UTF32 half_పునాది = 0x0010000UL;
స్థిర స్థిరరాశి UTF32 half_ముసుగు = 0x3FFUL;

#నిర్వచించు UNI_SUR_ఎగువ_ప్రారంభం  (UTF32)0xD800
#నిర్వచించు UNI_SUR_ఎగువ_చివర    (UTF32)0xDBFF
#నిర్వచించు UNI_SUR_దిగువ_ప్రారంభం   (UTF32)0xDC00
#నిర్వచించు UNI_SUR_దిగువ_చివర     (UTF32)0xDFFF

/* --------------------------------------------------------------------- */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
స్థిర స్థిరరాశి అక్షరం trailing_బైట్లు_For_UTF8[256] = {
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
స్థిర స్థిరరాశి UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
                     0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
స్థిర స్థిరరాశి UTF8 మొదటి_Byte_Mark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */


/* --------------------------------------------------------------------- */

Conversion_ఫలితం ConvertUTF32toUTF16 (
        స్థిరరాశి UTF32** ఆధారం_ప్రారంభం, స్థిరరాశి UTF32* ఆధారం_చివర,
        UTF16** లక్ష్యం_ప్రారంభం, UTF16* లక్ష్యం_చివర, ConversionFlags flags) {
    Conversion_ఫలితం ఫలితం_2 = conversionOK;
    స్థిరరాశి UTF32* ఆధారం_2 = *ఆధారం_ప్రారంభం;
    UTF16* లక్ష్యం = *లక్ష్యం_ప్రారంభం;
    అయితే (ఆధారం_2 < ఆధారం_చివర) {
        UTF32 ch;
        ఒకవేళ (లక్ష్యం >= లక్ష్యం_చివర) {
            ఫలితం_2 = లక్ష్యం_Exhausted; ఆపు;
        }
        ch = *ఆధారం_2++;
        ఒకవేళ (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
            ఒకవేళ (ch >= UNI_SUR_ఎగువ_ప్రారంభం && ch <= UNI_SUR_దిగువ_చివర) {
                ఒకవేళ (flags == strictConversion) {
                    --ఆధారం_2; /* return to the illegal value itself */
                    ఫలితం_2 = ఆధారం_Illegal;
                    ఆపు;
                } లేకపోతే {
                    *లక్ష్యం++ = UNI_REPLACEMENT_CHAR;
                }
            } లేకపోతే {
                *లక్ష్యం++ = (UTF16)ch; /* normal case */
            }
        } లేకపోతే ఒకవేళ (ch > UNI_MAX_LEGAL_UTF32) {
            ఒకవేళ (flags == strictConversion) {
                ఫలితం_2 = ఆధారం_Illegal;
            } లేకపోతే {
                *లక్ష్యం++ = UNI_REPLACEMENT_CHAR;
            }
        } లేకపోతే {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            ఒకవేళ (లక్ష్యం + 1 >= లక్ష్యం_చివర) {
                --ఆధారం_2; /* Back up source pointer! */
                ఫలితం_2 = లక్ష్యం_Exhausted; ఆపు;
            }
            ch -= half_పునాది;
            *లక్ష్యం++ = (UTF16)((ch >> half_జరుపు) + UNI_SUR_ఎగువ_ప్రారంభం);
            *లక్ష్యం++ = (UTF16)((ch & half_ముసుగు) + UNI_SUR_దిగువ_ప్రారంభం);
        }
    }
    *ఆధారం_ప్రారంభం = ఆధారం_2;
    *లక్ష్యం_ప్రారంభం = లక్ష్యం;
    తిరిగి ఫలితం_2;
}

/* --------------------------------------------------------------------- */

Conversion_ఫలితం ConvertUTF16toUTF32 (
        స్థిరరాశి UTF16** ఆధారం_ప్రారంభం, స్థిరరాశి UTF16* ఆధారం_చివర,
        UTF32** లక్ష్యం_ప్రారంభం, UTF32* లక్ష్యం_చివర, ConversionFlags flags) {
    Conversion_ఫలితం ఫలితం_2 = conversionOK;
    స్థిరరాశి UTF16* ఆధారం_2 = *ఆధారం_ప్రారంభం;
    UTF32* లక్ష్యం = *లక్ష్యం_ప్రారంభం;
    UTF32 ch, ch2;
    అయితే (ఆధారం_2 < ఆధారం_చివర) {
        స్థిరరాశి UTF16* old_ఆధారం = ఆధారం_2; /*  In case we have to back up because of target overflow. */
        ch = *ఆధారం_2++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        ఒకవేళ (ch >= UNI_SUR_ఎగువ_ప్రారంభం && ch <= UNI_SUR_ఎగువ_చివర) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            ఒకవేళ (ఆధారం_2 < ఆధారం_చివర) {
                ch2 = *ఆధారం_2;
                /* If it's a low surrogate, convert to UTF32. */
                ఒకవేళ (ch2 >= UNI_SUR_దిగువ_ప్రారంభం && ch2 <= UNI_SUR_దిగువ_చివర) {
                    ch = ((ch - UNI_SUR_ఎగువ_ప్రారంభం) << half_జరుపు)
                        + (ch2 - UNI_SUR_దిగువ_ప్రారంభం) + half_పునాది;
                    ++ఆధారం_2;
                } లేకపోతే ఒకవేళ (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --ఆధారం_2; /* return to the illegal value itself */
                    ఫలితం_2 = ఆధారం_Illegal;
                    ఆపు;
                }
            } లేకపోతే { /* We don't have the 16 bits following the high surrogate. */
                --ఆధారం_2; /* return to the high surrogate */
                ఫలితం_2 = ఆధారం_Exhausted;
                ఆపు;
            }
        } లేకపోతే ఒకవేళ (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            ఒకవేళ (ch >= UNI_SUR_దిగువ_ప్రారంభం && ch <= UNI_SUR_దిగువ_చివర) {
                --ఆధారం_2; /* return to the illegal value itself */
                ఫలితం_2 = ఆధారం_Illegal;
                ఆపు;
            }
        }
        ఒకవేళ (లక్ష్యం >= లక్ష్యం_చివర) {
            ఆధారం_2 = old_ఆధారం; /* Back up source pointer! */
            ఫలితం_2 = లక్ష్యం_Exhausted; ఆపు;
        }
        *లక్ష్యం++ = ch;
    }
    *ఆధారం_ప్రారంభం = ఆధారం_2;
    *లక్ష్యం_ప్రారంభం = లక్ష్యం;
#నిర్వచితమైతే CVTUTF_DEBUG
ఒకవేళ (ఫలితం_2 == ఆధారం_Illegal) {
    fprintf(stderr, "ConvertUTF16toUTF32 illegal seq 0x%04x,%04x\n", ch, ch2);
    fflush(stderr);
}
#అయితే_ముగింపు
    తిరిగి ఫలితం_2;
}
Conversion_ఫలితం ConvertUTF16toUTF8 (
        స్థిరరాశి UTF16** ఆధారం_ప్రారంభం, స్థిరరాశి UTF16* ఆధారం_చివర,
        UTF8** లక్ష్యం_ప్రారంభం, UTF8* లక్ష్యం_చివర, ConversionFlags flags) {
    Conversion_ఫలితం ఫలితం_2 = conversionOK;
    స్థిరరాశి UTF16* ఆధారం_2 = *ఆధారం_ప్రారంభం;
    UTF8* లక్ష్యం = *లక్ష్యం_ప్రారంభం;
    అయితే (ఆధారం_2 < ఆధారం_చివర) {
        UTF32 ch;
        గుర్తులేని పొట్టి బైట్లు_గమ్యం_Write = 0;
        స్థిరరాశి UTF32 byte_ముసుగు = 0xBF;
        స్థిరరాశి UTF32 byteMark = 0x80;
        స్థిరరాశి UTF16* old_ఆధారం = ఆధారం_2; /* In case we have to back up because of target overflow. */
        ch = *ఆధారం_2++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        ఒకవేళ (ch >= UNI_SUR_ఎగువ_ప్రారంభం && ch <= UNI_SUR_ఎగువ_చివర) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            ఒకవేళ (ఆధారం_2 < ఆధారం_చివర) {
                UTF32 ch2 = *ఆధారం_2;
                /* If it's a low surrogate, convert to UTF32. */
                ఒకవేళ (ch2 >= UNI_SUR_దిగువ_ప్రారంభం && ch2 <= UNI_SUR_దిగువ_చివర) {
                    ch = ((ch - UNI_SUR_ఎగువ_ప్రారంభం) << half_జరుపు)
                        + (ch2 - UNI_SUR_దిగువ_ప్రారంభం) + half_పునాది;
                    ++ఆధారం_2;
                } లేకపోతే ఒకవేళ (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --ఆధారం_2; /* return to the illegal value itself */
                    ఫలితం_2 = ఆధారం_Illegal;
                    ఆపు;
                }
            } లేకపోతే { /* We don't have the 16 bits following the high surrogate. */
                --ఆధారం_2; /* return to the high surrogate */
                ఫలితం_2 = ఆధారం_Exhausted;
                ఆపు;
            }
        } లేకపోతే ఒకవేళ (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            ఒకవేళ (ch >= UNI_SUR_దిగువ_ప్రారంభం && ch <= UNI_SUR_దిగువ_చివర) {
                --ఆధారం_2; /* return to the illegal value itself */
                ఫలితం_2 = ఆధారం_Illegal;
                ఆపు;
            }
        }
        /* Figure out how many bytes the result will require */
        ఒకవేళ (ch < (UTF32)0x80) {      బైట్లు_గమ్యం_Write = 1;
        } లేకపోతే ఒకవేళ (ch < (UTF32)0x800) {     బైట్లు_గమ్యం_Write = 2;
        } లేకపోతే ఒకవేళ (ch < (UTF32)0x10000) {   బైట్లు_గమ్యం_Write = 3;
        } లేకపోతే ఒకవేళ (ch < (UTF32)0x110000) {  బైట్లు_గమ్యం_Write = 4;
        } లేకపోతే {                            బైట్లు_గమ్యం_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
        }

        లక్ష్యం += బైట్లు_గమ్యం_Write;
        ఒకవేళ (లక్ష్యం > లక్ష్యం_చివర) {
            ఆధారం_2 = old_ఆధారం; /* Back up source pointer! */
            లక్ష్యం -= బైట్లు_గమ్యం_Write; ఫలితం_2 = లక్ష్యం_Exhausted; ఆపు;
        }
        ఎంపిక (బైట్లు_గమ్యం_Write) { /* note: everything falls through. */
            స్థితి 4: *--లక్ష్యం = (UTF8)((ch | byteMark) & byte_ముసుగు); ch >>= 6;
            స్థితి 3: *--లక్ష్యం = (UTF8)((ch | byteMark) & byte_ముసుగు); ch >>= 6;
            స్థితి 2: *--లక్ష్యం = (UTF8)((ch | byteMark) & byte_ముసుగు); ch >>= 6;
            స్థితి 1: *--లక్ష్యం =  (UTF8)(ch | మొదటి_Byte_Mark[బైట్లు_గమ్యం_Write]);
        }
        లక్ష్యం += బైట్లు_గమ్యం_Write;
    }
    *ఆధారం_ప్రారంభం = ఆధారం_2;
    *లక్ష్యం_ప్రారంభం = లక్ష్యం;
    తిరిగి ఫలితం_2;
}

/* --------------------------------------------------------------------- */

Conversion_ఫలితం ConvertUTF32toUTF8 (
        స్థిరరాశి UTF32** ఆధారం_ప్రారంభం, స్థిరరాశి UTF32* ఆధారం_చివర,
        UTF8** లక్ష్యం_ప్రారంభం, UTF8* లక్ష్యం_చివర, ConversionFlags flags) {
    Conversion_ఫలితం ఫలితం_2 = conversionOK;
    స్థిరరాశి UTF32* ఆధారం_2 = *ఆధారం_ప్రారంభం;
    UTF8* లక్ష్యం = *లక్ష్యం_ప్రారంభం;
    అయితే (ఆధారం_2 < ఆధారం_చివర) {
        UTF32 ch;
        గుర్తులేని పొట్టి బైట్లు_గమ్యం_Write = 0;
        స్థిరరాశి UTF32 byte_ముసుగు = 0xBF;
        స్థిరరాశి UTF32 byteMark = 0x80;
        ch = *ఆధారం_2++;
        ఒకవేళ (flags == strictConversion ) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            ఒకవేళ (ch >= UNI_SUR_ఎగువ_ప్రారంభం && ch <= UNI_SUR_దిగువ_చివర) {
                --ఆధారం_2; /* return to the illegal value itself */
                ఫలితం_2 = ఆధారం_Illegal;
                ఆపు;
            }
        }
        /*
         * Figure out how many bytes the result will require. Turn any
         * illegally large UTF32 things (> Plane 17) into replacement chars.
         */
        ఒకవేళ (ch < (UTF32)0x80) {      బైట్లు_గమ్యం_Write = 1;
        } లేకపోతే ఒకవేళ (ch < (UTF32)0x800) {     బైట్లు_గమ్యం_Write = 2;
        } లేకపోతే ఒకవేళ (ch < (UTF32)0x10000) {   బైట్లు_గమ్యం_Write = 3;
        } లేకపోతే ఒకవేళ (ch <= UNI_MAX_LEGAL_UTF32) {  బైట్లు_గమ్యం_Write = 4;
        } లేకపోతే {                            బైట్లు_గమ్యం_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
                                            ఫలితం_2 = ఆధారం_Illegal;
        }

        లక్ష్యం += బైట్లు_గమ్యం_Write;
        ఒకవేళ (లక్ష్యం > లక్ష్యం_చివర) {
            --ఆధారం_2; /* Back up source pointer! */
            లక్ష్యం -= బైట్లు_గమ్యం_Write; ఫలితం_2 = లక్ష్యం_Exhausted; ఆపు;
        }
        ఎంపిక (బైట్లు_గమ్యం_Write) { /* note: everything falls through. */
            స్థితి 4: *--లక్ష్యం = (UTF8)((ch | byteMark) & byte_ముసుగు); ch >>= 6;
            స్థితి 3: *--లక్ష్యం = (UTF8)((ch | byteMark) & byte_ముసుగు); ch >>= 6;
            స్థితి 2: *--లక్ష్యం = (UTF8)((ch | byteMark) & byte_ముసుగు); ch >>= 6;
            స్థితి 1: *--లక్ష్యం = (UTF8) (ch | మొదటి_Byte_Mark[బైట్లు_గమ్యం_Write]);
        }
        లక్ష్యం += బైట్లు_గమ్యం_Write;
    }
    *ఆధారం_ప్రారంభం = ఆధారం_2;
    *లక్ష్యం_ప్రారంభం = లక్ష్యం;
    తిరిగి ఫలితం_2;
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

స్థిర Boolean isLegalUTF8(స్థిరరాశి UTF8 *ఆధారం_2, పూర్ణాంకం నిడివి_2) {
    UTF8 a;
    స్థిరరాశి UTF8 *srcptr = ఆధారం_2+నిడివి_2;
    ఎంపిక (నిడివి_2) {
    అప్రమేయ: తిరిగి అబద్ధం;
        /* Everything else falls through when "true"... */
    స్థితి 4: ఒకవేళ ((a = (*--srcptr)) < 0x80 || a > 0xBF) తిరిగి అబద్ధం;
    స్థితి 3: ఒకవేళ ((a = (*--srcptr)) < 0x80 || a > 0xBF) తిరిగి అబద్ధం;
    స్థితి 2: ఒకవేళ ((a = (*--srcptr)) < 0x80 || a > 0xBF) తిరిగి అబద్ధం;

        ఎంపిక (*ఆధారం_2) {
            /* no fall-through in this inner switch */
            స్థితి 0xE0: ఒకవేళ (a < 0xA0) తిరిగి అబద్ధం; ఆపు;
            స్థితి 0xED: ఒకవేళ (a > 0x9F) తిరిగి అబద్ధం; ఆపు;
            స్థితి 0xF0: ఒకవేళ (a < 0x90) తిరిగి అబద్ధం; ఆపు;
            స్థితి 0xF4: ఒకవేళ (a > 0x8F) తిరిగి అబద్ధం; ఆపు;
            అప్రమేయ:   ఒకవేళ (a < 0x80) తిరిగి అబద్ధం;
        }

    స్థితి 1: ఒకవేళ (*ఆధారం_2 >= 0x80 && *ఆధారం_2 < 0xC2) తిరిగి అబద్ధం;
    }
    ఒకవేళ (*ఆధారం_2 > 0xF4) తిరిగి అబద్ధం;
    తిరిగి నిజం;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 sequence is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8Sequence(స్థిరరాశి UTF8 *ఆధారం_2, స్థిరరాశి UTF8 *ఆధారం_చివర) {
    పూర్ణాంకం నిడివి_2 = trailing_బైట్లు_For_UTF8[*ఆధారం_2]+1;
    ఒకవేళ (నిడివి_2 > ఆధారం_చివర - ఆధారం_2) {
        తిరిగి అబద్ధం;
    }
    తిరిగి isLegalUTF8(ఆధారం_2, నిడివి_2);
}

/*
 * Exported function to return the size of the first utf-8 code unit sequence,
 * Or 0 if the sequence is not valid;
 */
గుర్తులేని get_UTF8_Sequence_కొలత(స్థిరరాశి UTF8 *ఆధారం_2, స్థిరరాశి UTF8 *ఆధారం_చివర) {
  పూర్ణాంకం నిడివి_2 = trailing_బైట్లు_For_UTF8[*ఆధారం_2] + 1;
  తిరిగి (నిడివి_2 <= ఆధారం_చివర - ఆధారం_2 && isLegalUTF8(ఆధారం_2, నిడివి_2)) ? నిడివి_2
                                                                       : 0;
}

/* --------------------------------------------------------------------- */

గుర్తులేని కనుగొను_Maximal_Subpart_యొక్క_Ill_Formed_UTF8_Sequence(స్థిరరాశి UTF8 *ఆధారం_2,
                                                   స్థిరరాశి UTF8 *ఆధారం_చివర) {
  UTF8 b1, b2, b3;

  ప్రతిజ్ఞా(!isLegalUTF8Sequence(ఆధారం_2, ఆధారం_చివర));

  /*
   * Unicode 6.3.0, D93b:
   *
   *   Maximal subpart of an ill-formed subsequence: The longest code unit
   *   subsequence starting at an unconvertible offset that is either:
   *   a. the initial subsequence of a well-formed code unit sequence, or
   *   b. a subsequence of length one.
   */

  ఒకవేళ (ఆధారం_2 == ఆధారం_చివర)
    తిరిగి 0;

  /*
   * Perform case analysis.  See Unicode 6.3.0, Table 3-7. Well-Formed UTF-8
   * Byte Sequences.
   */

  b1 = *ఆధారం_2;
  ++ఆధారం_2;
  ఒకవేళ (b1 >= 0xC2 && b1 <= 0xDF) {
    /*
     * First byte is valid, but we know that this code unit sequence is
     * invalid, so the maximal subpart has to end after the first byte.
     */
    తిరిగి 1;
  }

  ఒకవేళ (ఆధారం_2 == ఆధారం_చివర)
    తిరిగి 1;

  b2 = *ఆధారం_2;
  ++ఆధారం_2;

  ఒకవేళ (b1 == 0xE0) {
    తిరిగి (b2 >= 0xA0 && b2 <= 0xBF) ? 2 : 1;
  }
  ఒకవేళ (b1 >= 0xE1 && b1 <= 0xEC) {
    తిరిగి (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  ఒకవేళ (b1 == 0xED) {
    తిరిగి (b2 >= 0x80 && b2 <= 0x9F) ? 2 : 1;
  }
  ఒకవేళ (b1 >= 0xEE && b1 <= 0xEF) {
    తిరిగి (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  ఒకవేళ (b1 == 0xF0) {
    ఒకవేళ (b2 >= 0x90 && b2 <= 0xBF) {
      ఒకవేళ (ఆధారం_2 == ఆధారం_చివర)
        తిరిగి 2;

      b3 = *ఆధారం_2;
      తిరిగి (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    తిరిగి 1;
  }
  ఒకవేళ (b1 >= 0xF1 && b1 <= 0xF3) {
    ఒకవేళ (b2 >= 0x80 && b2 <= 0xBF) {
      ఒకవేళ (ఆధారం_2 == ఆధారం_చివర)
        తిరిగి 2;

      b3 = *ఆధారం_2;
      తిరిగి (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    తిరిగి 1;
  }
  ఒకవేళ (b1 == 0xF4) {
    ఒకవేళ (b2 >= 0x80 && b2 <= 0x8F) {
      ఒకవేళ (ఆధారం_2 == ఆధారం_చివర)
        తిరిగి 2;

      b3 = *ఆధారం_2;
      తిరిగి (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    తిరిగి 1;
  }

  ప్రతిజ్ఞా((b1 >= 0x80 && b1 <= 0xC1) || b1 >= 0xF5);
  /*
   * There are no valid sequences that start with these bytes.  Maximal subpart
   * is defined to have length 1 in these cases.
   */
  తిరిగి 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return the total number of bytes in a codepoint
 * represented in UTF-8, given the value of the first byte.
 */
గుర్తులేని get_అంకె_బైట్లు_For_UTF8(UTF8 మొదటి_2) {
  తిరిగి trailing_బైట్లు_For_UTF8[మొదటి_2] + 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 string is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8String(స్థిరరాశి UTF8 **ఆధారం_2, స్థిరరాశి UTF8 *ఆధారం_చివర) {
    అయితే (*ఆధారం_2 != ఆధారం_చివర) {
        పూర్ణాంకం నిడివి_2 = trailing_బైట్లు_For_UTF8[**ఆధారం_2] + 1;
        ఒకవేళ (నిడివి_2 > ఆధారం_చివర - *ఆధారం_2 || !isLegalUTF8(*ఆధారం_2, నిడివి_2))
            తిరిగి అబద్ధం;
        *ఆధారం_2 += నిడివి_2;
    }
    తిరిగి నిజం;
}

/* --------------------------------------------------------------------- */

Conversion_ఫలితం ConvertUTF8toUTF16 (
        స్థిరరాశి UTF8** ఆధారం_ప్రారంభం, స్థిరరాశి UTF8* ఆధారం_చివర,
        UTF16** లక్ష్యం_ప్రారంభం, UTF16* లక్ష్యం_చివర, ConversionFlags flags) {
    Conversion_ఫలితం ఫలితం_2 = conversionOK;
    స్థిరరాశి UTF8* ఆధారం_2 = *ఆధారం_ప్రారంభం;
    UTF16* లక్ష్యం = *లక్ష్యం_ప్రారంభం;
    అయితే (ఆధారం_2 < ఆధారం_చివర) {
        UTF32 ch = 0;
        గుర్తులేని పొట్టి extra_బైట్లు_గమ్యం_Read = trailing_బైట్లు_For_UTF8[*ఆధారం_2];
        ఒకవేళ (extra_బైట్లు_గమ్యం_Read >= ఆధారం_చివర - ఆధారం_2) {
            ఫలితం_2 = ఆధారం_Exhausted; ఆపు;
        }
        /* Do this check whether lenient or strict */
        ఒకవేళ (!isLegalUTF8(ఆధారం_2, extra_బైట్లు_గమ్యం_Read+1)) {
            ఫలితం_2 = ఆధారం_Illegal;
            ఆపు;
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        ఎంపిక (extra_బైట్లు_గమ్యం_Read) {
            స్థితి 5: ch += *ఆధారం_2++; ch <<= 6; /* remember, illegal UTF-8 */
            స్థితి 4: ch += *ఆధారం_2++; ch <<= 6; /* remember, illegal UTF-8 */
            స్థితి 3: ch += *ఆధారం_2++; ch <<= 6;
            స్థితి 2: ch += *ఆధారం_2++; ch <<= 6;
            స్థితి 1: ch += *ఆధారం_2++; ch <<= 6;
            స్థితి 0: ch += *ఆధారం_2++;
        }
        ch -= offsetsFromUTF8[extra_బైట్లు_గమ్యం_Read];

        ఒకవేళ (లక్ష్యం >= లక్ష్యం_చివర) {
            ఆధారం_2 -= (extra_బైట్లు_గమ్యం_Read+1); /* Back up source pointer! */
            ఫలితం_2 = లక్ష్యం_Exhausted; ఆపు;
        }
        ఒకవేళ (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32 */
            ఒకవేళ (ch >= UNI_SUR_ఎగువ_ప్రారంభం && ch <= UNI_SUR_దిగువ_చివర) {
                ఒకవేళ (flags == strictConversion) {
                    ఆధారం_2 -= (extra_బైట్లు_గమ్యం_Read+1); /* return to the illegal value itself */
                    ఫలితం_2 = ఆధారం_Illegal;
                    ఆపు;
                } లేకపోతే {
                    *లక్ష్యం++ = UNI_REPLACEMENT_CHAR;
                }
            } లేకపోతే {
                *లక్ష్యం++ = (UTF16)ch; /* normal case */
            }
        } లేకపోతే ఒకవేళ (ch > UNI_MAX_UTF16) {
            ఒకవేళ (flags == strictConversion) {
                ఫలితం_2 = ఆధారం_Illegal;
                ఆధారం_2 -= (extra_బైట్లు_గమ్యం_Read+1); /* return to the start */
                ఆపు; /* Bail out; shouldn't continue */
            } లేకపోతే {
                *లక్ష్యం++ = UNI_REPLACEMENT_CHAR;
            }
        } లేకపోతే {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            ఒకవేళ (లక్ష్యం + 1 >= లక్ష్యం_చివర) {
                ఆధారం_2 -= (extra_బైట్లు_గమ్యం_Read+1); /* Back up source pointer! */
                ఫలితం_2 = లక్ష్యం_Exhausted; ఆపు;
            }
            ch -= half_పునాది;
            *లక్ష్యం++ = (UTF16)((ch >> half_జరుపు) + UNI_SUR_ఎగువ_ప్రారంభం);
            *లక్ష్యం++ = (UTF16)((ch & half_ముసుగు) + UNI_SUR_దిగువ_ప్రారంభం);
        }
    }
    *ఆధారం_ప్రారంభం = ఆధారం_2;
    *లక్ష్యం_ప్రారంభం = లక్ష్యం;
    తిరిగి ఫలితం_2;
}

/* --------------------------------------------------------------------- */

స్థిర Conversion_ఫలితం ConvertUTF8toUTF32Impl(
        స్థిరరాశి UTF8** ఆధారం_ప్రారంభం, స్థిరరాశి UTF8* ఆధారం_చివర,
        UTF32** లక్ష్యం_ప్రారంభం, UTF32* లక్ష్యం_చివర, ConversionFlags flags,
        Boolean ఆదానం_Is_Partial) {
    Conversion_ఫలితం ఫలితం_2 = conversionOK;
    స్థిరరాశి UTF8* ఆధారం_2 = *ఆధారం_ప్రారంభం;
    UTF32* లక్ష్యం = *లక్ష్యం_ప్రారంభం;
    అయితే (ఆధారం_2 < ఆధారం_చివర) {
        UTF32 ch = 0;
        గుర్తులేని పొట్టి extra_బైట్లు_గమ్యం_Read = trailing_బైట్లు_For_UTF8[*ఆధారం_2];
        ఒకవేళ (extra_బైట్లు_గమ్యం_Read >= ఆధారం_చివర - ఆధారం_2) {
            ఒకవేళ (flags == strictConversion || ఆదానం_Is_Partial) {
                ఫలితం_2 = ఆధారం_Exhausted;
                ఆపు;
            } లేకపోతే {
                ఫలితం_2 = ఆధారం_Illegal;

                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                ఆధారం_2 += కనుగొను_Maximal_Subpart_యొక్క_Ill_Formed_UTF8_Sequence(ఆధారం_2,
                                                                    ఆధారం_చివర);
                *లక్ష్యం++ = UNI_REPLACEMENT_CHAR;
                కొనసాగు;
            }
        }
        ఒకవేళ (లక్ష్యం >= లక్ష్యం_చివర) {
            ఫలితం_2 = లక్ష్యం_Exhausted; ఆపు;
        }

        /* Do this check whether lenient or strict */
        ఒకవేళ (!isLegalUTF8(ఆధారం_2, extra_బైట్లు_గమ్యం_Read+1)) {
            ఫలితం_2 = ఆధారం_Illegal;
            ఒకవేళ (flags == strictConversion) {
                /* Abort conversion. */
                ఆపు;
            } లేకపోతే {
                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                ఆధారం_2 += కనుగొను_Maximal_Subpart_యొక్క_Ill_Formed_UTF8_Sequence(ఆధారం_2,
                                                                    ఆధారం_చివర);
                *లక్ష్యం++ = UNI_REPLACEMENT_CHAR;
                కొనసాగు;
            }
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        ఎంపిక (extra_బైట్లు_గమ్యం_Read) {
            స్థితి 5: ch += *ఆధారం_2++; ch <<= 6;
            స్థితి 4: ch += *ఆధారం_2++; ch <<= 6;
            స్థితి 3: ch += *ఆధారం_2++; ch <<= 6;
            స్థితి 2: ch += *ఆధారం_2++; ch <<= 6;
            స్థితి 1: ch += *ఆధారం_2++; ch <<= 6;
            స్థితి 0: ch += *ఆధారం_2++;
        }
        ch -= offsetsFromUTF8[extra_బైట్లు_గమ్యం_Read];

        ఒకవేళ (ch <= UNI_MAX_LEGAL_UTF32) {
            /*
             * UTF-16 surrogate values are illegal in UTF-32, and anything
             * over Plane 17 (> 0x10FFFF) is illegal.
             */
            ఒకవేళ (ch >= UNI_SUR_ఎగువ_ప్రారంభం && ch <= UNI_SUR_దిగువ_చివర) {
                ఒకవేళ (flags == strictConversion) {
                    ఆధారం_2 -= (extra_బైట్లు_గమ్యం_Read+1); /* return to the illegal value itself */
                    ఫలితం_2 = ఆధారం_Illegal;
                    ఆపు;
                } లేకపోతే {
                    *లక్ష్యం++ = UNI_REPLACEMENT_CHAR;
                }
            } లేకపోతే {
                *లక్ష్యం++ = ch;
            }
        } లేకపోతే { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
            ఫలితం_2 = ఆధారం_Illegal;
            *లక్ష్యం++ = UNI_REPLACEMENT_CHAR;
        }
    }
    *ఆధారం_ప్రారంభం = ఆధారం_2;
    *లక్ష్యం_ప్రారంభం = లక్ష్యం;
    తిరిగి ఫలితం_2;
}

Conversion_ఫలితం ConvertUTF8toUTF32Partial(స్థిరరాశి UTF8 **ఆధారం_ప్రారంభం,
                                           స్థిరరాశి UTF8 *ఆధారం_చివర,
                                           UTF32 **లక్ష్యం_ప్రారంభం,
                                           UTF32 *లక్ష్యం_చివర,
                                           ConversionFlags flags) {
  తిరిగి ConvertUTF8toUTF32Impl(ఆధారం_ప్రారంభం, ఆధారం_చివర, లక్ష్యం_ప్రారంభం, లక్ష్యం_చివర,
                                flags, /*InputIsPartial=*/నిజం);
}

Conversion_ఫలితం ConvertUTF8toUTF32(స్థిరరాశి UTF8 **ఆధారం_ప్రారంభం,
                                    స్థిరరాశి UTF8 *ఆధారం_చివర, UTF32 **లక్ష్యం_ప్రారంభం,
                                    UTF32 *లక్ష్యం_చివర, ConversionFlags flags) {
  తిరిగి ConvertUTF8toUTF32Impl(ఆధారం_ప్రారంభం, ఆధారం_చివర, లక్ష్యం_ప్రారంభం, లక్ష్యం_చివర,
                                flags, /*InputIsPartial=*/అబద్ధం);
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
