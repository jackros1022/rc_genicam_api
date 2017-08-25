/*
 * This file is part of the rc_genicam_api package.
 *
 * Copyright (c) 2017 Roboception GmbH
 * All rights reserved
 *
 * Author: Heiko Hirschmueller
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "image.h"

#include "exception.h"

#include <cstring>

namespace rcg
{

Image::Image(const Buffer *buffer)
{
  if (buffer->getImagePresent())
  {
    timestamp=buffer->getTimestampNS();

    width=buffer->getWidth();
    height=buffer->getHeight();
    xoffset=buffer->getXOffset();
    yoffset=buffer->getYOffset();
    xpadding=buffer->getXPadding();
    ypadding=buffer->getYPadding();
    frameid=buffer->getFrameID();
    pixelformat=buffer->getPixelFormat();
    bigendian=buffer->isBigEndian();

    const size_t size=buffer->getSizeFilled();
    const size_t offset=buffer->getImageOffset();

    pixel.reset(new uint8_t [size-offset]);

    memcpy(pixel.get(), reinterpret_cast<uint8_t *>(buffer->getBase())+offset, size-offset);
  }
  else
  {
    throw GenTLException("Image::Image(): Now image available.");
  }
}

namespace
{

/**
  Clamp the given value to the range of 0 to 255 and cast to byte.
*/

inline unsigned char clamp8(int v)
{
  const int v2=v<0 ? 0:v;
  return static_cast<unsigned char>(v2>255 ? 255:v2);
}

}

void convYCbCr411toRGB(uint8_t rgb[3], const uint8_t *row, int i)
{
  const uint32_t j=(i>>2)*6;
  const uint32_t js=i&0x3;

  int Y=row[j+js];
  if (js > 1)
  {
    Y=row[j+js+1];
  }

  const int Cb=static_cast<int>(row[j+2])-128;
  const int Cr=static_cast<int>(row[j+5])-128;

  const int rc=(90*Cr+32)>>6;
  const int gc=(-22*Cb-46*Cr+32)>>6;
  const int bc=(113*Cb+32)>>6;

  rgb[0]=clamp8(Y+rc);
  rgb[1]=clamp8(Y+gc);
  rgb[2]=clamp8(Y+bc);
}

void convYCbCr411toQuadRGB(uint8_t rgb[12], const uint8_t *row, int i)
{
  i=(i>>2)*6;

  const int Y[4]={row[i], row[i+1], row[i+3], row[i+4]};
  const int Cb=static_cast<int>(row[i+2])-128;
  const int Cr=static_cast<int>(row[i+5])-128;

  const int rc=(90*Cr+32)>>6;
  const int gc=(-22*Cb-46*Cr+32)>>6;
  const int bc=(113*Cb+32)>>6;

  for (int j=0; j<4; j++)
  {
    *rgb++=clamp8(Y[j]+rc);
    *rgb++=clamp8(Y[j]+gc);
    *rgb++=clamp8(Y[j]+bc);
  }
}

}