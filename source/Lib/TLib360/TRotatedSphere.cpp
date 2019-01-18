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

/** \file     TCubeMap.cpp
    \brief    CubeMap class
*/

#include <assert.h>
#include <math.h>
#include "TRotatedSphere.h"

#if EXTENSION_360_VIDEO
#if SVIDEO_ROTATED_SPHERE

static const Double S_PI_4 = 0.785398163397448;

enum
{
    RSP_FRONT_FACE = 0,
    RSP_BACK_FACE = 1,
    RSP_TOP_FACE = 2,
    RSP_BOTTOM_FACE = 3,
    RSP_RIGHT_FACE = 4,
    RSP_LEFT_FACE = 5,
    
    RSP_FACE_COUNT = 6,
};

/*************************************
Cubemap geometry related functions;
**************************************/
TRotatedSphere::TRotatedSphere(SVideoInfo& sVideoInfo, InputGeoParam *pInGeoParam) : TGeometry()
{
   assert(sVideoInfo.geoType == SVIDEO_ROTATEDSPHERE);
   assert(sVideoInfo.iNumFaces == 6);
   geoInit(sVideoInfo, pInGeoParam); 
}

TRotatedSphere::~TRotatedSphere()
{
}

static Int faceMargin(Int FaceIndex)
{
    switch( FaceIndex )
    {
        case RSP_FRONT_FACE:
        case RSP_BACK_FACE:
            return 3;
            
        case RSP_TOP_FACE:
        case RSP_LEFT_FACE:
            return 5;
            
        case RSP_RIGHT_FACE:
        case RSP_BOTTOM_FACE:
            return 1;
    }
    
    assert(0);
    return 0;
}

Void TRotatedSphere::map2DTo3D(SPos& IPosIn, SPos *pSPosOut)
{
    Int FaceSize = m_sVideoInfo.iFaceWidth;
    
    POSType u = IPosIn.x + (POSType)(0.5);
    POSType v = IPosIn.y + (POSType)(0.5);
        
    pSPosOut->faceIdx =IPosIn.faceIdx;
    
    POSType yaw = (POSType)((u + faceMargin(IPosIn.faceIdx) * FaceSize / 2)*S_PI_2 / FaceSize - S_PI);
    POSType pitch = (POSType)(S_PI_2 - (v + FaceSize / 2)*S_PI / FaceSize / 2);
    
    pSPosOut->x = (POSType)(scos(pitch)*scos(yaw));
    pSPosOut->y = (POSType)(ssin(pitch));
    pSPosOut->z = -(POSType)(scos(pitch)*ssin(yaw));
    
    if( IPosIn.faceIdx == RSP_BACK_FACE || IPosIn.faceIdx == RSP_TOP_FACE || IPosIn.faceIdx == RSP_BOTTOM_FACE )
    {
        pSPosOut->x = -pSPosOut->x;
        POSType _y = pSPosOut->y;
        pSPosOut->y = -pSPosOut->z;
        pSPosOut->z = -_y;
    }
}

static inline POSType get2d_x( POSType yaw, Int face_size, Int face_index)
{
    return (S_PI + yaw)*face_size / S_PI_2 - faceMargin(face_index) * face_size / 2 - 0.5;
}

static inline POSType get2d_y( POSType pitch, Int face_size )
{
    return (S_PI_2 - pitch) * face_size / S_PI_2 - face_size / 2 - 0.5;
}

static inline POSType get3d_distance( SPos a, SPos b)
{
    return ( (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y) + (a.z-b.z)*(a.z-b.z) );
}

Void TRotatedSphere::map3DTo2D(SPos *pSPosIn, SPos *pSPosOut)
{
    POSType aX = sfabs(pSPosIn->x);
    POSType aY = sfabs(pSPosIn->y);
    POSType aZ = sfabs(pSPosIn->z);
#if (SVIDEO_FIX_TICKET54 && (SVIDEO_RSP_FULL_IMAGE == 0)) || (!SVIDEO_FIX_TICKET54 && SVIDEO_RSP_GUARD_BAND)
    POSType DistanceThreshold = 0.600505655929;
#endif
    Int faceIdx;    
    Int face_size = m_sVideoInfo.iFaceHeight;
    
    POSType org_yaw, org_pitch, rot_yaw, rot_pitch;
    
    {
        POSType x = pSPosIn->x;
        POSType y = pSPosIn->y;
        POSType z = pSPosIn->z;
        
        POSType len = ssqrt(x*x + y*y + z*z);
        
        org_yaw = (POSType)(satan2(-z, x));
        org_pitch = (POSType)(sasin(y / len));
        
        // rot_yaw = satan2( len * ssin(org_pitch), -x );
        // rot_yaw = satan2( stan(org_pitch), -scos(org_yaw) );
        rot_yaw = (POSType)(satan2(y, -x));
        
        // rot_pitch = sasin( x * stan(org_yaw) / len );
        // rot_pitch = sasin( scos(org_pitch) * ssin(org_yaw) );
        rot_pitch = (POSType)(sasin(-z / len));
    }
    
    if(aX >= aY && aX >= aZ)
    {
        if(pSPosIn->x > 0)
            faceIdx = RSP_FRONT_FACE;
        else
            faceIdx = RSP_BACK_FACE;
    }
    else if(aY >= aX && aY >= aZ)
    {
        if(pSPosIn->y > 0)
            faceIdx = RSP_TOP_FACE;
        else
            faceIdx = RSP_BOTTOM_FACE;

#if (SVIDEO_FIX_TICKET54 && (SVIDEO_RSP_FULL_IMAGE == 0)) || (!SVIDEO_FIX_TICKET54 && SVIDEO_RSP_GUARD_BAND)
        SPos TopMostPoint(0, 0, faceIdx == RSP_TOP_FACE ? 1 : -1, 0);
        if( pSPosIn->x > 0 && get3d_distance( *pSPosIn, TopMostPoint ) > DistanceThreshold )
        {
            if( org_yaw > -S_PI_4 && org_yaw < S_PI_4 )
                faceIdx = RSP_FRONT_FACE;
            else if( org_yaw < -S_PI_4 )
                faceIdx = RSP_RIGHT_FACE;
            else if( org_yaw > S_PI_4 )
                faceIdx = RSP_LEFT_FACE;
            else
                assert(0);
        }
#endif
    }
    else
    {
        if(pSPosIn->z > 0)
            faceIdx = RSP_RIGHT_FACE;
        else
            faceIdx = RSP_LEFT_FACE;
     
#if (SVIDEO_FIX_TICKET54 && (SVIDEO_RSP_FULL_IMAGE == 0)) || (!SVIDEO_FIX_TICKET54 && SVIDEO_RSP_GUARD_BAND)
        SPos RightMostPoint(0, 0, 0, faceIdx == RSP_RIGHT_FACE ? 1 : -1);
        if( pSPosIn->x < 0 && get3d_distance( *pSPosIn, RightMostPoint ) > DistanceThreshold )
        {
            if( org_pitch < 0 )
            {
                if( rot_yaw < -S_PI_4 )
                    faceIdx = RSP_BOTTOM_FACE;
                else
                    faceIdx = RSP_BACK_FACE;
            }
            else if( org_pitch > 0 )
            {
                if( rot_yaw > S_PI_4 )
                    faceIdx = RSP_TOP_FACE;
                else
                    faceIdx = RSP_BACK_FACE;
            }
            else
                assert(0);
        }
#endif
    }
    
    Bool back_face = faceIdx == RSP_BACK_FACE || faceIdx == RSP_TOP_FACE || faceIdx == RSP_BOTTOM_FACE;
    
    pSPosOut->faceIdx = faceIdx;
    pSPosOut->x       = get2d_x( back_face ? rot_yaw  : org_yaw,     face_size, faceIdx);
    pSPosOut->y       = get2d_y( back_face ? rot_pitch: org_pitch,  face_size);
    pSPosOut->z       = 0;
}

Void TRotatedSphere::sPad(Pel *pSrc0, Int iHStep0, Int iStrideSrc0, Pel* pSrc1, Int iHStep1, Int iStrideSrc1, Int iNumSamples, Int hCnt, Int vCnt)
{
  Pel *pSrc0Start = pSrc0 + iHStep0;
  Pel *pSrc1Start = pSrc1 - iHStep1;

  for(Int j=0; j<vCnt; j++)
  {
    for(Int i=0; i<hCnt; i++)
    {
      memcpy(pSrc0Start+i*iHStep0, pSrc1+i*iHStep1, iNumSamples*sizeof(Pel));
      memcpy(pSrc1Start-i*iHStep1, pSrc0-i*iHStep0, iNumSamples*sizeof(Pel));
    }
    pSrc0 += iStrideSrc0;
    pSrc0Start += iStrideSrc0;
    pSrc1 += iStrideSrc1;
    pSrc1Start += iStrideSrc1;
  }
}

//90 anti clockwise: source -> destination;
Void TRotatedSphere::rot90(Pel *pSrcBuf, Int iStrideSrc, Int iWidth, Int iHeight, Int iNumSamples, Pel *pDst, Int iStrideDst)
{
    Pel *pSrcCol = pSrcBuf + (iWidth-1)*iNumSamples;
    for(Int j=0; j<iWidth; j++)
    {
      Pel *pSrc = pSrcCol;
      for(Int i=0; i<iHeight; i++, pSrc+= iStrideSrc)
      {
        memcpy(pDst+i*iNumSamples,  pSrc, iNumSamples*sizeof(Pel));
      }
      pDst += iStrideDst;
      pSrcCol -= iNumSamples;
    }
} 

//corner;
Void TRotatedSphere::cPad(Pel *pSrc, Int iWidth, Int iHeight, Int iStrideSrc, Int iNumSamples, Int hCnt, Int vCnt)
{
  //top-left;
  rot90(pSrc-hCnt*iStrideSrc, iStrideSrc, vCnt, hCnt, iNumSamples, pSrc-vCnt*iStrideSrc-hCnt*iNumSamples, iStrideSrc); 
  //bottom-left;
  rot90(pSrc+(iHeight-1-hCnt)*iStrideSrc, iStrideSrc, vCnt, hCnt, iNumSamples, pSrc+iHeight*iStrideSrc-hCnt*iNumSamples, iStrideSrc); 
  //bottom-right;
  rot90(pSrc+iHeight*iStrideSrc+(iWidth-vCnt)*iNumSamples, iStrideSrc, vCnt, hCnt, iNumSamples, pSrc+iHeight*iStrideSrc+iWidth*iNumSamples, iStrideSrc); 
  //top-right;
  rot90(pSrc+iWidth*iNumSamples, iStrideSrc, vCnt, hCnt, iNumSamples, pSrc-vCnt*iStrideSrc+iWidth*iNumSamples, iStrideSrc); 
}

Void TRotatedSphere::ConvertFace(Int ch, Int faceIdx, TComPicYuv *pSrcYuv)
{
    ComponentID chId = ComponentID(ch);
    
    Int nWidth = m_sVideoInfo.iFaceWidth >> pSrcYuv->getComponentScaleX(chId);
    Int nHeight = m_sVideoInfo.iFaceHeight >> pSrcYuv->getComponentScaleY(chId);
    
    Int faceX = m_facePos[faceIdx][1]*nWidth;
    Int faceY = m_facePos[faceIdx][0]*nHeight;
    assert(faceIdx == m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].id);
    Int iRot = m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].rot;
    Int iStrideSrc = pSrcYuv->getStride((ComponentID)(ch));
    Pel *pSrc = pSrcYuv->getAddr((ComponentID)ch) + faceY*iStrideSrc + faceX;
    Pel *pDst = m_pFacesOrig[faceIdx][ch];
    rotFaceChannelGeneral(pSrc, nWidth, nHeight, pSrcYuv->getStride((ComponentID)ch), 1, iRot, pDst, getStride((ComponentID)ch), 1, true);
}

Void TRotatedSphere::convertYuv(TComPicYuv *pSrcYuv)
{
  Int nWidth = m_sVideoInfo.iFaceWidth;
  Int nHeight = m_sVideoInfo.iFaceHeight;

  assert(pSrcYuv->getWidth(ComponentID(0)) == nWidth*m_sVideoInfo.framePackStruct.cols && pSrcYuv->getHeight(ComponentID(0)) == nHeight*m_sVideoInfo.framePackStruct.rows);
  assert(pSrcYuv->getNumberValidComponents() == getNumChannels());

  if(pSrcYuv->getChromaFormat()==CHROMA_420)
  {
    Int nFaces = m_sVideoInfo.iNumFaces;

    for(Int ch=0; ch<pSrcYuv->getNumberValidComponents(); ch++)
    {
      ComponentID chId = ComponentID(ch);

      if(!ch || (m_chromaFormatIDC==CHROMA_420 && !m_bResampleChroma))
      {
          ConvertFace( ch, RSP_TOP_FACE, pSrcYuv );
          ConvertFace( ch, RSP_BOTTOM_FACE, pSrcYuv );
          ConvertFace( ch, RSP_LEFT_FACE, pSrcYuv );
          ConvertFace( ch, RSP_RIGHT_FACE, pSrcYuv );
          ConvertFace( ch, RSP_FRONT_FACE, pSrcYuv );
          ConvertFace( ch, RSP_BACK_FACE, pSrcYuv );
          
        continue;
      }

      //memory allocation;
      if(!m_pFacesBufTemp)
      {
        assert(!m_pFacesBufTempOrig);
        m_nMarginSizeBufTemp = std::max(m_filterUps[2].nTaps, m_filterUps[3].nTaps)>>1;;  //depends on the vertical upsampling filter;
        m_nStrideBufTemp = nWidth + (m_nMarginSizeBufTemp<<1);
        m_pFacesBufTemp = new Pel*[nFaces];
        memset(m_pFacesBufTemp, 0, sizeof(Pel*)*nFaces);
        m_pFacesBufTempOrig = new Pel*[nFaces];
        memset(m_pFacesBufTempOrig, 0, sizeof(Pel*)*nFaces);
        Int iTotalHeight = (nHeight +(m_nMarginSizeBufTemp<<1));
        for(Int i=0; i<nFaces; i++)
        {
          m_pFacesBufTemp[i] = (Pel *)xMalloc(Pel,  m_nStrideBufTemp*iTotalHeight);
          m_pFacesBufTempOrig[i] = m_pFacesBufTemp[i] +  m_nStrideBufTemp * m_nMarginSizeBufTemp + m_nMarginSizeBufTemp;
        }
      }
      //read content first;
      for(Int faceIdx=0; faceIdx<nFaces; faceIdx++)
      {
        Int faceX = m_facePos[faceIdx][1]*nWidth;
        Int faceY = m_facePos[faceIdx][0]*nHeight;
        assert(faceIdx == m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].id);
        Int iRot = m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].rot;

        Int iStrideSrc = pSrcYuv->getStride((ComponentID)(ch));
        Pel *pSrc = pSrcYuv->getAddr((ComponentID)ch) + faceY*iStrideSrc + faceX;
        Pel *pDst = m_pFacesBufTempOrig[faceIdx];
        rotFaceChannelGeneral(pSrc, nWidth, nHeight, pSrcYuv->getStride((ComponentID)ch), 1, iRot, pDst, m_nStrideBufTemp, 1, true);
      }

      //padding;
      {
        Int iFaceStride = m_nStrideBufTemp;
        //edges parallel with Y axis;
        sPad(m_pFacesBufTempOrig[0]+(nWidth-1), 1, iFaceStride, m_pFacesBufTempOrig[5], 1, iFaceStride, 1, m_nMarginSizeBufTemp, nHeight); 
        sPad(m_pFacesBufTempOrig[5]+(nWidth-1), 1, iFaceStride, m_pFacesBufTempOrig[1], 1, iFaceStride, 1, m_nMarginSizeBufTemp, nHeight); 
        sPad(m_pFacesBufTempOrig[1]+(nWidth-1), 1, iFaceStride, m_pFacesBufTempOrig[4], 1, iFaceStride, 1, m_nMarginSizeBufTemp, nHeight); 
        sPad(m_pFacesBufTempOrig[4]+(nWidth-1), 1, iFaceStride, m_pFacesBufTempOrig[0], 1, iFaceStride, 1, m_nMarginSizeBufTemp, nHeight); 
    
        //edges parallel with Z axis;
        sPad(m_pFacesBufTempOrig[0], -iFaceStride, 1, m_pFacesBufTempOrig[2]+(nHeight-1)*iFaceStride+(nWidth-1), -1, -iFaceStride, 1, m_nMarginSizeBufTemp, nWidth); 
        sPad(m_pFacesBufTempOrig[2], -1, iFaceStride, m_pFacesBufTempOrig[1], iFaceStride, 1, 1, m_nMarginSizeBufTemp, nHeight);
        sPad(m_pFacesBufTempOrig[1]+(nHeight-1)*iFaceStride+(nWidth-1), iFaceStride, -1, m_pFacesBufTempOrig[3], 1, iFaceStride, 1, m_nMarginSizeBufTemp, nWidth);
        sPad(m_pFacesBufTempOrig[3]+(nWidth-1), 1, iFaceStride, m_pFacesBufTempOrig[0]+(nHeight-1)*iFaceStride, -iFaceStride, 1, 1, m_nMarginSizeBufTemp, nHeight);

        //edges parallel with X axis;
        sPad(m_pFacesBufTempOrig[2]+(nHeight-1)*iFaceStride, iFaceStride, 1, m_pFacesBufTempOrig[4], iFaceStride, 1, 1, m_nMarginSizeBufTemp, nHeight);
        sPad(m_pFacesBufTempOrig[4]+(nHeight-1)*iFaceStride, iFaceStride, 1, m_pFacesBufTempOrig[3], iFaceStride, 1, 1, m_nMarginSizeBufTemp, nWidth);
        sPad(m_pFacesBufTempOrig[3]+(nHeight-1)*iFaceStride, iFaceStride, 1, m_pFacesBufTempOrig[5]+(nHeight-1)*iFaceStride+(nWidth-1), -iFaceStride, -1, 1, m_nMarginSizeBufTemp, nWidth);
        sPad(m_pFacesBufTempOrig[5], -iFaceStride, 1, m_pFacesBufTempOrig[2]+(nWidth-1), iFaceStride, -1, 1, m_nMarginSizeBufTemp, nWidth);

        //corner region padding;
        for(Int f=0; f<nFaces; f++)
          cPad(m_pFacesBufTempOrig[f], nWidth, nHeight, iFaceStride, 1, m_nMarginSizeBufTemp, m_nMarginSizeBufTemp); 
      }

      if(m_chromaFormatIDC == CHROMA_420)
      {
        //convert chroma_sample_loc from 0 to 2;
        for(Int f=0; f<nFaces; f++)
          chromaResampleType0toType2(m_pFacesBufTempOrig[f], nWidth, nHeight, m_nStrideBufTemp, m_pFacesOrig[f][ch], getStride(chId));
      }
      else
      {
        //420->444;
        for(Int f=0; f<nFaces; f++)
          chromaUpsample(m_pFacesBufTempOrig[f], nWidth, nHeight, m_nStrideBufTemp, f, chId);
      }
    }
  }
  else if(pSrcYuv->getChromaFormat()==CHROMA_400 || pSrcYuv->getChromaFormat()==CHROMA_444)
  {  
     if(m_chromaFormatIDC == pSrcYuv->getChromaFormat())
     {
      for(Int faceIdx=0; faceIdx<m_sVideoInfo.iNumFaces; faceIdx++)
      {
        Int faceX = m_facePos[faceIdx][1]*nWidth;
        Int faceY = m_facePos[faceIdx][0]*nHeight;
        assert(faceIdx == m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].id);
        Int iRot = m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].rot;

        for(Int ch=0; ch<getNumChannels(); ch++)
        {
          Int iStrideSrc = pSrcYuv->getStride((ComponentID)(ch));
          Pel *pSrc = pSrcYuv->getAddr((ComponentID)ch) + faceY*iStrideSrc + faceX;
          Pel *pDst = m_pFacesOrig[faceIdx][ch];
          rotFaceChannelGeneral(pSrc, nWidth, nHeight, pSrcYuv->getStride((ComponentID)ch), 1, iRot, pDst, getStride((ComponentID)ch), 1, true);
        }
      }
     }
     else
       assert(!"Not supported yet!");
  }
  else
    assert(!"Not supported yet!");
 
  //set padding flag;
  setPaddingFlag(false);
}

static Void sPadH(Pel *pSrc, Pel *pDst, Int iCount, Int iVCnt, Int iStride)
{
    for (Int j = 0; j < iVCnt; j++)
    {
        for (Int i = 1; i <= iCount; i++)
        {
            pDst[i - 1] = pSrc[i - 1];
            pSrc[-i] = pDst[-i];
        }
        pSrc += iStride;
        pDst += iStride;
    }
}

Void TRotatedSphere::spherePadding(Bool bEnforced)
{
    if (!bEnforced && m_bPadded)
    {
        return;
    }
    m_bPadded = false;
    
#if 0
    //dump to file;
    static Bool bFirstDumpBeforePading = true;
    dumpAllFacesToFile("equirect_before_padding", false, !bFirstDumpBeforePading);
    bFirstDumpBeforePading = false;
#endif
    
    TGeometry::spherePadding(bEnforced);
    
    Int nFaces = m_sVideoInfo.iNumFaces;
    
    for (Int ch = 0; ch<(getNumChannels()); ch++)
    {
        ComponentID chId = (ComponentID)ch;
        Int nWidth = m_sVideoInfo.iFaceWidth >> getComponentScaleX(chId);
        Int nHeight = m_sVideoInfo.iFaceHeight >> getComponentScaleY(chId);
        Int nMarginX = m_iMarginX >> getComponentScaleX(chId);
        Int iStride = getStride(ComponentID(ch));
        
        //Equatorial area, face 2,3,4,5
        Pel **pSrc = new Pel*[nFaces];
        Pel **pDst = new Pel*[nFaces];
        for (Int faceIdx = 0; faceIdx<nFaces; faceIdx++)
        {
            pSrc[faceIdx] = m_pFacesOrig[faceIdx][ch];
            pDst[faceIdx] = pSrc[faceIdx] + nWidth;
        }
        
        sPadH(pSrc[RSP_FRONT_FACE], pDst[RSP_RIGHT_FACE], nMarginX, nHeight, iStride);
        sPadH(pSrc[RSP_LEFT_FACE], pDst[RSP_FRONT_FACE], nMarginX, nHeight, iStride);
        
        sPadH(pSrc[RSP_BACK_FACE],  pDst[RSP_BOTTOM_FACE], nMarginX, nHeight, iStride);
        sPadH(pSrc[RSP_TOP_FACE],  pDst[RSP_BACK_FACE], nMarginX, nHeight, iStride);
        
        delete[] pSrc;
        delete[] pDst;
    }
    m_bPadded = true;
    
#if 0
    //dump to file;
    static Bool bFirstDumpAfterPading = true;
    dumpAllFacesToFile("equirect_after_padding", true, !bFirstDumpAfterPading);
    bFirstDumpAfterPading = false;
#endif
}

#if SVIDEO_FIX_TICKET54
#if (SVIDEO_RSP_FULL_IMAGE == 0)
Bool TRotatedSphere::insideFace(Int fId, Int x, Int y, ComponentID chId, ComponentID origchId)
{
    assert(m_sVideoInfo.iFaceHeight == m_sVideoInfo.iFaceWidth && "RSP Face shall be square.");
    
    if ( y < 0 || y >= (m_sVideoInfo.iFaceHeight >> getComponentScaleY(chId)) ||
        x < 0 || x >= (m_sVideoInfo.iFaceWidth  >> getComponentScaleX(chId)) )
        return false;
    
    Int radius = (m_sVideoInfo.iFaceHeight >> 1);
    
    Double x_L = fabs(radius - ( x << getComponentScaleX(chId)) - 0.5);
    Double y_L = fabs(radius - ( y << getComponentScaleY(chId)) - 0.5);
    
    const Double factor = 2.0;
    
    Double d = pow( pow( x_L, factor ) + pow(y_L, factor), Double(1)/Double(factor) );
    
    Int margin = 2 * ( (m_sVideoInfo.iFaceHeight / 90) + 1 );
    
    if( ( x < radius && ( fId == RSP_RIGHT_FACE || fId == RSP_BOTTOM_FACE ) ) || ( x > radius && ( fId == RSP_LEFT_FACE || fId == RSP_TOP_FACE ) ) )
    {
        if (m_chromaFormatIDC == CHROMA_444 || origchId == COMPONENT_Y)
        {
            return d <= radius + margin;
        }
        else  // chroma in 420
        {
            return d/2 <= (radius + margin)>>1;
        }
    }
    
    return true;
    
}
#endif // (SVIDEO_RSP_FULL_IMAGE == 0)

#else
#if SVIDEO_RSP_GUARD_BAND

Bool TRotatedSphere::insideFace(Int fId, Int x, Int y, ComponentID chId, ComponentID origchId)
{
    assert(m_sVideoInfo.iFaceHeight == m_sVideoInfo.iFaceWidth && "RSP Face shall be square.");

    if ( y < 0 || y >= (m_sVideoInfo.iFaceHeight >> getComponentScaleY(chId)) ||
         x < 0 || x >= (m_sVideoInfo.iFaceWidth  >> getComponentScaleX(chId)) )
        return false;
    
    Int radius = (m_sVideoInfo.iFaceHeight >> 1);
    
    Double x_L = radius - (x << getComponentScaleX(chId)) - 0.5;
    Double y_L = radius - (y << getComponentScaleY(chId)) - 0.5;
    Double d = ssqrt(x_L*x_L + y_L*y_L);
    
    if( x < radius && ( fId == RSP_RIGHT_FACE || fId == RSP_BOTTOM_FACE ) )
    {
        if (m_chromaFormatIDC == CHROMA_444 || origchId == COMPONENT_Y)
        {
            return d <= radius + SVIDEO_RSP_GUARD_BAND;
        }
        else  // chroma in 420
        {
            return d/2 <= (radius + SVIDEO_RSP_GUARD_BAND)>>1;
        }
    }
    
    if( x > radius && ( fId == RSP_LEFT_FACE || fId == RSP_TOP_FACE ) )
    {
        if (m_chromaFormatIDC == CHROMA_444 || origchId == COMPONENT_Y)
        {
            return d <= radius + SVIDEO_RSP_GUARD_BAND;
        }
        else  // chroma in 420
        {
            return d/2 <= (radius + SVIDEO_RSP_GUARD_BAND)>>1;
        }
    }
    
    return true;
    
}
#endif // SVIDEO_RSP_GUARD_BAND
#endif
#endif
#endif
