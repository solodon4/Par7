//
//  utf-8-reader.h
//  
//
//  Created by Yuriy Solodkyy on 2/17/15.
//
//

#pragma once

namespace unicode
{
    typedef char32_t code_point;
    typedef char code_unit;

    // Based on http://en.wikipedia.org/wiki/UTF-8#Sample_code
    void utf8decode(const code_unit* s, size_t src_size, code_point* dst, size_t& dst_size)
    {
        const unsigned char* src = reinterpret_cast<const unsigned char*>(s);

        while (src_size && dst_size)
        {
            if (src[0] < 0x80)
            {
                *dst++ = src[0];
                src += 1; // Skip 1 read code unit
                src_size -= 1;
                dst_size--;
            }
            else
                if (src[0] < 0xC2)
                {
                    /* continuation or overlong 2-byte sequence */
                    goto ERROR1;
                }
                else
                    if (src[0] < 0xE0)
                    {
                        /* 2-byte sequence */
                        if ((src[1] & 0xC0) != 0x80) goto ERROR2;

                        *dst++ = (src[0] << 6) + src[1] - 0x3080;
                        src += 2; // Skip 2 read code units
                        src_size -= 2;
                        dst_size--;
                    }
                    else
                        if (src[0] < 0xF0)
                        {
                            /* 3-byte sequence */
                            if ((src[1] & 0xC0) != 0x80) goto ERROR2;
                            if (src[0] == 0xE0 && src[1] < 0xA0) goto ERROR2; /* overlong */
                            if ((src[2] & 0xC0) != 0x80) goto ERROR3;
                            *dst++ = (src[0] << 12) + (src[1] << 6) + src[2] - 0xE2080;
                            src += 3; // Skip 3 read code units
                            src_size -= 3;
                            dst_size--;
                        } else
                            if (src[0] < 0xF5)
                            {
                                /* 4-byte sequence */
                                if ((src[1] & 0xC0) != 0x80) goto ERROR2;
                                if (src[0] == 0xF0 && src[1] <  0x90) goto ERROR2; /* overlong */
                                if (src[0] == 0xF4 && src[1] >= 0x90) goto ERROR2; /* > U+10FFFF */
                                if ((src[2] & 0xC0) != 0x80) goto ERROR3;
                                if ((src[3] & 0xC0) != 0x80) goto ERROR4;
                                *dst++ = (src[0] << 18) + (src[1] << 12) + (src[2] << 6) + src[3] - 0x3C82080;
                                src += 4; // Skip 4 read code units
                                src_size -= 4;
                                dst_size--;
                            }
                            else
                            {
                                /* > U+10FFFF */
                                goto ERROR1;
                            }
            
        ERROR4: ;
            //ungetc(src[3], stdin);
        ERROR3: ;
            //ungetc(src[2], stdin);
        ERROR2: ;
            //ungetc(src[1], stdin);
        ERROR1: ;
            //return src[0] + 0xDC00;
        }
    }
} // of namespace unicode
