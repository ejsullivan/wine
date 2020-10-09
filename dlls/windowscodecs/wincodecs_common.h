/*
 * Copyright 2020 Esme Povirk
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

HRESULT CDECL decoder_initialize(struct decoder *decoder, IStream *stream, struct decoder_stat *st)
{
    return decoder->vtable->initialize(decoder, stream, st);
}

HRESULT CDECL decoder_get_frame_info(struct decoder *decoder, UINT frame, struct decoder_frame *info)
{
    return decoder->vtable->get_frame_info(decoder, frame, info);
}

HRESULT CDECL decoder_copy_pixels(struct decoder *decoder, UINT frame,
    const WICRect *prc, UINT stride, UINT buffersize, BYTE *buffer)
{
    return decoder->vtable->copy_pixels(decoder, frame, prc, stride, buffersize, buffer);
}

void CDECL decoder_destroy(struct decoder *decoder)
{
    decoder->vtable->destroy(decoder);
}

HRESULT copy_pixels(UINT bpp, const BYTE *srcbuffer,
    UINT srcwidth, UINT srcheight, INT srcstride,
    const WICRect *rc, UINT dststride, UINT dstbuffersize, BYTE *dstbuffer)
{
    UINT bytesperrow;
    UINT row_offset; /* number of bits into the source rows where the data starts */
    WICRect rect;

    if (!rc)
    {
        rect.X = 0;
        rect.Y = 0;
        rect.Width = srcwidth;
        rect.Height = srcheight;
        rc = &rect;
    }
    else
    {
        if (rc->X < 0 || rc->Y < 0 || rc->X+rc->Width > srcwidth || rc->Y+rc->Height > srcheight)
            return E_INVALIDARG;
    }

    bytesperrow = ((bpp * rc->Width)+7)/8;

    if (dststride < bytesperrow)
        return E_INVALIDARG;

    if ((dststride * (rc->Height-1)) + bytesperrow > dstbuffersize)
        return E_INVALIDARG;

    /* if the whole bitmap is copied and the buffer format matches then it's a matter of a single memcpy */
    if (rc->X == 0 && rc->Y == 0 && rc->Width == srcwidth && rc->Height == srcheight &&
        srcstride == dststride && srcstride == bytesperrow)
    {
        memcpy(dstbuffer, srcbuffer, srcstride * srcheight);
        return S_OK;
    }

    row_offset = rc->X * bpp;

    if (row_offset % 8 == 0)
    {
        /* everything lines up on a byte boundary */
        INT row;
        const BYTE *src;
        BYTE *dst;

        src = srcbuffer + (row_offset / 8) + srcstride * rc->Y;
        dst = dstbuffer;
        for (row=0; row < rc->Height; row++)
        {
            memcpy(dst, src, bytesperrow);
            src += srcstride;
            dst += dststride;
        }
        return S_OK;
    }
    else
    {
        /* we have to do a weird bitwise copy. eww. */
        FIXME("cannot reliably copy bitmap data if bpp < 8\n");
        return E_FAIL;
    }
}
