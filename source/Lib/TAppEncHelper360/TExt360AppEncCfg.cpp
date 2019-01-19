/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2016, ITU/ISO/IEC
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


#include "TAppEncHelper360/TExt360AppEncCfg.h"
#include <math.h>
#include "TLib360/TGeometry.h"
#include "TAppCommon/program_options_lite.h"
#include "../App/TAppEncoder/TAppEncCfg.h"
#include <sstream>

Bool confirmPara(Bool bflag, const TChar* message);


TExt360AppEncCfg::TExt360AppEncCfg(TAppEncCfg &cfg) : m_cfg(cfg)
{
#if SVIDEO_VIEWPORT_PSNR
  m_viewPortPSNRParam.bViewPortPSNREnabled = false;
  m_viewPortPSNRParam.viewPortSettingsList.clear();
#endif
#if SVIDEO_DYNAMIC_VIEWPORT_PSNR
  m_dynamicViewPortPSNRParam.bViewPortPSNREnabled = false;
  m_dynamicViewPortPSNRParam.viewPortSettingsList.clear();
#endif
}

TExt360AppEncCfg::~TExt360AppEncCfg()
{
}

static inline std::istringstream &operator>>(std::istringstream &in, GeometryRotation &rot)
{
#if SVIDEO_ROT_FIX
  Double t;
  in>>t; //yaw;
  rot.degree[2] = TGeometry::round(t*SVIDEO_ROT_PRECISION);
  in>>t; //pitch;
  rot.degree[1] = TGeometry::round(t*SVIDEO_ROT_PRECISION);
  in>>t; //roll
  rot.degree[0] = TGeometry::round(t*SVIDEO_ROT_PRECISION);
#else
  in>>rot.degree[0];
  in>>rot.degree[1];
  in>>rot.degree[2];
#endif
  return in;
}

static inline std::istringstream &operator>>(std::istringstream &in, SVideoFPStruct &sFPStruct)
{
  in>>sFPStruct.rows;
  in>>sFPStruct.cols;
  for ( Int i = 0; i < sFPStruct.rows; i++ )
  {
    for(Int j=0; j<sFPStruct.cols; j++)
    {
      in>>sFPStruct.faces[i][j].id;
      in>>sFPStruct.faces[i][j].rot;
    }
  }
  return in;
}

#if SVIDEO_VIEWPORT_PSNR
static inline std::istringstream &operator>>(std::istringstream &in, std::vector<ViewPortSettings> &vpList)
{
  Int nNum;
  in>>nNum;
  vpList.clear();
  for(Int i=0; i<nNum; i++)
  {
    ViewPortSettings vp;
    in>>vp.hFOV;
    in>>vp.vFOV;
    in>>vp.fYaw;
    in>>vp.fPitch;
    vpList.push_back(vp);
  }
  return in;
}
#endif

#if SVIDEO_DYNAMIC_VIEWPORT_PSNR
static inline std::istringstream &operator>>(std::istringstream &in, std::vector<DynViewPortSettings> &dynVpList)     //input
{
  Int nNum;
  in>>nNum;

  dynVpList.clear();
  for(Int i=0; i<nNum; i++)
  {
    DynViewPortSettings dynvp;
    in>>dynvp.hFOV;
    in>>dynvp.vFOV;
    in>>dynvp.iPOC[0];
    in>>dynvp.fYaw[0];
    in>>dynvp.fPitch[0];
    in>>dynvp.iPOC[1];
    in>>dynvp.fYaw[1];
    in>>dynvp.fPitch[1];
    dynVpList.push_back(dynvp);
  }
  return in;
}
#endif

static inline std::istringstream &operator>>(std::istringstream &in, ViewPortSettings &vp)
{
  in>>vp.hFOV;
  in>>vp.vFOV;
  in>>vp.fYaw;
  in>>vp.fPitch;
  return in;
}

Void TExt360AppEncCfg::xSetDefaultFramePackingParam(SVideoInfo& sVideoInfo)
{
  if(  sVideoInfo.geoType == SVIDEO_EQUIRECT 
#if SVIDEO_ADJUSTED_EQUALAREA
    || sVideoInfo.geoType == SVIDEO_ADJUSTEDEQUALAREA 
#else
    || sVideoInfo.geoType == SVIDEO_EQUALAREA 
#endif
    || sVideoInfo.geoType == SVIDEO_VIEWPORT 
#if SVIDEO_CPPPSNR
    || sVideoInfo.geoType == SVIDEO_CRASTERSPARABOLIC
#endif
    )
  {
    if(sVideoInfo.framePackStruct.rows ==0 || sVideoInfo.framePackStruct.cols ==0)
    {
      SVideoFPStruct &frmPack = sVideoInfo.framePackStruct;
      frmPack.chromaFormatIDC = CHROMA_420;
      frmPack.rows = 1;
      frmPack.cols = 1;
      frmPack.faces[0][0].id = 0; frmPack.faces[0][0].rot = 0;
    }
  }
  else if ((sVideoInfo.geoType == SVIDEO_NEWUNIFORMMAP))
  {
	  if (sVideoInfo.framePackStruct.rows == 0 || sVideoInfo.framePackStruct.cols == 0)
	  {
		  SVideoFPStruct &frmPack = sVideoInfo.framePackStruct;
		  frmPack.chromaFormatIDC = CHROMA_420;
		  frmPack.rows = 1;
		  frmPack.cols = 1;
		  frmPack.faces[0][0].id = 0; frmPack.faces[0][0].rot = 0;
	  }
  }
  else if( (sVideoInfo.geoType == SVIDEO_CUBEMAP) 
#if SVIDEO_ADJUSTED_CUBEMAP
        || (sVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP)
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
        || (sVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL)
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
        || (sVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP)
#endif
          )
  {
    if(sVideoInfo.framePackStruct.rows ==0 || sVideoInfo.framePackStruct.cols ==0)
    {
      //set default frame packing format as CMP 3x2;
      SVideoFPStruct &frmPack = sVideoInfo.framePackStruct;
      frmPack.chromaFormatIDC = CHROMA_420;
      frmPack.rows = 2;
      frmPack.cols = 3;
      frmPack.faces[0][0].id = 4; frmPack.faces[0][0].rot = 0;
      frmPack.faces[0][1].id = 0; frmPack.faces[0][1].rot = 0;
      frmPack.faces[0][2].id = 5; frmPack.faces[0][2].rot = 0;
      frmPack.faces[1][0].id = 3; frmPack.faces[1][0].rot = 180;
      frmPack.faces[1][1].id = 1; frmPack.faces[1][1].rot = 270;
      frmPack.faces[1][2].id = 2; frmPack.faces[1][2].rot = 0;
    }
  }
  else if(sVideoInfo.geoType == SVIDEO_OCTAHEDRON)
  {
    if(sVideoInfo.framePackStruct.rows ==0 || sVideoInfo.framePackStruct.cols ==0)
    {
      //set default frame packing format;
      if (sVideoInfo.iCompactFPStructure == 1)
      {
        SVideoFPStruct &frmPack = sVideoInfo.framePackStruct;
        frmPack.chromaFormatIDC = CHROMA_420;
        frmPack.rows = 4;
        frmPack.cols = 2;
        frmPack.faces[0][0].id = 5; frmPack.faces[0][0].rot = 0;
        frmPack.faces[0][1].id = 0; frmPack.faces[0][1].rot = 0;
        frmPack.faces[0][2].id = 7; frmPack.faces[0][2].rot = 0;
        frmPack.faces[0][3].id = 2; frmPack.faces[0][3].rot = 0;
        frmPack.faces[1][0].id = 4; frmPack.faces[1][0].rot = 180;
        frmPack.faces[1][1].id = 1; frmPack.faces[1][1].rot = 180;
        frmPack.faces[1][2].id = 6; frmPack.faces[1][2].rot = 180;
        frmPack.faces[1][3].id = 3; frmPack.faces[1][3].rot = 180;
      }
      else 
      {
        assert(sVideoInfo.iCompactFPStructure == 0 || sVideoInfo.iCompactFPStructure == 2);
        SVideoFPStruct &frmPack = sVideoInfo.framePackStruct;
        frmPack.chromaFormatIDC = CHROMA_420;
        frmPack.rows = 4;
        frmPack.cols = 2;
        frmPack.faces[0][0].id = 4; frmPack.faces[0][0].rot = 0;
        frmPack.faces[0][1].id = 0; frmPack.faces[0][1].rot = 0;
        frmPack.faces[0][2].id = 6; frmPack.faces[0][2].rot = 0;
        frmPack.faces[0][3].id = 2; frmPack.faces[0][3].rot = 0;
        frmPack.faces[1][0].id = 5; frmPack.faces[1][0].rot = 180;
        frmPack.faces[1][1].id = 1; frmPack.faces[1][1].rot = 180;
        frmPack.faces[1][2].id = 7; frmPack.faces[1][2].rot = 180;
        frmPack.faces[1][3].id = 3; frmPack.faces[1][3].rot = 180;
      }
    }
  }
  else if(sVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
  {
    if(sVideoInfo.framePackStruct.rows ==0 || sVideoInfo.framePackStruct.cols ==0)
    {
#if SVIDEO_SEC_VID_ISP3
      if (sVideoInfo.iCompactFPStructure == 1)
      {
        SVideoFPStruct &frmPack = sVideoInfo.framePackStruct;
        frmPack.chromaFormatIDC = CHROMA_420;
        frmPack.rows = 4;
        frmPack.cols = 5;
        frmPack.faces[0][0].id = 0; frmPack.faces[0][0].rot = 180;
        frmPack.faces[0][1].id = 2; frmPack.faces[0][1].rot = 180;
        frmPack.faces[0][2].id = 4; frmPack.faces[0][2].rot = 0;
        frmPack.faces[0][3].id = 6; frmPack.faces[0][3].rot = 180;
        frmPack.faces[0][4].id = 8; frmPack.faces[0][4].rot = 0;
        frmPack.faces[1][0].id = 1; frmPack.faces[1][0].rot = 180;
        frmPack.faces[1][1].id = 3; frmPack.faces[1][1].rot = 180;
        frmPack.faces[1][2].id = 5; frmPack.faces[1][2].rot = 180;
        frmPack.faces[1][3].id = 7; frmPack.faces[1][3].rot = 180;
        frmPack.faces[1][4].id = 9; frmPack.faces[1][4].rot = 180;
        frmPack.faces[2][0].id = 11; frmPack.faces[1][0].rot = 0;
        frmPack.faces[2][1].id = 13; frmPack.faces[1][1].rot = 0;
        frmPack.faces[2][2].id = 15; frmPack.faces[1][2].rot = 0;
        frmPack.faces[2][3].id = 17; frmPack.faces[1][3].rot = 0;
        frmPack.faces[2][4].id = 19; frmPack.faces[1][4].rot = 0;
        frmPack.faces[3][0].id = 10; frmPack.faces[3][0].rot = 180;
        frmPack.faces[3][1].id = 12; frmPack.faces[3][1].rot = 0;
        frmPack.faces[3][2].id = 14; frmPack.faces[3][2].rot = 180;
        frmPack.faces[3][3].id = 16; frmPack.faces[3][3].rot = 0;
        frmPack.faces[3][4].id = 18; frmPack.faces[3][4].rot = 0;
      }
      else
      {
#endif
        //set default frame packing format;
        SVideoFPStruct &frmPack = sVideoInfo.framePackStruct;
        frmPack.chromaFormatIDC = CHROMA_420;
        frmPack.rows = 4;
        frmPack.cols = 5;
        frmPack.faces[0][0].id = 0; frmPack.faces[0][0].rot = 0;
        frmPack.faces[0][1].id = 2; frmPack.faces[0][1].rot = 0;
        frmPack.faces[0][2].id = 4; frmPack.faces[0][2].rot = 0;
        frmPack.faces[0][3].id = 6; frmPack.faces[0][3].rot = 0;
        frmPack.faces[0][4].id = 8; frmPack.faces[0][4].rot = 0;
        frmPack.faces[1][0].id = 1; frmPack.faces[1][0].rot = 180;
        frmPack.faces[1][1].id = 3; frmPack.faces[1][1].rot = 180;
        frmPack.faces[1][2].id = 5; frmPack.faces[1][2].rot = 180;
        frmPack.faces[1][3].id = 7; frmPack.faces[1][3].rot = 180;
        frmPack.faces[1][4].id = 9; frmPack.faces[1][4].rot = 180;
        frmPack.faces[2][0].id = 11; frmPack.faces[1][0].rot = 0;
        frmPack.faces[2][1].id = 13; frmPack.faces[1][1].rot = 0;
        frmPack.faces[2][2].id = 15; frmPack.faces[1][2].rot = 0;
        frmPack.faces[2][3].id = 17; frmPack.faces[1][3].rot = 0;
        frmPack.faces[2][4].id = 19; frmPack.faces[1][4].rot = 0;
        frmPack.faces[3][0].id = 10; frmPack.faces[3][0].rot = 180;
        frmPack.faces[3][1].id = 12; frmPack.faces[3][1].rot = 180;
        frmPack.faces[3][2].id = 14; frmPack.faces[3][2].rot = 180;
        frmPack.faces[3][3].id = 16; frmPack.faces[3][3].rot = 180;
        frmPack.faces[3][4].id = 18; frmPack.faces[3][4].rot = 180;
#if SVIDEO_SEC_VID_ISP3
      }
#endif
    }
  }
#if SVIDEO_TSP_IMP
  else if(sVideoInfo.geoType == SVIDEO_TSP)
  {
    if(sVideoInfo.framePackStruct.rows ==0 || sVideoInfo.framePackStruct.cols ==0)
    {
      //set default frame packing format
      SVideoFPStruct &frmPack = sVideoInfo.framePackStruct;
      frmPack.chromaFormatIDC = CHROMA_420;
      frmPack.rows = 1;
      frmPack.cols = 2;
      frmPack.faces[0][0].id = 0; frmPack.faces[0][0].rot = 0;
      frmPack.faces[0][1].id = 1; frmPack.faces[0][1].rot = 0;
    }
  }
#endif
#if SVIDEO_SEGMENTED_SPHERE
  else if(sVideoInfo.geoType == SVIDEO_SEGMENTEDSPHERE)
  {
    if(sVideoInfo.framePackStruct.rows ==0 || sVideoInfo.framePackStruct.cols ==0)
    {
      SVideoFPStruct &frmPack = sVideoInfo.framePackStruct;
      frmPack.chromaFormatIDC = CHROMA_420;
#if SVIDEO_SSP_VERT
      frmPack.rows = 6;
      frmPack.cols = 1;
      //sVidInfo.framePackStruct.faces[0][0].id = 0;
      frmPack.faces[0][0].id = 0; frmPack.faces[0][0].rot = 0;
      frmPack.faces[1][0].id = 1; frmPack.faces[1][0].rot = 0;
      frmPack.faces[2][0].id = 2; frmPack.faces[2][0].rot = 270;
      frmPack.faces[3][0].id = 3; frmPack.faces[3][0].rot = 270;
      frmPack.faces[4][0].id = 4; frmPack.faces[4][0].rot = 270;
      frmPack.faces[5][0].id = 5; frmPack.faces[5][0].rot = 270;
#else
      frmPack.rows = 1;
      frmPack.cols = 6;
      //sVidInfo.framePackStruct.faces[0][0].id = 0;
      frmPack.faces[0][0].id = 0; frmPack.faces[0][0].rot = 0;
      frmPack.faces[0][1].id = 1; frmPack.faces[0][1].rot = 0;
      frmPack.faces[0][2].id = 2; frmPack.faces[0][2].rot = 0;
      frmPack.faces[0][3].id = 3; frmPack.faces[0][3].rot = 0;
      frmPack.faces[0][4].id = 4; frmPack.faces[0][4].rot = 0;
      frmPack.faces[0][5].id = 5; frmPack.faces[0][5].rot = 0;
#endif
    }
  }
#endif
#if SVIDEO_ROTATED_SPHERE
  else if(sVideoInfo.geoType == SVIDEO_ROTATEDSPHERE)
  {
      if(sVideoInfo.framePackStruct.rows ==0 || sVideoInfo.framePackStruct.cols ==0)
      {
          //set default frame packing format as CMP 3x2;
          SVideoFPStruct &frmPack = sVideoInfo.framePackStruct;
          frmPack.chromaFormatIDC = CHROMA_420;
          frmPack.rows = 2;
          frmPack.cols = 3;          
          frmPack.faces[0][0].id = 4; frmPack.faces[0][0].rot = 0;
          frmPack.faces[0][1].id = 0; frmPack.faces[0][1].rot = 0;
          frmPack.faces[0][2].id = 5; frmPack.faces[0][2].rot = 0;
          frmPack.faces[1][0].id = 3; frmPack.faces[1][0].rot = 0;
          frmPack.faces[1][1].id = 1; frmPack.faces[1][1].rot = 0;
          frmPack.faces[1][2].id = 2; frmPack.faces[1][2].rot = 0;
      }
  }
#endif
}

TExt360AppEncCfg::TExt360AppEncCfgContext::TExt360AppEncCfgContext()
  : tmpInternalChromaFormat(0), defViewPortLists(), vp()
#if SVIDEO_DYNAMIC_VIEWPORT_PSNR
  , defDynViewPortLists(), dynvp()
#endif
{
}

Void TExt360AppEncCfg::addOptions(df::program_options_lite::Options &opts, TExt360AppEncCfg::TExt360AppEncCfgContext &ctx)
{
  memset(&m_sourceSVideoInfo, 0, sizeof(m_sourceSVideoInfo));
  memset(&m_codingSVideoInfo, 0, sizeof(m_codingSVideoInfo));
  m_inputGeoParam.chromaFormat = CHROMA_444;
  m_inputGeoParam.bResampleChroma = false;
  m_inputGeoParam.nBitDepth = 8;
  m_inputGeoParam.iInterp[0] = SI_LANCZOS3;
  m_inputGeoParam.iInterp[1] = SI_LANCZOS2;
#if SVIDEO_VIEWPORT_PSNR
  ctx.vp.hFOV = ctx.vp.vFOV = 75;
  ctx.vp.fYaw = ctx.vp.fPitch = 0;
  ctx.defViewPortLists.push_back(ctx.vp);
  ctx.vp.fYaw = -90;
  ctx.defViewPortLists.push_back(ctx.vp);
#endif
#if SVIDEO_DYNAMIC_VIEWPORT_PSNR
  ctx.dynvp.hFOV = ctx.dynvp.vFOV = 75;
  ctx.dynvp.iPOC[0] = 0;
  ctx.dynvp.fYaw[0] = -45;
  ctx.dynvp.fPitch[0] = -15;
  ctx.dynvp.iPOC[1] = 299;
  ctx.dynvp.fYaw[1] = 45;
  ctx.dynvp.fPitch[1] = 15;
  ctx.defDynViewPortLists.push_back(ctx.dynvp);
  ctx.dynvp.fYaw[0] = -135;
  ctx.dynvp.fYaw[1] = -45;
  ctx.defDynViewPortLists.push_back(ctx.dynvp);
#endif

  opts.addOptions()
  ("SphereVideo,-360vid",                        m_bSVideo,                           false,                                "Enable 360 video projection conversion")
  ("InputGeometryType",                          m_sourceSVideoInfo.geoType,          0,                                    "The geometry of input 360 video")
  ("SourceFPStructure",                          m_sourceSVideoInfo.framePackStruct,  m_sourceSVideoInfo.framePackStruct,   "Source framepacking structure")
  ("SourceCompactFPStructure",                   m_sourceSVideoInfo.iCompactFPStructure, 1,                             "Compact source framepacking structure; only valid for octahedron and icosahedron projection format")
  ("CodingGeometryType",                         m_codingSVideoInfo.geoType,          0,                                    "The internal geometry for encoding")
  ("CodingFPStructure",                          m_codingSVideoInfo.framePackStruct,  m_codingSVideoInfo.framePackStruct,   "Coding framepacking structure")
  ("CodingCompactFPStructure",                   m_codingSVideoInfo.iCompactFPStructure, 1,                             "Compact coding framepacking structure; only valid for octahedron and icosahedron projection format")
#if SVIDEO_ERP_PADDING
  ("InputPERP",                                  m_sourceSVideoInfo.bPERP,            false,                                "enable PERP input")
  ("CodingPERP",                                 m_codingSVideoInfo.bPERP,            false,                                "enable PERP coding")
#endif
#if SVIDEO_ROT_FIX
  ("SVideoRotation",                             m_codingSVideoInfo.sVideoRotation,   m_codingSVideoInfo.sVideoRotation,    "Rotation in (yaw, pitch, roll)")
#else
  ("SVideoRotation",                             m_codingSVideoInfo.sVideoRotation,   m_codingSVideoInfo.sVideoRotation,    "Rotation along X, Y, Z")
#endif
  ("ViewPortSettings",                           m_codingSVideoInfo.viewPort,  m_codingSVideoInfo.viewPort,          "Viewport settings for static viewport generation")
  ("CodingFaceWidth",                            m_iCodingFaceWidth,                  0,                                    "Face width for coding")
  ("CodingFaceHeight",                           m_iCodingFaceHeight,                 0,                                    "Face height for coding")
  ("FaceSizeAlignment",                          m_faceSizeAlignment,                 4,                                    "Unit size for alignment, 0: minimal CU size")
  ("InternalChromaFormat,-intercf",              ctx.tmpInternalChromaFormat,             0,                                    "InternalChromaFormatIDC (400|420|422|444 or set 0 (default) for same as OutputChromaFormat)")
  ("InterpolationMethodY,-interpY",              m_inputGeoParam.iInterp[CHANNEL_TYPE_LUMA],   (Int)SI_LANCZOS3,            "Interpolation method for luma, 0: default setting(lanczos3); 1:NN, 2: bilinear, 3: bicubic, 4: lanczos2, 5: lanczos3")
  ("InterpolationMethodC,-interpC",              m_inputGeoParam.iInterp[CHANNEL_TYPE_CHROMA], (Int)SI_LANCZOS2,            "Interpolation method for chroma, 0: default setting(lanczos2); 1:NN, 2: bilinear, 3: bicubic, 4: lanczos2, 5: lanczos3")
  ("ResampleChroma,-rc",                         m_inputGeoParam.bResampleChroma,     false,                                "ResampleChroma indiates to do conversion with aligned phase with luma")
  ("ChromaSampleLocType,-csl",                   m_inputGeoParam.iChromaSampleLocType, 2,                                   "Chroma sample location type relative to luma, 0: 0.5 shift in vertical direction; 1: 0.5 shift in both directions, 2: aligned with luma (default setting), 3: 0.5 shift in horizontal direction")
#if SVIDEO_VIEWPORT_PSNR
#if SVIDEO_DYNAMIC_VIEWPORT_PSNR
  ("ViewPortPSNREnable,-vppsnr",                 m_viewPortPSNRParam.bViewPortPSNREnabled,       false,              "Flag to enable viewport PSNR calculation")  
#else
  ("ViewPortPSNREnable,-vppsnr",                 m_viewPortPSNRParam.bViewPortPSNREnabled,       true,               "Flag to enable viewport PSNR calculation")
#endif
  ("ViewPortList",                               m_viewPortPSNRParam.viewPortSettingsList,              ctx.defViewPortLists,   "Viewport settings list for static viewport PSNR calculation")
  ("ViewPortWidth",                              m_viewPortPSNRParam.iViewPortWidth,                    1816,               "Viewport width for static viewport PSNR calculation")
  ("ViewPortHeight",                             m_viewPortPSNRParam.iViewPortHeight,                   1816,               "Viewport height for static viewport PSNR calculation")
#endif
#if SVIDEO_DYNAMIC_VIEWPORT_PSNR
  ("DynamicViewPortPSNREnable,-dynvppsnr",       m_dynamicViewPortPSNRParam.bViewPortPSNREnabled,       true,               "Flag to enable dynamic viewport PSNR calculation")  
  ("DynamicViewPortList",                        m_dynamicViewPortPSNRParam.viewPortSettingsList,       ctx.defDynViewPortLists, "Start and end viewports setting for dynamic viewport PSNR calculation") 
  ("DynamicViewPortWidth",                       m_dynamicViewPortPSNRParam.iViewPortWidth,             1816,               "Viewport width for dynamic viewport PSNR calculation") 
  ("DynamicViewPortHeight",                      m_dynamicViewPortPSNRParam.iViewPortHeight,            1816,               "Viewport height for dynamic viewport PSNR calculation")
#endif
#if SVIDEO_SPSNR_NN
#if SVIDEO_E2E_METRICS
  ("SPSNR_NN,-spsnr_nn",                         m_bSPSNRNNEnabled,                            true,  "Flag to enable end to end spsnr-nn calculation")
#else
  ("SPSNR_NN,-spsnr_nn",                         m_bSPSNRNNEnabled,                            true,  "Flag to enable spsnr calculation")
#endif
#if SVIDEO_CODEC_SPSNR_NN
  ("CODEC_SPSNR_NN,-codec_spsnr_nn",             m_bCodecSPSNRNNEnabled,                        true,  "Flag to enable S-PSNR-NN calculation in coding projection format domain")
#endif
#endif
#if SVIDEO_SPSNR_NN || SVIDEO_SPSNR_I || SVIDEO_CF_SPSNR_NN || SVIDEO_CF_SPSNR_I || SVIDEO_CODEC_SPSNR_NN
  ("SphFile",                                    m_sphFilename,                                           std::string(""),         "Spherical points data file name for S-PSNR calculation")
#endif
#if SVIDEO_WSPSNR
  ("WSPSNR,-wspsnr",                             m_bWSPSNREnabled,                            true,  "Flag to enable ws-psnr calculation")
#endif
#if SVIDEO_SPSNR_I
#if SVIDEO_E2E_METRICS
  ("SPSNR_I,-spsnr_i",                           m_bSPSNRIEnabled,                                      false,  "Flag to enable end to end spsnr-i calculation")
#else
  ("SPSNR_I,-spsnr_i",                           m_bSPSNRIEnabled,                                      false,  "Flag to enable spsnr-i calculation")
#endif
#endif
#if SVIDEO_CPPPSNR
#if SVIDEO_E2E_METRICS
  ("CPP_PSNR,-cpsnr",                            m_bCPPPSNREnabled,                                     false, "Flag to enable end to end cpp-psnr calculation")
#else
  ("CPP_PSNR,-cpsnr",                            m_bCPPPSNREnabled,                                     false, "Flag to enable cpp-psnr calculation")
#endif
#endif
#if SVIDEO_WSPSNR_E2E
  ("E2EWSPSNR,-e2e_wspsnr",                      m_bE2EWSPSNREnabled,                           true,  "Flag to enable end to end ws-psnr calculation")
#endif
#if SVIDEO_CF_SPSNR_NN
  ("CF_SPSNR_NN,-cf_spsnr_nn",              m_bCFSPSNRNNEnabled,                           true,  "Flag to enable cross format spsnr-nn calculation")
#endif
#if SVIDEO_CF_SPSNR_I
  ("CF_SPSNR_I,-cf_spsnr_i",                m_bCFSPSNRIEnabled,                            false,  "Flag to enable cross format spsnr-i calculation")
#endif
#if SVIDEO_CF_CPPPSNR
  ("CF_CPP_PSNR,-cf_cpppsnr",               m_bCFCPPPSNREnabled,                           true, "Flag to enable cross format cpp-psnr calculation")
#endif
  ;
}


static inline ChromaFormat numberToChromaFormat(const Int val)
{
  switch (val)
  {
    case 400: return CHROMA_400; break;
    case 420: return CHROMA_420; break;
    case 422: return CHROMA_422; break;
    case 444: return CHROMA_444; break;
    default:  return NUM_CHROMA_FORMAT;
  }
}


Void TExt360AppEncCfg::processOptions(TExt360AppEncCfg::TExt360AppEncCfgContext &ctx)
{
  // Set any derived parameters
  if(m_bSVideo)
  {
    //fill the information;
    if (m_sourceSVideoInfo.geoType >= SVIDEO_TYPE_NUM)
    {
      printf( "InputGeometryType is invalid.\n" );
      exit( EXIT_FAILURE );
    }
    // Set default parameters

    xSetDefaultFramePackingParam(m_sourceSVideoInfo);
    xSetDefaultFramePackingParam(m_codingSVideoInfo);

    xFillSourceSVideoInfo(m_sourceSVideoInfo, m_cfg.m_inputFileWidth, m_cfg.m_inputFileHeight);
    if((m_sourceSVideoInfo.geoType == SVIDEO_OCTAHEDRON || m_sourceSVideoInfo.geoType == SVIDEO_ICOSAHEDRON) && ((m_sourceSVideoInfo.iFaceWidth%4) != 0 || (m_sourceSVideoInfo.iFaceHeight%4) != 0))
    {
      printf("For OHP and ISP, face width and height (%d, %d) are not multiple of 4.\n", m_sourceSVideoInfo.iFaceWidth, m_sourceSVideoInfo.iFaceHeight);
      exit( EXIT_FAILURE );
    }

    if (m_codingSVideoInfo.geoType >= SVIDEO_TYPE_NUM)
    {
      printf( "CodingGeometryType is invalid.\n" );
      exit( EXIT_FAILURE );
    }

    if(!m_faceSizeAlignment)
    {
      m_faceSizeAlignment = (Int)(m_cfg.m_uiMaxCUHeight >> (m_cfg.m_uiMaxCUDepth - 1));
    }
    //calculate the width/height for encoding based on frame packing information;
    xCalcOutputResolution(m_sourceSVideoInfo, m_codingSVideoInfo, m_cfg.m_iSourceWidth, m_cfg.m_iSourceHeight, m_faceSizeAlignment);


    m_sourceSVideoInfo.framePackStruct.chromaFormatIDC = m_cfg.m_InputChromaFormatIDC;
    m_codingSVideoInfo.framePackStruct.chromaFormatIDC = m_cfg.m_chromaFormatIDC;

    m_inputGeoParam.chromaFormat = ((ctx.tmpInternalChromaFormat == 0) ? (m_cfg.m_chromaFormatIDC) : (numberToChromaFormat(ctx.tmpInternalChromaFormat)));
    m_inputGeoParam.nBitDepth =  m_cfg.m_internalBitDepth[0];
    m_inputGeoParam.nOutputBitDepth = m_cfg.m_internalBitDepth[0];
    m_sourceSVideoInfo.framePackStruct.chromaFormatIDC = m_cfg.m_InputChromaFormatIDC;
    m_codingSVideoInfo.framePackStruct.chromaFormatIDC = m_cfg.m_chromaFormatIDC;
  }
}

Void TExt360AppEncCfg::xFillSourceSVideoInfo(SVideoInfo& sVidInfo, Int inputWidth, Int inputHeight)
{
  if(  sVidInfo.geoType == SVIDEO_EQUIRECT 
#if SVIDEO_ADJUSTED_EQUALAREA
    || sVidInfo.geoType == SVIDEO_ADJUSTEDEQUALAREA 
#else
    || sVidInfo.geoType == SVIDEO_EQUALAREA 
#endif
    || sVidInfo.geoType == SVIDEO_VIEWPORT)
  {
    //assert(sVidInfo.framePackStruct.rows == 1);
    //assert(sVidInfo.framePackStruct.cols == 1);
    //enforce;
    sVidInfo.framePackStruct.rows = 1;
    sVidInfo.framePackStruct.cols = 1;
    sVidInfo.framePackStruct.faces[0][0].id = 0; 
    //sVidInfo.framePackStruct.faces[0][0].rot = 0;
    sVidInfo.iNumFaces =1;
    if(sVidInfo.framePackStruct.faces[0][0].rot == 90 || sVidInfo.framePackStruct.faces[0][0].rot == 270)
    {
#if SVIDEO_ERP_PADDING
      if (sVidInfo.bPERP)
        inputHeight -= (SVIDEO_ERP_PAD_L + SVIDEO_ERP_PAD_R);
#endif
      sVidInfo.iFaceWidth = inputHeight;
      sVidInfo.iFaceHeight = inputWidth;
    }
    else
    {
#if SVIDEO_ERP_PADDING
      if (sVidInfo.bPERP)
        inputWidth -= (SVIDEO_ERP_PAD_L + SVIDEO_ERP_PAD_R);
#endif
      sVidInfo.iFaceWidth = inputWidth;
      sVidInfo.iFaceHeight = inputHeight;
    }
  }
  else if ((sVidInfo.geoType == SVIDEO_NEWUNIFORMMAP))
  {
	  //assert(sVidInfo.framePackStruct.rows == 1);
	  //assert(sVidInfo.framePackStruct.cols == 1);
	  //enforce;
	  sVidInfo.framePackStruct.rows = 1;
	  sVidInfo.framePackStruct.cols = 1;
	  sVidInfo.framePackStruct.faces[0][0].id = 0;
	  //sVidInfo.framePackStruct.faces[0][0].rot = 0;
	  sVidInfo.iNumFaces = 1;
	  if (sVidInfo.framePackStruct.faces[0][0].rot == 90 || sVidInfo.framePackStruct.faces[0][0].rot == 270)
	  {
		  sVidInfo.iFaceWidth = inputHeight;
		  sVidInfo.iFaceHeight = inputWidth;
	  }
	  else
	  {
#if SVIDEO_ERP_PADDING
		  if (sVidInfo.bPERP)
			  inputWidth -= (SVIDEO_ERP_PAD_L + SVIDEO_ERP_PAD_R);
#endif
		  sVidInfo.iFaceWidth = inputWidth;
		  sVidInfo.iFaceHeight = inputHeight;
	  }
  }
  else if (  (sVidInfo.geoType == SVIDEO_CUBEMAP)
#if SVIDEO_ADJUSTED_CUBEMAP
          || (sVidInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP)
#endif
#if SVIDEO_ROTATED_SPHERE
          || (sVidInfo.geoType == SVIDEO_ROTATEDSPHERE)
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
          || (sVidInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL)
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
          || (sVidInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP)
#endif
         )
  {
    //assert(sVidInfo.framePackStruct.cols*sVidInfo.framePackStruct.rows == 6);
    sVidInfo.iNumFaces = 6;
    //maybe there are some virtual faces;
    sVidInfo.iFaceWidth = inputWidth/sVidInfo.framePackStruct.cols;
    sVidInfo.iFaceHeight = inputHeight/sVidInfo.framePackStruct.rows;
    assert(sVidInfo.iFaceWidth == sVidInfo.iFaceHeight);
  }
  else if(sVidInfo.geoType == SVIDEO_OCTAHEDRON
         || sVidInfo.geoType == SVIDEO_ICOSAHEDRON
    )
  {
    //assert(sVidInfo.framePackStruct.cols*sVidInfo.framePackStruct.rows == 8);
    sVidInfo.iNumFaces = (sVidInfo.geoType == SVIDEO_OCTAHEDRON)? 8 : 20;
    //maybe there are some virtual faces;
    if(sVidInfo.geoType == SVIDEO_OCTAHEDRON && sVidInfo.iCompactFPStructure)
    {
#if SVIDEO_MTK_MODIFIED_COHP1
      if (sVidInfo.iCompactFPStructure == 1)
      {
#if SVIDEO_COHP1_PADDING
        inputHeight -= (S_COHP1_PAD << 1);
#endif
        sVidInfo.iFaceWidth  = inputHeight / (sVidInfo.framePackStruct.rows >> 1) - 4;
        sVidInfo.iFaceHeight = inputWidth / sVidInfo.framePackStruct.cols;
      }
      else
      {
        sVidInfo.iFaceWidth = (inputWidth/sVidInfo.framePackStruct.cols) - 4;
        sVidInfo.iFaceHeight = (inputHeight<<1)/sVidInfo.framePackStruct.rows;
      }
#else
      sVidInfo.iFaceWidth = (inputWidth/sVidInfo.framePackStruct.cols) - 4;
      sVidInfo.iFaceHeight = (inputHeight<<1)/sVidInfo.framePackStruct.rows;
#endif
    }
    else if(sVidInfo.geoType == SVIDEO_ICOSAHEDRON && sVidInfo.iCompactFPStructure)
    {
      Int halfCol = (sVidInfo.framePackStruct.cols>>1);
#if SVIDEO_SEC_VID_ISP3
      //if (sVidInfo.iCompactFPStructure == 1)
      {
        sVidInfo.iFaceWidth = (2 * inputWidth - 16 * halfCol - 8 - 2 * S_CISP_PAD_HOR) / (2 * halfCol + 1);
        sVidInfo.iFaceHeight = (inputHeight - S_CISP_PAD_VER) / sVidInfo.framePackStruct.rows;
      }
#else
#if SVIDEO_SEC_ISP
      sVidInfo.iFaceWidth = (2*inputWidth - 16*halfCol - 8 )/(2*halfCol + 1);
      sVidInfo.iFaceHeight = (inputHeight - 96)/sVidInfo.framePackStruct.rows;
#else
      sVidInfo.iFaceWidth = (2*inputWidth - 8*halfCol - 4 )/(2*halfCol + 1);
      sVidInfo.iFaceHeight = inputHeight/sVidInfo.framePackStruct.rows;
#endif
#endif
    }
    else
    {
      sVidInfo.iFaceWidth = inputWidth/sVidInfo.framePackStruct.cols;
      sVidInfo.iFaceHeight = inputHeight/sVidInfo.framePackStruct.rows;
    }
  }
#if SVIDEO_TSP_IMP
  else if(sVidInfo.geoType == SVIDEO_TSP)
  {
    sVidInfo.iNumFaces = 6;
    sVidInfo.iFaceWidth = inputWidth/sVidInfo.framePackStruct.cols;
    sVidInfo.iFaceHeight = inputHeight/sVidInfo.framePackStruct.rows;
    assert(sVidInfo.iFaceWidth == sVidInfo.iFaceHeight);
  }
#endif
#if SVIDEO_SEGMENTED_SPHERE
  else if(sVidInfo.geoType == SVIDEO_SEGMENTEDSPHERE)
  {
#if !SVIDEO_SSP_VERT
      sVidInfo.framePackStruct.rows = 1;
      sVidInfo.framePackStruct.cols = 6;
#endif
      //sVidInfo.framePackStruct.faces[0][0].id = 0;
      sVidInfo.iNumFaces = 6;
      sVidInfo.iFaceWidth = inputWidth / sVidInfo.framePackStruct.cols;
#if SVIDEO_EAP_SSP_PADDING
      sVidInfo.iFaceHeight = (inputHeight-(SVIDEO_SSP_GUARD_BAND << 2)) / sVidInfo.framePackStruct.rows;
#else
      sVidInfo.iFaceHeight = inputHeight / sVidInfo.framePackStruct.rows;
#endif
  }
#endif
  else
  {
    assert(!"Not supported yet");
  }
}

Void TExt360AppEncCfg::xCalcOutputResolution(SVideoInfo& sourceSVideoInfo, SVideoInfo& codingSVideoInfo, Int& iOutputWidth, Int& iOutputHeight, Int minCuSize)
{
  //calulate the coding resolution;
  if(  sourceSVideoInfo.geoType == SVIDEO_EQUIRECT 
	|| sourceSVideoInfo.geoType == SVIDEO_NEWUNIFORMMAP
#if SVIDEO_ADJUSTED_EQUALAREA
    || sourceSVideoInfo.geoType == SVIDEO_ADJUSTEDEQUALAREA
#else
    || sourceSVideoInfo.geoType == SVIDEO_EQUALAREA
#endif
    )
  {
    if(  codingSVideoInfo.geoType == SVIDEO_CUBEMAP 
      || codingSVideoInfo.geoType ==SVIDEO_OCTAHEDRON 
      || codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON
      || codingSVideoInfo.geoType ==SVIDEO_VIEWPORT
#if SVIDEO_TSP_IMP
      || codingSVideoInfo.geoType == SVIDEO_TSP
#endif
#if SVIDEO_SEGMENTED_SPHERE
      || codingSVideoInfo.geoType == SVIDEO_SEGMENTEDSPHERE
#endif
#if SVIDEO_ADJUSTED_CUBEMAP
      || codingSVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP
#endif
#if SVIDEO_ROTATED_SPHERE
      || codingSVideoInfo.geoType == SVIDEO_ROTATEDSPHERE
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
      || codingSVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
      || codingSVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP
#endif
      )
    {
      if( (codingSVideoInfo.geoType == SVIDEO_CUBEMAP) 
#if SVIDEO_TSP_IMP
        ||(codingSVideoInfo.geoType == SVIDEO_TSP)
#endif
#if SVIDEO_SEGMENTED_SPHERE
        ||(codingSVideoInfo.geoType == SVIDEO_SEGMENTEDSPHERE)
#endif
#if SVIDEO_ADJUSTED_CUBEMAP
        || (codingSVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP)
#endif
#if SVIDEO_ROTATED_SPHERE
        ||(codingSVideoInfo.geoType == SVIDEO_ROTATEDSPHERE)
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
        || (codingSVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL)
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
        || (codingSVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP)
#endif
        )
      {
        codingSVideoInfo.iNumFaces = 6;
      }
      else if(codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON)
      {
        codingSVideoInfo.iNumFaces = 8;
      }
      else if(codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
      {
        codingSVideoInfo.iNumFaces = 20;
      }
      else if(codingSVideoInfo.geoType == SVIDEO_VIEWPORT)
      {
        codingSVideoInfo.iNumFaces = 1;
      }
      if(m_iCodingFaceWidth==0 || m_iCodingFaceHeight==0)
      {
        Int tmp;
        if( (codingSVideoInfo.geoType == SVIDEO_CUBEMAP) 
#if SVIDEO_TSP_IMP
          ||(codingSVideoInfo.geoType == SVIDEO_TSP)
#endif
#if SVIDEO_ADJUSTED_CUBEMAP
          ||(codingSVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP)
#endif
#if SVIDEO_ROTATED_SPHERE
          ||(codingSVideoInfo.geoType == SVIDEO_ROTATEDSPHERE)
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
          ||(codingSVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL)
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
          ||(codingSVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP)
#endif
         )
        {
          tmp = Int(sqrt((Double)sourceSVideoInfo.iFaceWidth * sourceSVideoInfo.iFaceHeight/codingSVideoInfo.iNumFaces));
          codingSVideoInfo.iFaceWidth = codingSVideoInfo.iFaceHeight = (tmp + (minCuSize-1))/minCuSize*minCuSize;
        }
        else if(codingSVideoInfo.geoType ==SVIDEO_OCTAHEDRON
                || codingSVideoInfo.geoType ==SVIDEO_ICOSAHEDRON
          )
        {
          tmp = Int(sqrt((Double)sourceSVideoInfo.iFaceWidth * sourceSVideoInfo.iFaceHeight*4/(sqrt(3.0)*codingSVideoInfo.iNumFaces)));
          codingSVideoInfo.iFaceHeight = ((Int)(tmp*sqrt(3.0)/2.0 +0.5) + (minCuSize-1))/minCuSize*minCuSize;
          codingSVideoInfo.iFaceWidth = (tmp + (minCuSize-1))/minCuSize*minCuSize;
        }
        if((codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON || codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON) && ((codingSVideoInfo.iFaceWidth%4) != 0 || (codingSVideoInfo.iFaceHeight%4) != 0))
        {
          codingSVideoInfo.iFaceWidth = (codingSVideoInfo.iFaceWidth>>2)<<2;
          codingSVideoInfo.iFaceHeight = (codingSVideoInfo.iFaceHeight>>2)<<2;
        }
        iOutputWidth = codingSVideoInfo.iFaceWidth* codingSVideoInfo.framePackStruct.cols;
        iOutputHeight = codingSVideoInfo.iFaceHeight * codingSVideoInfo.framePackStruct.rows;
        if(codingSVideoInfo.iCompactFPStructure)
        {
          if(codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON)
          {
            iOutputWidth  = (codingSVideoInfo.iFaceWidth + 4) * codingSVideoInfo.framePackStruct.cols;
            iOutputHeight = (codingSVideoInfo.iFaceHeight>>1) * codingSVideoInfo.framePackStruct.rows;
#if SVIDEO_COHP1_PADDING
            iOutputHeight += (S_COHP1_PAD << 1);
#endif
          }
          else if(codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
          {
            Int halfCol = (codingSVideoInfo.framePackStruct.cols>>1);
#if SVIDEO_SEC_VID_ISP3
            //if (codingSVideoInfo.iCompactFPStructure == 1)
            {
              iOutputWidth = halfCol*(codingSVideoInfo.iFaceWidth + 8) + (codingSVideoInfo.iFaceWidth >> 1) + 4 + 2 * S_CISP_PAD_HOR;
              iOutputHeight = codingSVideoInfo.framePackStruct.rows*codingSVideoInfo.iFaceHeight + S_CISP_PAD_VER;
            }
#else
#if SVIDEO_SEC_ISP
            iOutputWidth  = halfCol*(codingSVideoInfo.iFaceWidth + 8) + (codingSVideoInfo.iFaceWidth>>1) + 4;
            iOutputHeight = codingSVideoInfo.framePackStruct.rows*codingSVideoInfo.iFaceHeight + 96;
#else
            iOutputWidth  = halfCol*(codingSVideoInfo.iFaceWidth + 4) + (codingSVideoInfo.iFaceWidth>>1) + 2;
            iOutputHeight = codingSVideoInfo.framePackStruct.rows*codingSVideoInfo.iFaceHeight;
#endif
#endif
          }
        }
#if SVIDEO_TSP_IMP
        if(codingSVideoInfo.geoType == SVIDEO_TSP)
        {
          iOutputWidth  = (codingSVideoInfo.iFaceWidth << 1);
          iOutputHeight = codingSVideoInfo.iFaceHeight;
        }
#endif
#if SVIDEO_SEGMENTED_SPHERE
        if(codingSVideoInfo.geoType == SVIDEO_SEGMENTEDSPHERE)
        {
          codingSVideoInfo.iFaceWidth = sourceSVideoInfo.iFaceWidth / 4;
          codingSVideoInfo.iFaceHeight = sourceSVideoInfo.iFaceHeight / 2;
          iOutputWidth = codingSVideoInfo.iFaceWidth*codingSVideoInfo.framePackStruct.cols;
          iOutputHeight = codingSVideoInfo.iFaceHeight*codingSVideoInfo.framePackStruct.rows;
#if SVIDEO_EAP_SSP_PADDING
          iOutputHeight += (SVIDEO_SSP_GUARD_BAND << 2);
#endif
        }
#endif
      }
      else
      {
        codingSVideoInfo.iFaceWidth = (m_iCodingFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
        codingSVideoInfo.iFaceHeight = (m_iCodingFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
        if((codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON || codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON) && ((codingSVideoInfo.iFaceWidth%4) != 0 || (codingSVideoInfo.iFaceHeight%4) != 0))
        {
          codingSVideoInfo.iFaceWidth = (codingSVideoInfo.iFaceWidth>>2)<<2;
          codingSVideoInfo.iFaceHeight = (codingSVideoInfo.iFaceHeight>>2)<<2;
        }
        iOutputWidth = codingSVideoInfo.iFaceWidth* codingSVideoInfo.framePackStruct.cols;
        iOutputHeight = codingSVideoInfo.iFaceHeight * codingSVideoInfo.framePackStruct.rows;
#if SVIDEO_EAP_SSP_PADDING
        if(codingSVideoInfo.geoType == SVIDEO_SEGMENTEDSPHERE)
          iOutputHeight += (SVIDEO_SSP_GUARD_BAND << 2);
#endif
        if(codingSVideoInfo.iCompactFPStructure)
        {
          if(codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON)
          {
#if SVIDEO_MTK_MODIFIED_COHP1
            if (codingSVideoInfo.iCompactFPStructure == 1)
            {
              iOutputHeight = (codingSVideoInfo.iFaceWidth + 4) * (codingSVideoInfo.framePackStruct.rows >> 1);
              iOutputWidth  = codingSVideoInfo.iFaceHeight * codingSVideoInfo.framePackStruct.cols;
#if SVIDEO_COHP1_PADDING
              iOutputHeight += (S_COHP1_PAD << 1);
#endif
            }
            else
            {
              iOutputWidth  = (codingSVideoInfo.iFaceWidth + 4) * codingSVideoInfo.framePackStruct.cols;
              iOutputHeight = (codingSVideoInfo.iFaceHeight>>1) * codingSVideoInfo.framePackStruct.rows;
            }
#else
            iOutputWidth  = (codingSVideoInfo.iFaceWidth + 4) * codingSVideoInfo.framePackStruct.cols;
            iOutputHeight = (codingSVideoInfo.iFaceHeight>>1) * codingSVideoInfo.framePackStruct.rows;
#endif
          }
          else if(codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
          {
            Int halfCol = (codingSVideoInfo.framePackStruct.cols>>1);
#if SVIDEO_SEC_VID_ISP3
            //if (codingSVideoInfo.iCompactFPStructure == 1)
            {
              iOutputWidth = halfCol*(codingSVideoInfo.iFaceWidth + 8) + (codingSVideoInfo.iFaceWidth >> 1) + 4 + 2 * S_CISP_PAD_HOR;
              iOutputHeight = codingSVideoInfo.framePackStruct.rows*codingSVideoInfo.iFaceHeight + S_CISP_PAD_VER;
            }
#else
#if SVIDEO_SEC_ISP
            iOutputWidth  = halfCol*(codingSVideoInfo.iFaceWidth + 8) + (codingSVideoInfo.iFaceWidth>>1) + 4;
            iOutputHeight = codingSVideoInfo.framePackStruct.rows*codingSVideoInfo.iFaceHeight + 96;
#else
            iOutputWidth  = halfCol*(codingSVideoInfo.iFaceWidth + 4) + (codingSVideoInfo.iFaceWidth>>1) + 2;
            iOutputHeight = codingSVideoInfo.framePackStruct.rows*codingSVideoInfo.iFaceHeight;
#endif
#endif
          }
        }
#if SVIDEO_TSP_IMP
        if(codingSVideoInfo.geoType == SVIDEO_TSP)
        {
          iOutputWidth  = (codingSVideoInfo.iFaceWidth << 1);
          iOutputHeight = codingSVideoInfo.iFaceHeight;
        }
#endif
      }
    }
    else if(   codingSVideoInfo.geoType ==SVIDEO_EQUIRECT 
	        || codingSVideoInfo.geoType == SVIDEO_NEWUNIFORMMAP
#if SVIDEO_ADJUSTED_EQUALAREA
            || codingSVideoInfo.geoType == SVIDEO_ADJUSTEDEQUALAREA
#else
            || codingSVideoInfo.geoType == SVIDEO_EQUALAREA
#endif
#if SVIDEO_CPPPSNR
            || codingSVideoInfo.geoType == SVIDEO_CRASTERSPARABOLIC
#endif
      )
    {
      codingSVideoInfo.iNumFaces = 1;
      if(m_iCodingFaceWidth==0 || m_iCodingFaceHeight==0)
      {
        codingSVideoInfo.iFaceWidth = (sourceSVideoInfo.iFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
        codingSVideoInfo.iFaceHeight = (sourceSVideoInfo.iFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
      }
      else
      {
        codingSVideoInfo.iFaceWidth = (m_iCodingFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
        codingSVideoInfo.iFaceHeight = (m_iCodingFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
      }

      Int degree = codingSVideoInfo.framePackStruct.faces[0][0].rot;
      if(degree ==90 || degree ==270)
      {
        iOutputWidth = codingSVideoInfo.iFaceHeight;
        iOutputHeight = codingSVideoInfo.iFaceWidth;
#if SVIDEO_ERP_PADDING
#if 1 //bugfix;
        if (codingSVideoInfo.bPERP)
#else
        if (!sourceSVideoInfo.bPERP && codingSVideoInfo.bPERP)
#endif
            iOutputHeight += SVIDEO_ERP_PAD_L + SVIDEO_ERP_PAD_R;
#endif
      }
      else
      {
        iOutputWidth = codingSVideoInfo.iFaceWidth;
        iOutputHeight = codingSVideoInfo.iFaceHeight; 
#if SVIDEO_ERP_PADDING
#if 1 //bugfix;
        if (codingSVideoInfo.bPERP)
#else
        if (!sourceSVideoInfo.bPERP && codingSVideoInfo.bPERP)
#endif
            iOutputWidth += SVIDEO_ERP_PAD_L + SVIDEO_ERP_PAD_R;
#endif
      }
    }
    else
    {
      assert(!"Not supported yet");
    }
  }
  //else if (sourceSVideoInfo.geoType == SVIDEO_NEWUNIFORMMAP)

  else if(    (sourceSVideoInfo.geoType == SVIDEO_CUBEMAP) 
           || (sourceSVideoInfo.geoType == SVIDEO_OCTAHEDRON) 
           || (sourceSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
#if SVIDEO_TSP_IMP
           || (sourceSVideoInfo.geoType == SVIDEO_TSP)
#endif
#if SVIDEO_ADJUSTED_CUBEMAP
           || (sourceSVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP)
#endif
#if SVIDEO_ROTATED_SPHERE
           || (sourceSVideoInfo.geoType == SVIDEO_ROTATEDSPHERE)
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
           || (sourceSVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL)
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
           || (codingSVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP)
#endif
           )
  { 
    if(  codingSVideoInfo.geoType == SVIDEO_EQUIRECT 
	  || codingSVideoInfo.geoType == SVIDEO_NEWUNIFORMMAP
#if SVIDEO_ADJUSTED_EQUALAREA
      || codingSVideoInfo.geoType == SVIDEO_ADJUSTEDEQUALAREA 
#else
      || codingSVideoInfo.geoType == SVIDEO_EQUALAREA 
#endif
      || codingSVideoInfo.geoType == SVIDEO_VIEWPORT)
    {
      codingSVideoInfo.iNumFaces = 1;
      if(m_iCodingFaceWidth==0 || m_iCodingFaceHeight==0)
      {
        Int tmp;
        if( (sourceSVideoInfo.geoType == SVIDEO_CUBEMAP )
#if SVIDEO_ADJUSTED_CUBEMAP
          ||(sourceSVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP)
#endif
#if SVIDEO_ROTATED_SPHERE
          ||(sourceSVideoInfo.geoType == SVIDEO_ROTATEDSPHERE)
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
          ||(sourceSVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL)
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
          ||(sourceSVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP)
#endif
           )
          tmp = Int(sqrt((Double)sourceSVideoInfo.iFaceWidth * sourceSVideoInfo.iFaceHeight*sourceSVideoInfo.iNumFaces*2.0));
        else if((sourceSVideoInfo.geoType == SVIDEO_OCTAHEDRON) || (sourceSVideoInfo.geoType == SVIDEO_ICOSAHEDRON))
          tmp = Int(sqrt((Double)sourceSVideoInfo.iFaceWidth * sourceSVideoInfo.iFaceHeight*sourceSVideoInfo.iNumFaces));
#if SVIDEO_TSP_IMP
        else if (sourceSVideoInfo.geoType == SVIDEO_TSP)
          tmp = sourceSVideoInfo.iFaceWidth*4;
#endif
        else
          assert(!"To be extended\n");
        codingSVideoInfo.iFaceHeight = (tmp/2 + (minCuSize-1))/minCuSize*minCuSize;
        codingSVideoInfo.iFaceWidth = codingSVideoInfo.iFaceHeight*2;
      }
      else
      {
        codingSVideoInfo.iFaceWidth = (m_iCodingFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
        codingSVideoInfo.iFaceHeight = (m_iCodingFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
      }
      Int degree = codingSVideoInfo.framePackStruct.faces[0][0].rot;
      if(degree ==90 || degree ==270)
      {
        iOutputWidth = codingSVideoInfo.iFaceHeight;
        iOutputHeight = codingSVideoInfo.iFaceWidth;
      }
      else
      {
        iOutputWidth = codingSVideoInfo.iFaceWidth;
        iOutputHeight = codingSVideoInfo.iFaceHeight;
      }
    }
#if SVIDEO_SEGMENTED_SPHERE
    else if(codingSVideoInfo.geoType == SVIDEO_SEGMENTEDSPHERE)
    {
        codingSVideoInfo.iNumFaces = 6;
        if (m_iCodingFaceWidth == 0 || m_iCodingFaceHeight == 0)
        {
            Int tmp;
            if( (sourceSVideoInfo.geoType == SVIDEO_CUBEMAP)
#if SVIDEO_ADJUSTED_CUBEMAP
              ||(sourceSVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP)
#endif
#if SVIDEO_ROTATED_SPHERE
              ||(sourceSVideoInfo.geoType == SVIDEO_ROTATEDSPHERE)
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
              ||(sourceSVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL)
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
              ||(sourceSVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP)
#endif
                )
            {
                tmp = Int(sqrt((Double)sourceSVideoInfo.iFaceWidth * sourceSVideoInfo.iFaceHeight*sourceSVideoInfo.iNumFaces*2.0));
            }
            else if (sourceSVideoInfo.geoType == SVIDEO_OCTAHEDRON
                || (sourceSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
                )
            {
                tmp = Int(sqrt((Double)sourceSVideoInfo.iFaceWidth * sourceSVideoInfo.iFaceHeight*sourceSVideoInfo.iNumFaces));
            }
            else
            {
                assert(!"To be extended\n");
            }
            codingSVideoInfo.iFaceHeight = (tmp * 3 / 4 + (minCuSize - 1)) / minCuSize*minCuSize;
            codingSVideoInfo.iFaceWidth = codingSVideoInfo.iFaceHeight;
        }
        else
        {
            codingSVideoInfo.iFaceWidth = (m_iCodingFaceWidth + (minCuSize - 1)) / minCuSize*minCuSize;
            codingSVideoInfo.iFaceHeight = (m_iCodingFaceHeight + (minCuSize - 1)) / minCuSize*minCuSize;
        }
        iOutputWidth = codingSVideoInfo.iFaceWidth*codingSVideoInfo.framePackStruct.cols;
        iOutputHeight = codingSVideoInfo.iFaceHeight*codingSVideoInfo.framePackStruct.rows;
#if SVIDEO_EAP_SSP_PADDING
        iOutputHeight += (SVIDEO_SSP_GUARD_BAND << 2);
#endif
    }
#endif
    else if(  (codingSVideoInfo.geoType == SVIDEO_CUBEMAP )
#if SVIDEO_ADJUSTED_CUBEMAP
           || (codingSVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP)
#endif
#if SVIDEO_ROTATED_SPHERE
           || (codingSVideoInfo.geoType == SVIDEO_ROTATEDSPHERE)
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
           || (codingSVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL)
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
           || (codingSVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP)
#endif
            )
    {
      codingSVideoInfo.iNumFaces = 6;
      if(m_iCodingFaceWidth==0 || m_iCodingFaceHeight==0)
      {
        if( (sourceSVideoInfo.geoType == SVIDEO_CUBEMAP)
#if SVIDEO_TSP_IMP
          ||(sourceSVideoInfo.geoType == SVIDEO_TSP)
#endif
#if SVIDEO_ADJUSTED_CUBEMAP
          || (sourceSVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP)
#endif
#if SVIDEO_ROTATED_SPHERE
          || (sourceSVideoInfo.geoType == SVIDEO_ROTATEDSPHERE)
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
          || (sourceSVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL)
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
          || (sourceSVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP)
#endif
          )
        {
          codingSVideoInfo.iFaceWidth = (sourceSVideoInfo.iFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
          codingSVideoInfo.iFaceHeight = (sourceSVideoInfo.iFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
        }
        else if(sourceSVideoInfo.geoType == SVIDEO_OCTAHEDRON
                || (sourceSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
          )
        {
          Int tmp = Int(sqrt((Double)sourceSVideoInfo.iFaceWidth * sourceSVideoInfo.iFaceHeight*sourceSVideoInfo.iNumFaces*0.5/codingSVideoInfo.iNumFaces));
          codingSVideoInfo.iFaceHeight = codingSVideoInfo.iFaceWidth = (tmp + (minCuSize-1))/minCuSize*minCuSize;
        }
        else
          assert(!"To be extended\n");
      }
      else
      {
        codingSVideoInfo.iFaceWidth = (m_iCodingFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
        codingSVideoInfo.iFaceHeight = (m_iCodingFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
      }
      iOutputWidth = codingSVideoInfo.iFaceWidth*codingSVideoInfo.framePackStruct.cols;
      iOutputHeight = codingSVideoInfo.iFaceHeight*codingSVideoInfo.framePackStruct.rows;
    }
    else if( codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON
           || (codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
      )
    {
      codingSVideoInfo.iNumFaces = (codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON)? 8 : 20;
      if(m_iCodingFaceWidth==0 || m_iCodingFaceHeight==0)
      {
        if(sourceSVideoInfo.geoType == SVIDEO_OCTAHEDRON
          || (sourceSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
          )
        {
          codingSVideoInfo.iFaceWidth = (sourceSVideoInfo.iFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
          codingSVideoInfo.iFaceHeight = (sourceSVideoInfo.iFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
        }
        else if( (sourceSVideoInfo.geoType == SVIDEO_CUBEMAP)
#if SVIDEO_TSP_IMP
               ||(sourceSVideoInfo.geoType == SVIDEO_TSP)
#endif
#if SVIDEO_ADJUSTED_CUBEMAP
               ||(sourceSVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP)
#endif
#if SVIDEO_ROTATED_SPHERE
               ||(sourceSVideoInfo.geoType == SVIDEO_ROTATEDSPHERE)
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
               ||(sourceSVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL)
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
               ||(sourceSVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP)
#endif
                )
        {
          Int tmp = Int(sqrt((Double)sourceSVideoInfo.iFaceWidth * sourceSVideoInfo.iFaceHeight*sourceSVideoInfo.iNumFaces*4/(sqrt(3.0)*codingSVideoInfo.iNumFaces)));
          codingSVideoInfo.iFaceHeight = ((Int)(tmp*sqrt(3.0)/2.0 +0.5) + (minCuSize-1))/minCuSize*minCuSize;
          codingSVideoInfo.iFaceWidth = (tmp + (minCuSize-1))/minCuSize*minCuSize;
        }
        else
          assert(!"To be extended\n");
      }
      else
      {
        codingSVideoInfo.iFaceWidth = (m_iCodingFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
        codingSVideoInfo.iFaceHeight = (m_iCodingFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
      }
      if((codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON || codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON) && ((codingSVideoInfo.iFaceWidth%4) != 0 || (codingSVideoInfo.iFaceHeight%4) != 0))
      {
        codingSVideoInfo.iFaceWidth = (codingSVideoInfo.iFaceWidth>>2)<<2;
        codingSVideoInfo.iFaceHeight = (codingSVideoInfo.iFaceHeight>>2)<<2;
      }
      iOutputWidth = codingSVideoInfo.iFaceWidth*codingSVideoInfo.framePackStruct.cols;
      iOutputHeight = codingSVideoInfo.iFaceHeight*codingSVideoInfo.framePackStruct.rows;
      if(codingSVideoInfo.iCompactFPStructure)
      {
        if(codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON)
        {
          iOutputWidth  = (codingSVideoInfo.iFaceWidth + 4)*codingSVideoInfo.framePackStruct.cols;
          iOutputHeight = (codingSVideoInfo.iFaceHeight>>1)*codingSVideoInfo.framePackStruct.rows;
        }
        else if(codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
        {
          Int halfCol = (codingSVideoInfo.framePackStruct.cols>>1);
          iOutputWidth  = halfCol*(codingSVideoInfo.iFaceWidth + 4) + (codingSVideoInfo.iFaceWidth>>1) + 2;
          iOutputHeight = codingSVideoInfo.framePackStruct.rows*codingSVideoInfo.iFaceHeight;
        }
      } 
    }
#if SVIDEO_TSP_IMP
    else if(codingSVideoInfo.geoType == SVIDEO_TSP )
    {
      codingSVideoInfo.iNumFaces = 6;
      if(m_iCodingFaceWidth==0 || m_iCodingFaceHeight==0)
      {
        if(  (sourceSVideoInfo.geoType == SVIDEO_CUBEMAP) 
          || (sourceSVideoInfo.geoType == SVIDEO_TSP) 
#if SVIDEO_ADJUSTED_CUBEMAP
          || (sourceSVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP)
#endif
#if SVIDEO_ROTATED_SPHERE
          || (sourceSVideoInfo.geoType == SVIDEO_ROTATEDSPHERE)
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
          || (sourceSVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL)
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
          || (sourceSVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP)
#endif
           )
        {
          codingSVideoInfo.iFaceWidth = (sourceSVideoInfo.iFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
          codingSVideoInfo.iFaceHeight = (sourceSVideoInfo.iFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
        }
        else if(sourceSVideoInfo.geoType == SVIDEO_OCTAHEDRON
                || (sourceSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
          )
        {
          Int tmp = Int(sqrt((Double)sourceSVideoInfo.iFaceWidth * sourceSVideoInfo.iFaceHeight*sourceSVideoInfo.iNumFaces*0.5/codingSVideoInfo.iNumFaces));
          codingSVideoInfo.iFaceHeight = codingSVideoInfo.iFaceWidth = (tmp + (minCuSize-1))/minCuSize*minCuSize;
        }
        else
        {
          assert(!"To be extended\n");
        }
      }
      else
      {
        codingSVideoInfo.iFaceWidth = (m_iCodingFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
        codingSVideoInfo.iFaceHeight = (m_iCodingFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
      }
      iOutputWidth = (codingSVideoInfo.iFaceWidth << 1);
      iOutputHeight = codingSVideoInfo.iFaceHeight;
    }
#endif
    else
    {
      assert(!"Not supported yet");
    }
  }   
#if SVIDEO_SEGMENTED_SPHERE
  else if(sourceSVideoInfo.geoType == SVIDEO_SEGMENTEDSPHERE)
  {
      Int height = sourceSVideoInfo.iFaceHeight * 2, width = sourceSVideoInfo.iFaceWidth * 4;
      if (   codingSVideoInfo.geoType == SVIDEO_CUBEMAP
          || codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON
          || codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON
          || codingSVideoInfo.geoType == SVIDEO_VIEWPORT
          || codingSVideoInfo.geoType == SVIDEO_SEGMENTEDSPHERE
#if SVIDEO_ADJUSTED_CUBEMAP
          || codingSVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP
#endif
#if SVIDEO_ROTATED_SPHERE
          || codingSVideoInfo.geoType == SVIDEO_ROTATEDSPHERE
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
          || codingSVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
          || codingSVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP
#endif
          )
      {
          if( (codingSVideoInfo.geoType == SVIDEO_CUBEMAP)
#if SVIDEO_ADJUSTED_CUBEMAP
            ||(codingSVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP)
#endif
#if SVIDEO_ROTATED_SPHERE
            ||(codingSVideoInfo.geoType == SVIDEO_ROTATEDSPHERE)
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
            ||(codingSVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL)
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
            ||(codingSVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP)
#endif
            )
          {
              codingSVideoInfo.iNumFaces = 6;
          }
          else if (codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON)
          {
              codingSVideoInfo.iNumFaces = 8;
          }
          else if (codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
          {
              codingSVideoInfo.iNumFaces = 20;
          }
          else if (codingSVideoInfo.geoType == SVIDEO_SEGMENTEDSPHERE)
          {
              codingSVideoInfo.iNumFaces = 3;
          }
          else if (codingSVideoInfo.geoType == SVIDEO_VIEWPORT)
          {
              codingSVideoInfo.iNumFaces = 1;
          }

          if (m_iCodingFaceWidth == 0 || m_iCodingFaceHeight == 0)
          {
              Int tmp;
              if( (codingSVideoInfo.geoType == SVIDEO_CUBEMAP)
#if SVIDEO_ADJUSTED_CUBEMAP
                ||(codingSVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP)
#endif
#if SVIDEO_ROTATED_SPHERE
                ||(codingSVideoInfo.geoType == SVIDEO_ROTATEDSPHERE)
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
                ||(codingSVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL)
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
                ||(codingSVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP)
#endif
                )
              {
                  tmp = Int(sqrt((Double)width * height / codingSVideoInfo.iNumFaces));
                  codingSVideoInfo.iFaceWidth = codingSVideoInfo.iFaceHeight = (tmp + (minCuSize - 1)) / minCuSize*minCuSize;
              }
              else if (codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON
                  || codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON
                  )
              {
                  tmp = Int(sqrt((Double)width * height * 4 / (sqrt(3.0)*codingSVideoInfo.iNumFaces)));
                  codingSVideoInfo.iFaceHeight = ((Int)(tmp*sqrt(3.0) / 2.0 + 0.5) + (minCuSize - 1)) / minCuSize*minCuSize;
                  codingSVideoInfo.iFaceWidth = (tmp + (minCuSize - 1)) / minCuSize*minCuSize;
              }
              if ((codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON || codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON) && (codingSVideoInfo.iFaceWidth != 0 || codingSVideoInfo.iFaceHeight % 4 != 0))
              {
                  codingSVideoInfo.iFaceWidth = codingSVideoInfo.iFaceWidth / 4 * 4;
                  codingSVideoInfo.iFaceHeight = codingSVideoInfo.iFaceHeight / 4 * 4;
              }
              iOutputWidth = codingSVideoInfo.iFaceWidth* codingSVideoInfo.framePackStruct.cols;
              iOutputHeight = codingSVideoInfo.iFaceHeight * codingSVideoInfo.framePackStruct.rows;
              if (codingSVideoInfo.iCompactFPStructure)
              {
                  if (codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON)
                  {
                      iOutputWidth = (codingSVideoInfo.iFaceWidth + 4) * codingSVideoInfo.framePackStruct.cols;
                      iOutputHeight = (codingSVideoInfo.iFaceHeight >> 1) * codingSVideoInfo.framePackStruct.rows;
                  }
                  else if (codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
                  {
                      Int halfCol = (codingSVideoInfo.framePackStruct.cols >> 1);
                      iOutputWidth = halfCol*(codingSVideoInfo.iFaceWidth + 4) + (codingSVideoInfo.iFaceWidth >> 1) + 2;
                      iOutputHeight = codingSVideoInfo.framePackStruct.rows*codingSVideoInfo.iFaceHeight;
                  }
              }
              if (codingSVideoInfo.geoType == SVIDEO_SEGMENTEDSPHERE)
              {
                  codingSVideoInfo.iFaceWidth = iOutputWidth = width;
                  codingSVideoInfo.iFaceHeight = height;
                  iOutputHeight = height * 3 / 2;
#if SVIDEO_EAP_SSP_PADDING
                  iOutputHeight += (SVIDEO_SSP_GUARD_BAND << 2);
#endif
              }
          }
          else
          {
              codingSVideoInfo.iFaceWidth = (m_iCodingFaceWidth + (minCuSize - 1)) / minCuSize*minCuSize;
              codingSVideoInfo.iFaceHeight = (m_iCodingFaceHeight + (minCuSize - 1)) / minCuSize*minCuSize;
              if ((codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON || codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON) && (codingSVideoInfo.iFaceWidth != 0 || codingSVideoInfo.iFaceHeight % 4 != 0))
              {
                  codingSVideoInfo.iFaceWidth = codingSVideoInfo.iFaceWidth / 4 * 4;
                  codingSVideoInfo.iFaceHeight = codingSVideoInfo.iFaceHeight / 4 * 4;
              }
              iOutputWidth = codingSVideoInfo.iFaceWidth* codingSVideoInfo.framePackStruct.cols;
              iOutputHeight = codingSVideoInfo.iFaceHeight * codingSVideoInfo.framePackStruct.rows;
#if SVIDEO_EAP_SSP_PADDING
              if(codingSVideoInfo.geoType == SVIDEO_SEGMENTEDSPHERE)
                iOutputHeight += (SVIDEO_SSP_GUARD_BAND << 2);
#endif
              if (codingSVideoInfo.iCompactFPStructure)
              {
                  if (codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON)
                  {
                      iOutputWidth = (codingSVideoInfo.iFaceWidth + 4) * codingSVideoInfo.framePackStruct.cols;
                      iOutputHeight = (codingSVideoInfo.iFaceHeight >> 1) * codingSVideoInfo.framePackStruct.rows;
                  }
                  else if (codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
                  {
                      Int halfCol = (codingSVideoInfo.framePackStruct.cols >> 1);
                      iOutputWidth = halfCol*(codingSVideoInfo.iFaceWidth + 4) + (codingSVideoInfo.iFaceWidth >> 1) + 2;
                      iOutputHeight = codingSVideoInfo.framePackStruct.rows*codingSVideoInfo.iFaceHeight;
                  }
              }
          }
      }
      else if (   codingSVideoInfo.geoType == SVIDEO_EQUIRECT 
	           || codingSVideoInfo.geoType == SVIDEO_NEWUNIFORMMAP
#if SVIDEO_ADJUSTED_EQUALAREA
               || codingSVideoInfo.geoType == SVIDEO_ADJUSTEDEQUALAREA
#else
               || codingSVideoInfo.geoType == SVIDEO_EQUALAREA
#endif
        )
      {
          codingSVideoInfo.iNumFaces = 1;
          if (m_iCodingFaceWidth == 0 || m_iCodingFaceHeight == 0)
          {
              codingSVideoInfo.iFaceWidth = (width + (minCuSize - 1)) / minCuSize*minCuSize;
              codingSVideoInfo.iFaceHeight = (height + (minCuSize - 1)) / minCuSize*minCuSize;
          }
          else
          {
              codingSVideoInfo.iFaceWidth = (m_iCodingFaceWidth + (minCuSize - 1)) / minCuSize*minCuSize;
              codingSVideoInfo.iFaceHeight = (m_iCodingFaceHeight + (minCuSize - 1)) / minCuSize*minCuSize;
          }

          Int degree = codingSVideoInfo.framePackStruct.faces[0][0].rot;
          if (degree == 90 || degree == 270)
          {
              iOutputWidth = codingSVideoInfo.iFaceHeight;
              iOutputHeight = codingSVideoInfo.iFaceWidth;
          }
          else
          {
              iOutputWidth = codingSVideoInfo.iFaceWidth;
              iOutputHeight = codingSVideoInfo.iFaceHeight;
          }
      }
  }
#endif
  else
  {
    printf("Source geometry type: %d is not supported!\n", sourceSVideoInfo.geoType);
    assert(0);
  }
}

Void TExt360AppEncCfg::xPrintGeoTypeName(Int nType, Bool bCompactFPFormat)
{
  switch(nType)
  {
  case SVIDEO_EQUIRECT:
    printf("Equirectangular ");
    break;
  case SVIDEO_CUBEMAP:
    printf("Cubemap ");
    break;
#if SVIDEO_ADJUSTED_EQUALAREA
  case SVIDEO_ADJUSTEDEQUALAREA:
    printf("Adjusted equal-area ");
    break;
#else
  case SVIDEO_EQUALAREA:
    printf("Equal-area ");
    break;
#endif
  case SVIDEO_OCTAHEDRON:
    if(!bCompactFPFormat)
    {
      printf("Octahedron ");
    }
    else
    {
      printf("Compact octahedron");
    }
    break;
  case SVIDEO_VIEWPORT:
    printf("Viewport ");
    break;
  case SVIDEO_ICOSAHEDRON:
    if(!bCompactFPFormat)
    {
      printf("Icosahedron ");
    }
    else
    {
      printf("Compact icosahedron ");
    }
    break;
#if SVIDEO_TSP_IMP
  case SVIDEO_TSP:
    printf("Truncated-Square-Pyramid ");
    break;
#endif
#if SVIDEO_SEGMENTED_SPHERE
  case SVIDEO_SEGMENTEDSPHERE:
    printf("Segmented-Sphere ");
    break;
#endif
#if SVIDEO_ADJUSTED_CUBEMAP
  case SVIDEO_ADJUSTEDCUBEMAP:
    printf("Adjusted-CubeMap ");
    break;
#endif
#if SVIDEO_ROTATED_SPHERE
  case SVIDEO_ROTATEDSPHERE:
    printf("Rotated-Sphere ");
    break;
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
  case SVIDEO_EQUATORIALCYLINDRICAL:
    printf("Equatorial-Cylindrical ");
    break;
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
  case SVIDEO_EQUIANGULARCUBEMAP:
    printf("Equiangular-CubeMap ");
    break;
#endif
  default:
    printf("Unknown ");
    break;
  }
  printf("\n");
}

Bool TExt360AppEncCfg::verifyParameters()
{
  Bool check_failed=false;

#define xConfirmPara(a,b) check_failed |= confirmPara(a,b)

  if(m_bSVideo)
  {
    xConfirmPara(m_faceSizeAlignment<0, "FaceSizeAlignment must be no less than 0");
    //check source;
    if(   m_sourceSVideoInfo.geoType == SVIDEO_EQUIRECT 
	   || m_sourceSVideoInfo.geoType == SVIDEO_NEWUNIFORMMAP
#if SVIDEO_ADJUSTED_EQUALAREA
       || m_sourceSVideoInfo.geoType == SVIDEO_ADJUSTEDEQUALAREA
#else
       || m_sourceSVideoInfo.geoType == SVIDEO_EQUALAREA
#endif
      )
    {
      xConfirmPara(m_sourceSVideoInfo.framePackStruct.cols != 1 || m_sourceSVideoInfo.framePackStruct.rows != 1, "source: cols and rows of frame packing must be 1 for equirectangular and equalarea");
      for(Int j=0; j<m_sourceSVideoInfo.framePackStruct.rows; j++)
        for(Int i=0; i<m_sourceSVideoInfo.framePackStruct.cols; i++)
        {
          xConfirmPara(m_sourceSVideoInfo.framePackStruct.faces[j][i].id != 0, "source: face id of frame packing must be 0 for equirectangular and equalarea");
          xConfirmPara((m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 0) &&(m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 90)&&(m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 180)&& (m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 270)
            , "source: face rotation of frame packing must be {0, 90, 180, 270}");
        }
    }
    if( (m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP)
#if SVIDEO_ADJUSTED_CUBEMAP
      ||(m_sourceSVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP)
#endif
#if SVIDEO_ROTATED_SPHERE
      ||(m_sourceSVideoInfo.geoType == SVIDEO_ROTATEDSPHERE)
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
      ||(m_sourceSVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL)
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
      ||(m_sourceSVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP)
#endif
       )
    {
      xConfirmPara(m_sourceSVideoInfo.framePackStruct.cols*m_sourceSVideoInfo.framePackStruct.rows < 6, "source: cols x rows of frame packing must be no smaller than 6 for cubemap");
      Int iActualFaces = 0;
      for(Int j=0; j<m_sourceSVideoInfo.framePackStruct.rows; j++)
        for(Int i=0; i<m_sourceSVideoInfo.framePackStruct.cols; i++)
        {
          //xConfirmPara(m_sourceSVideoInfo.framePackStruct.faces[j][i].id < 0 || m_sourceSVideoInfo.framePackStruct.faces[j][i].id > 5, "source: face id of frame packing must be in [0,5] for cubemap");
          iActualFaces += (m_sourceSVideoInfo.framePackStruct.faces[j][i].id <6)? 1: 0;
          xConfirmPara((m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 0) &&(m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 90)&&(m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 180)&& (m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 270)
                        , "source: face rotation of frame packing must be {0, 90, 180, 270}");
        }
        xConfirmPara(iActualFaces <6, "number of actual faces source must be no smaller than 6");
    }
    if(m_sourceSVideoInfo.iCompactFPStructure && !(m_sourceSVideoInfo.geoType == SVIDEO_OCTAHEDRON || m_sourceSVideoInfo.geoType == SVIDEO_ICOSAHEDRON))
    {
      printf("CompactCodingFPFormat is automatically disabled for source video because it is only supported for OHP and ISP\n");
      m_sourceSVideoInfo.iCompactFPStructure = 0;
    }
#if SVIDEO_TSP_IMP
    if(m_sourceSVideoInfo.geoType == SVIDEO_TSP)
    {
      for(Int j=0; j<m_sourceSVideoInfo.framePackStruct.rows; j++)
        for(Int i=0; i<m_sourceSVideoInfo.framePackStruct.cols; i++)
        {
          xConfirmPara(m_sourceSVideoInfo.framePackStruct.faces[j][i].id < 0 || m_sourceSVideoInfo.framePackStruct.faces[j][i].id > 1, "source: face id of frame packing must be in [0,1] for TSP");
          xConfirmPara((m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 0), "source: face rotation of frame packing must be {0}");
        }
    }
#endif
    //check coding;
    if(   m_codingSVideoInfo.geoType == SVIDEO_EQUIRECT 
	   || m_codingSVideoInfo.geoType == SVIDEO_NEWUNIFORMMAP
#if SVIDEO_ADJUSTED_EQUALAREA
       || m_codingSVideoInfo.geoType == SVIDEO_ADJUSTEDEQUALAREA
#else
       || m_codingSVideoInfo.geoType == SVIDEO_EQUALAREA
#endif
      )
    {
      xConfirmPara(m_codingSVideoInfo.framePackStruct.cols != 1 || m_codingSVideoInfo.framePackStruct.rows != 1, "coding: cols and rows of frame packing must be 1 for equirectangular and equalarea");
      for(Int j=0; j<m_codingSVideoInfo.framePackStruct.rows; j++)
        for(Int i=0; i<m_codingSVideoInfo.framePackStruct.cols; i++)
        {
          xConfirmPara(m_codingSVideoInfo.framePackStruct.faces[j][i].id != 0, "coding: face id of frame packing must be 0 for equirectangular and equalarea");
          xConfirmPara((m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 0) &&(m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 90) &&(m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 180) && (m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 270)
            , "coding: face rotation of frame packing must be {0, 90, 180, 270}");
        }
    }
    if( (m_codingSVideoInfo.geoType == SVIDEO_CUBEMAP)
#if SVIDEO_ADJUSTED_CUBEMAP
      ||(m_codingSVideoInfo.geoType == SVIDEO_ADJUSTEDCUBEMAP)
#endif
#if SVIDEO_ROTATED_SPHERE
      ||(m_codingSVideoInfo.geoType == SVIDEO_ROTATEDSPHERE)
#endif
#if SVIDEO_EQUATORIAL_CYLINDRICAL
      ||(m_codingSVideoInfo.geoType == SVIDEO_EQUATORIALCYLINDRICAL)
#endif
#if SVIDEO_EQUIANGULAR_CUBEMAP
      ||(m_codingSVideoInfo.geoType == SVIDEO_EQUIANGULARCUBEMAP)
#endif
       )
    {
      Int iTotalNumFaces = m_codingSVideoInfo.framePackStruct.cols*m_codingSVideoInfo.framePackStruct.rows; //including virtual faces;
      TChar msg[256];
      sprintf(msg, "coding: face id of frame packing must be in [0, %d] for cubemap", iTotalNumFaces);
      for(Int j=0; j<m_codingSVideoInfo.framePackStruct.rows; j++)
        for(Int i=0; i<m_codingSVideoInfo.framePackStruct.cols; i++)
        {
          xConfirmPara(m_codingSVideoInfo.framePackStruct.faces[j][i].id < 0 || m_codingSVideoInfo.framePackStruct.faces[j][i].id > iTotalNumFaces, msg);
          xConfirmPara((m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 0) &&(m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 90)&&(m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 180)&& (m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 270)
                        , "coding: face rotation of frame packing must be {0, 90, 180, 270}");
        }
    }
    if(m_codingSVideoInfo.iCompactFPStructure && !(m_codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON || m_codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON))
    {
      printf("CompactCodingFPFormat is automatically disabled for output video because it is only supported for OHP and ISP\n");
      m_codingSVideoInfo.iCompactFPStructure = 0;
    }
#if SVIDEO_TSP_IMP
    if(m_codingSVideoInfo.geoType == SVIDEO_TSP)
    {
      xConfirmPara(m_codingSVideoInfo.framePackStruct.cols*m_codingSVideoInfo.framePackStruct.rows != 2, "coding: cols x rows of frame packing must be 2 for TSP");
      for(Int j=0; j<m_codingSVideoInfo.framePackStruct.rows; j++)
        for(Int i=0; i<m_codingSVideoInfo.framePackStruct.cols; i++)
        {
          xConfirmPara(m_codingSVideoInfo.framePackStruct.faces[j][i].id < 0 || m_codingSVideoInfo.framePackStruct.faces[j][i].id > 1, "coding: face id of frame packing must be in [0,1] for TSP");
          xConfirmPara((m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 0), "coding: face rotation of frame packing must be {0}");
        }
    }
#endif
#if !SVIDEO_ROT_FIX
    for(Int i=0; i<3; i++)
    {
      TChar msgBuf[256];
      sprintf(msgBuf, "coding: SVideoRotation for dim(%d) must be in [0, 360)", i);
      xConfirmPara(m_codingSVideoInfo.sVideoRotation.degree[i] <0 || m_codingSVideoInfo.sVideoRotation.degree[i] >=360, msgBuf);
    }
#endif
    if(m_inputGeoParam.iInterp[CHANNEL_TYPE_LUMA] <=0 || m_inputGeoParam.iInterp[CHANNEL_TYPE_LUMA] >=SI_TYPE_NUM)
    {
      printf("The interpolation method is not valid for luma, and it is set to default value: Lanczos3\n");
      m_inputGeoParam.iInterp[CHANNEL_TYPE_LUMA] = SI_LANCZOS3;
    }
    if(m_inputGeoParam.iInterp[CHANNEL_TYPE_CHROMA] <=0 || m_inputGeoParam.iInterp[CHANNEL_TYPE_CHROMA] >=SI_TYPE_NUM)
    {
      printf("The interpolation method is not valid for chroma, and it is set to default value: Lanczos2\n");
      m_inputGeoParam.iInterp[CHANNEL_TYPE_CHROMA] = SI_LANCZOS2;
    }
    xConfirmPara( m_inputGeoParam.chromaFormat >= NUM_CHROMA_FORMAT,                          "InternalChromaFormatIDC must be either 400, 420, 422 or 444" );
    if(m_cfg.m_chromaFormatIDC == CHROMA_444 && m_inputGeoParam.chromaFormat != CHROMA_444)
    {
      printf("InternalChromaFormat is changed to 444 for better conversion quality because the output is 444!\n");
      m_inputGeoParam.chromaFormat = CHROMA_444;
    }
    if(m_cfg.m_InputChromaFormatIDC == CHROMA_444 && m_inputGeoParam.chromaFormat != CHROMA_444)
    {
      printf("InternalChromaFormat is changed to 444 because the input is 444!\n");
      m_inputGeoParam.chromaFormat = CHROMA_444;
    }
    if(m_cfg.m_chromaFormatIDC == CHROMA_400 && m_inputGeoParam.chromaFormat != CHROMA_400)
    {
      printf("InternalChromaFormat is changed to 400 because the output is 400!\n");
      m_inputGeoParam.chromaFormat = CHROMA_400;
    }
    if(m_cfg.m_InputChromaFormatIDC == CHROMA_400 && m_inputGeoParam.chromaFormat != CHROMA_400)
    {
      printf("InternalChromaFormat is changed to 400 because the input is 400!\n");
      m_inputGeoParam.chromaFormat = CHROMA_400;
    }
    if(m_inputGeoParam.chromaFormat != CHROMA_420 && m_inputGeoParam.bResampleChroma)
    {
      printf("ResampleChroma is reset to false because the internal chroma format is not 4:2:0!\n");
      m_inputGeoParam.bResampleChroma = false;
    }
    if(m_inputGeoParam.iChromaSampleLocType < 0 || m_inputGeoParam.iChromaSampleLocType > 3)
    {
      printf("ChromaSampleLocType must be in the range of [0, 3], and it is reset to 2!\n");
      m_inputGeoParam.iChromaSampleLocType = 2;
    }
#if !SVIDEO_WSPSNR_ISP1
    if(m_codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON && m_codingSVideoInfo.iCompactFPStructure)
    {
      if(m_bWSPSNREnabled)
      {
        printf("WS-PSNR is disabled!\n");
        m_bWSPSNREnabled = false;
      }
    }
#endif
#if SVIDEO_TSP_IMP
    if(m_codingSVideoInfo.geoType == SVIDEO_TSP || m_sourceSVideoInfo.geoType == SVIDEO_TSP)
    {
      printf("360 Metrics are disabled for TSP!\n");
      m_bSPSNRNNEnabled = false;
      m_bWSPSNREnabled = false;
      m_bE2EWSPSNREnabled = false;
      m_bSPSNRIEnabled = false;
      m_bCPPPSNREnabled = false;
      m_viewPortPSNRParam.bViewPortPSNREnabled = false;
#if SVIDEO_DYNAMIC_VIEWPORT_PSNR
      m_dynamicViewPortPSNRParam.bViewPortPSNREnabled = false;
#endif
#if SVIDEO_CF_SPSNR_NN
      m_bCFSPSNRNNEnabled = false;
#endif
#if SVIDEO_CF_SPSNR_I
      m_bCFSPSNRIEnabled = false;
#endif
#if SVIDEO_CF_CPPPSNR
      m_bCFCPPPSNREnabled = false;
#endif
#if SVIDEO_CODEC_SPSNR_NN
      m_bCodecSPSNRNNEnabled = false;
#endif
    }
#endif
  }
#if SVIDEO_VIEWPORT_PSNR
  else
  {
    m_viewPortPSNRParam.bViewPortPSNREnabled = false;
#if SVIDEO_DYNAMIC_VIEWPORT_PSNR
    m_dynamicViewPortPSNRParam.bViewPortPSNREnabled = false;
#endif
  }
#endif

#undef xConfirmPara

  return check_failed;
}

Void TExt360AppEncCfg::outputConfigurationSummary()
{
  printf("\n\n-----360Lib software version [%s]-----\n", VERSION_360Lib);
  printf("-----360 video parameters----\n");
  printf("SphereVideo:%d\n", m_bSVideo);
  if(m_bSVideo)
  {
    printf("InputGeometryType: ");
    xPrintGeoTypeName(m_sourceSVideoInfo.geoType, m_sourceSVideoInfo.iCompactFPStructure);
    printf("ChromaFormat:%d Resolution:%dx%dxF%d FPStructure:%dx%d | ", m_sourceSVideoInfo.framePackStruct.chromaFormatIDC, m_sourceSVideoInfo.iFaceWidth, m_sourceSVideoInfo.iFaceHeight, m_sourceSVideoInfo.iNumFaces,
      m_sourceSVideoInfo.framePackStruct.cols, m_sourceSVideoInfo.framePackStruct.rows);
    for(Int i=0; i<m_sourceSVideoInfo.framePackStruct.rows; i++)
    {
      for(Int j=0; j<m_sourceSVideoInfo.framePackStruct.cols; j++)
        printf("Id_%d(R_%d) ", m_sourceSVideoInfo.framePackStruct.faces[i][j].id,m_sourceSVideoInfo.framePackStruct.faces[i][j].rot);
      printf(" | ");
    }
    printf("\nCompact type: %d\n", m_sourceSVideoInfo.iCompactFPStructure);
#if SVIDEO_ERP_PADDING
    if(m_sourceSVideoInfo.geoType == SVIDEO_EQUIRECT)
      printf("InputPERP: %d\n", m_sourceSVideoInfo.bPERP);
#endif
    printf("\nCodingGeometryType: ");
    xPrintGeoTypeName(m_codingSVideoInfo.geoType, m_codingSVideoInfo.iCompactFPStructure);
    printf("ChromaFormat:%d Resolution:%dx%dxF%d FPStructure:%dx%d | ", m_codingSVideoInfo.framePackStruct.chromaFormatIDC, m_codingSVideoInfo.iFaceWidth, m_codingSVideoInfo.iFaceHeight, m_codingSVideoInfo.iNumFaces,
      m_codingSVideoInfo.framePackStruct.cols, m_codingSVideoInfo.framePackStruct.rows);
    for(Int i=0; i<m_codingSVideoInfo.framePackStruct.rows; i++)
    {
      for(Int j=0; j<m_codingSVideoInfo.framePackStruct.cols; j++)
        printf("Id_%d(R_%d) ", m_codingSVideoInfo.framePackStruct.faces[i][j].id, m_codingSVideoInfo.framePackStruct.faces[i][j].rot);
      printf(" | ");
    }
    printf("\nCompact type: %d\n", m_codingSVideoInfo.iCompactFPStructure);
#if SVIDEO_ERP_PADDING
    if(m_codingSVideoInfo.geoType == SVIDEO_EQUIRECT)
      printf("CodingPERP: %d\n", m_codingSVideoInfo.bPERP);
#endif
    printf("\n");
    if(m_codingSVideoInfo.geoType == SVIDEO_VIEWPORT)
      printf("Global viewport setting: %.2f %.2f %.2f %.2f\n", m_codingSVideoInfo.viewPort.hFOV, m_codingSVideoInfo.viewPort.vFOV, m_codingSVideoInfo.viewPort.fYaw, m_codingSVideoInfo.viewPort.fPitch);
    printf("Packed frame resolution: %dx%d (Input face resolution:%dx%d)\n", m_cfg.m_iSourceWidth, m_cfg.m_iSourceHeight, m_iCodingFaceWidth, m_iCodingFaceHeight);
    printf("Interpolation method for luma: %d, interpolation method for chroma: %d\n", m_inputGeoParam.iInterp[CHANNEL_TYPE_LUMA], m_inputGeoParam.iInterp[CHANNEL_TYPE_CHROMA]);
    printf("ChromaSampleLocType: %d\n", m_inputGeoParam.iChromaSampleLocType);
    printf("Input ChromaFormatIDC: %d; ", m_cfg.m_InputChromaFormatIDC);    
    if(m_inputGeoParam.chromaFormat == CHROMA_420)
      printf("Internal ChromaFormatIDC: %d, ChromaResample: %d; ", m_inputGeoParam.chromaFormat, m_inputGeoParam.bResampleChroma);
    else
      printf("Internal ChromaFormatIDC: %d; ", m_inputGeoParam.chromaFormat);
    printf("Output ChromaFormatIDC: %d\n", m_cfg.m_chromaFormatIDC);
    printf("Internal bit depth for projection conversion: %d, output bit depth from pejction conversion: %d\n", m_inputGeoParam.nBitDepth, m_inputGeoParam.nOutputBitDepth);

    if(isGeoConvertSkipped())
      printf("Geometry conversion is skipped!\n");

    printf("\n");
#if SVIDEO_SPSNR_NN
    if(m_bSPSNRNNEnabled)
    {
#if SVIDEO_E2E_METRICS
      printf("End to end S-PSNR-NN is enabled; SphFile file: %s\n", m_sphFilename.empty() ? "NULL" : m_sphFilename.c_str());
#else
      printf("S-PSNR-NN is enabled; SphFile file: %s\n", m_sphFilename.empty() ? "NULL" : m_sphFilename.c_str());
#endif
    }
#if SVIDEO_CODEC_SPSNR_NN
    if(m_bCodecSPSNRNNEnabled)
    {
      printf("Codec S-PSNR-NN is enabled; SphFile file: %s\n", m_sphFilename.empty() ? "NULL" : m_sphFilename.c_str());
    }
#endif
#endif
#if SVIDEO_WSPSNR
    if(m_bWSPSNREnabled)
      printf("WS-PSNR is enabled\n");
#endif
#if SVIDEO_SPSNR_I
    if(m_bSPSNRIEnabled)
    {
#if SVIDEO_E2E_METRICS
      printf("End to end S-PSNR-I is enabled; SphFile file: %s\n", m_sphFilename.empty() ? "NULL" : m_sphFilename.c_str());
#else
      printf("S-PSNR-I is enabled; SphFile file: %s\n", m_sphFilename.empty() ? "NULL" : m_sphFilename.c_str());
#endif
    }
#endif    
#if SVIDEO_CPPPSNR
    if(m_bCPPPSNREnabled)
    {
#if SVIDEO_E2E_METRICS
      printf("End to end CPP-PSNR is enabled\n");
#else
      printf("CPP-PSNR is enabled\n");
#endif
    }
#endif
#if SVIDEO_WSPSNR_E2E
    if(m_bE2EWSPSNREnabled)
      printf("End to end WS-PSNR is enabled\n");
#endif
#if SVIDEO_VIEWPORT_PSNR
    if(m_viewPortPSNRParam.bViewPortPSNREnabled)
    {
#if SVIDEO_DYNAMIC_VIEWPORT_PSNR
      printf("ViewPort parameters for static ViewPort PSNR calculation:\n");
#else
      printf("ViewPort parameters for ViewPort-PSNR calculation:\n");
#endif
      printf("Number of viewports: %d, Resolutoin:%dx%d\n", (Int)(m_viewPortPSNRParam.viewPortSettingsList.size()), m_viewPortPSNRParam.iViewPortWidth, m_viewPortPSNRParam.iViewPortHeight);
      for(std::vector<ViewPortSettings>::iterator it = m_viewPortPSNRParam.viewPortSettingsList.begin(); it != m_viewPortPSNRParam.viewPortSettingsList.end(); it++)
        printf("Global viewport setting: %.2f %.2f %.2f %.2f\n", it->hFOV, it->vFOV, it->fYaw, it->fPitch);      
    }
    else
#if SVIDEO_DYNAMIC_VIEWPORT_PSNR
      printf("Static ViewPort PSNR calculation is not enabled!\n");
#else
      printf("ViewPort-PSNR calculation is not enabled!\n");
#endif
#endif
#if SVIDEO_DYNAMIC_VIEWPORT_PSNR
    if(m_dynamicViewPortPSNRParam.bViewPortPSNREnabled)
    {
      printf("ViewPort parameters for dynamic ViewPort PSNR calculation:\n");
      printf("Number of viewports: %d, Resolutoin:%dx%d\n", (Int)(m_dynamicViewPortPSNRParam.viewPortSettingsList.size()), m_dynamicViewPortPSNRParam.iViewPortWidth, m_dynamicViewPortPSNRParam.iViewPortHeight);
      for(Int i =0; i<m_dynamicViewPortPSNRParam.viewPortSettingsList.size(); i++) 
      {
        DynViewPortSettings* pDynVP = &(m_dynamicViewPortPSNRParam.viewPortSettingsList[i]);
        printf("Dyanmic viewport %d, hFOV:%.2f, vFOV:%.2f\n", i, pDynVP->hFOV, pDynVP->vFOV);
        printf("Start viewport setting(POC_%d): %.2f %.2f; End viewport setting(POC_%d): %.2f %.2f\n", pDynVP->iPOC[0], pDynVP->fYaw[0], pDynVP->fPitch[0], pDynVP->iPOC[1], pDynVP->fYaw[1], pDynVP->fPitch[1]);        
      }
    }
    else
    {
      printf("Dynamic ViewPort PSNR calculation is not enabled!\n");
    }
#endif
#if SVIDEO_CF_SPSNR_NN
    if(m_bCFSPSNRNNEnabled)
      printf("Cross-format S-PSNR-NN is enabled; SphFile file: %s\n", m_sphFilename.empty() ? "NULL" : m_sphFilename.c_str());
#endif
#if SVIDEO_CF_SPSNR_I
    if(m_bCFSPSNRIEnabled)
      printf("Cross-format S-PSNR-I is enabled; SphFile file: %s\n", m_sphFilename.empty() ? "NULL" : m_sphFilename.c_str());
#endif
#if SVIDEO_CF_CPPPSNR
    if(m_bCFCPPPSNREnabled)
      printf("Cross-format CPP-PSNR is enabled\n");
#endif
#if SVIDEO_ROT_FIX
    printf("Rotation in 1/100 degrees: (yaw:%d  pitch:%d  roll:%d)\n", m_codingSVideoInfo.sVideoRotation.degree[2], m_codingSVideoInfo.sVideoRotation.degree[1], m_codingSVideoInfo.sVideoRotation.degree[0]); 
#endif
  }
  printf("-----360 video parameters----\n");
}

Bool TExt360AppEncCfg::isGeoConvertSkipped()
{
  return (   (m_sourceSVideoInfo.geoType==m_codingSVideoInfo.geoType)
          && (m_sourceSVideoInfo.iFaceHeight==m_codingSVideoInfo.iFaceHeight)
          && (m_sourceSVideoInfo.iFaceWidth==m_codingSVideoInfo.iFaceWidth)
          && (m_cfg.m_chromaFormatIDC==m_cfg.m_InputChromaFormatIDC)
          && (!memcmp(&(m_sourceSVideoInfo.framePackStruct), &(m_codingSVideoInfo.framePackStruct), sizeof(SVideoFPStruct)))
          && (m_sourceSVideoInfo.iCompactFPStructure == m_codingSVideoInfo.iCompactFPStructure)
#if SVIDEO_ROT_FIX
          && (!m_codingSVideoInfo.sVideoRotation.degree[0] && !m_codingSVideoInfo.sVideoRotation.degree[1] && !m_codingSVideoInfo.sVideoRotation.degree[2])
#endif
         );
};

Bool TExt360AppEncCfg::isDirectFPConvert()
{
  return (  ((m_sourceSVideoInfo.geoType == SVIDEO_OCTAHEDRON && m_codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON) || (m_sourceSVideoInfo.geoType == SVIDEO_ICOSAHEDRON && m_codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON))
          && (m_sourceSVideoInfo.iFaceHeight == m_codingSVideoInfo.iFaceHeight)
          && (m_sourceSVideoInfo.iFaceWidth  == m_codingSVideoInfo.iFaceWidth)
          && (m_cfg.m_chromaFormatIDC              == m_cfg.m_InputChromaFormatIDC)
          && (!memcmp(&(m_sourceSVideoInfo.framePackStruct), &(m_codingSVideoInfo.framePackStruct), sizeof(SVideoFPStruct)))
          && (m_sourceSVideoInfo.iCompactFPStructure != m_codingSVideoInfo.iCompactFPStructure)
#if SVIDEO_ROT_FIX
          && (!m_codingSVideoInfo.sVideoRotation.degree[0] && !m_codingSVideoInfo.sVideoRotation.degree[1] && !m_codingSVideoInfo.sVideoRotation.degree[2])
#endif
         );
};

Void TExt360AppEncCfg::setMaxCUInfo(UInt   uiCTUSize, UInt minCuSize)
{
  m_cfg.m_uiMaxCUWidth = m_cfg.m_uiMaxCUHeight = uiCTUSize;
  UInt uiMaxCUDepth = 0;
  while( (uiCTUSize>>uiMaxCUDepth) > minCuSize )
  {
    uiMaxCUDepth++;
  }
  m_cfg.m_uiMaxCUDepth = uiMaxCUDepth;
}