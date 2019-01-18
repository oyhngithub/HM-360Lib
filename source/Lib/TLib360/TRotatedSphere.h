/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2015, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TRotatedSphere.h
    \brief    TRotatedSphere class (header)
*/

#ifndef __TROTATEDSPHERE__
#define __TROTATEDSPHERE__
#include "TGeometry.h"

// ====================================================================================================================
// Class definition
// ====================================================================================================================

#if EXTENSION_360_VIDEO
#if SVIDEO_ROTATED_SPHERE
#if SVIDEO_FIX_TICKET54
#define SVIDEO_RSP_FULL_IMAGE  0          // 0: minimum pixels are active, 1: all pixels are active
#endif
class TRotatedSphere : public TGeometry
{
private:

  Void sPad(Pel *pSrc0, Int iHStep0, Int iStrideSrc0, Pel* pSrc1, Int iHStep1, Int iStrideSrc1, Int iNumSamples, Int hCnt, Int vCnt);
  Void cPad(Pel *pSrc0, Int iWidth, Int iHeight, Int iStrideSrc0, Int iNumSamples, Int hCnt, Int vCnt);
  Void rot90(Pel *pSrc, Int iStrideSrc, Int iWidth, Int iHeight, Int iNumSamples, Pel *pDst, Int iStrideDst);
  
    Void ConvertFace(Int ch, Int faceIdx, TComPicYuv *pSrcYuv);
    
  //
public:
  TRotatedSphere(SVideoInfo& sVideoInfo, InputGeoParam *pInGeoParam);
  virtual ~TRotatedSphere();

  virtual Void map2DTo3D(SPos& IPosIn, SPos *pSPosOut); 
  virtual Void map3DTo2D(SPos *pSPosIn, SPos *pSPosOut); 
  virtual Void convertYuv(TComPicYuv *pSrcYuv);
  
  Void spherePadding(Bool bEnforced);
#if (SVIDEO_FIX_TICKET54 && (SVIDEO_RSP_FULL_IMAGE == 0)) || (!SVIDEO_FIX_TICKET54 && SVIDEO_RSP_GUARD_BAND)
    virtual Bool insideFace(Int fId, Int x, Int y, ComponentID chId, ComponentID origchId);
#endif
};
#endif
#endif
#endif // __TROTATEDSPHERE__

