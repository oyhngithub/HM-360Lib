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

/** \file     TSPSNRMetricCalc.cpp
    \brief    SPSNRMetric class
*/

#include "TSPSNRMetricCalc.h"

// ====================================================================================================================
// DIY
#include <sstream>
#include <fstream>
#include <iostream>
// ====================================================================================================================

#if SVIDEO_SPSNR_NN

TSPSNRMetric::TSPSNRMetric()
: m_bSPSNREnabled(false)
, m_pCart2D(NULL)
, m_fpTable(NULL)
#if SVIDEO_CF_SPSNR_NN
, m_pcCodingGeometry(NULL)
, m_pcRefGeometry(NULL)
, m_pSamplePosTable(NULL)
, m_pSamplePosRecTable(NULL)
, m_pSamplePosCTable(NULL)
, m_pSamplePosCRecTable(NULL)
#endif
{
  m_dSPSNR[0] = m_dSPSNR[1] = m_dSPSNR[2] = 0;
}

TSPSNRMetric::~TSPSNRMetric()
{
  if(m_pCart2D)
  {
    free(m_pCart2D); m_pCart2D = NULL;
  }
  if (m_fpTable)
  {
    free(m_fpTable); m_fpTable = NULL;
  }
#if SVIDEO_CF_SPSNR_NN
  if(m_pSamplePosTable)
  {
    free(m_pSamplePosTable); m_pSamplePosTable = NULL;
  }
  if (m_pSamplePosRecTable)
  {
    free(m_pSamplePosRecTable); m_pSamplePosRecTable = NULL;
  }
  if (m_pSamplePosCTable)
  {
    free(m_pSamplePosCTable); m_pSamplePosCTable = NULL;
  }
  if (m_pSamplePosCRecTable)
  {
    free(m_pSamplePosCRecTable); m_pSamplePosCRecTable = NULL;
  }
  if(m_pcCodingGeometry)
  {
    delete m_pcCodingGeometry;
    m_pcCodingGeometry = NULL;
  }
  if(m_pcRefGeometry)
  {
    delete m_pcRefGeometry;
    m_pcRefGeometry = NULL;
  }
#endif
}

Void TSPSNRMetric::setOutputBitDepth(Int iOutputBitDepth[MAX_NUM_CHANNEL_TYPE])
{
  for(Int i = 0; i < MAX_NUM_CHANNEL_TYPE; i++)
  {
    m_outputBitDepth[i] = iOutputBitDepth[i];
  }
}

Void TSPSNRMetric::setReferenceBitDepth(Int iReferenceBitDepth[MAX_NUM_CHANNEL_TYPE])
{
  for(Int i = 0; i < MAX_NUM_CHANNEL_TYPE; i++)
  {
    m_referenceBitDepth[i] = iReferenceBitDepth[i];
  }
}

Void TSPSNRMetric::sphSampoints(const std::string &cSphDataFile)
{
  if(cSphDataFile.empty())
  {
    m_bSPSNREnabled = false;
    return;
  }

  // read data
  FILE *fp = fopen(cSphDataFile.c_str(),"r");
  if(!fp)
  {
    printf("SPSNR-NN is disabled because metadata file (%s) cannot be opened!\n", cSphDataFile.c_str());
    m_bSPSNREnabled = false;
    return;
  }

  if(fscanf(fp, "%d ", &m_iSphNumPoints)!=1)
  { 
    printf("SphData file does not exist.\n"); 
    exit(EXIT_FAILURE); 
    fclose(fp);
  }

  m_pCart2D = (CPos2D*)malloc(sizeof(CPos2D)*(m_iSphNumPoints));
  memset(m_pCart2D,0,sizeof(CPos2D)*(m_iSphNumPoints));

  for(Int z = 0; z < m_iSphNumPoints; z++)
  {
    if(fscanf(fp, "%lf %lf", &m_pCart2D[z].x, &m_pCart2D[z].y)!=2)
    { 
      printf("Format error SphData in sphSampoints().\n"); 
      exit(EXIT_FAILURE); 
    }
  }
  fclose(fp);
}

void TSPSNRMetric::sphToCart(CPos2D* sph, CPos3D* out)
{
  POSType fLat = (POSType)(sph->x*S_PI/180.0);
  POSType fLon = (POSType)(sph->y*S_PI/180.0);

  out->x =  ssin(fLon) * scos(fLat);
  out->y =               ssin(fLat);
  out->z = -scos(fLon) * scos(fLat);
}

void TSPSNRMetric::createTable(TGeometry *pcCodingGeomtry)
{
  if(!m_bSPSNREnabled)
  {
    return;
  }

  Int iNumPoints = m_iSphNumPoints;
  CPos2D In2d;
  CPos3D Out3d;
  SPos posIn, posOut;
  m_fpTable  = (IPos2D*)malloc(iNumPoints*sizeof(IPos2D));

  for (Int np=0; np < iNumPoints; np++)
  {
    In2d.x=m_pCart2D[np].x;
    In2d.y=m_pCart2D[np].y;

    //get cartesian coordinates
    sphToCart(&In2d, &Out3d);
    posIn.x = Out3d.x; posIn.y = Out3d.y; posIn.z = Out3d.z;
    pcCodingGeomtry->map3DTo2D(&posIn, &posOut);

    posOut.x = (POSType)TGeometry::round(posOut.x);
    posOut.y = (POSType)TGeometry::round(posOut.y);
    IPos tmpPos;
    tmpPos.faceIdx = posOut.faceIdx;
    tmpPos.u = (Int)(posOut.x);
    tmpPos.v = (Int)(posOut.y);
    pcCodingGeomtry->clamp(&tmpPos);
    pcCodingGeomtry->geoToFramePack(&tmpPos, &m_fpTable[np]);
  }
  // ====================================================================================================================
  // DIY
  //std::ofstream outfile("CountData.txt", std::ios::out | std::ios::binary);
  //if (!outfile.is_open())
  //{
	 // std::cout << " the file open fail" << std::endl;
	 // exit(1);
  //}
  //point2DCount countVector = pcCodingGeomtry->getPoint2DCount();
  //for (int i = 0; i < countVector.size(); ++i)
  //{
	 // for (int j = 0; j < countVector[i].size(); ++j)
	 // {
		//  outfile << countVector[i][j] << " ";
	 // }
	 // outfile << "\r\n";
  //}
  //outfile.close();

  // ====================================================================================================================
}

Void TSPSNRMetric::xCalculateSPSNR( TComPicYuv* pcOrgPicYuv, TComPicYuv* pcPicD )
{
  Int iNumPoints = m_iSphNumPoints;
  Int iBitDepthForPSNRCalc[MAX_NUM_CHANNEL_TYPE];
  Int iReferenceBitShift[MAX_NUM_CHANNEL_TYPE];
  Int iOutputBitShift[MAX_NUM_CHANNEL_TYPE];
  iBitDepthForPSNRCalc[CHANNEL_TYPE_LUMA] = std::max(m_outputBitDepth[CHANNEL_TYPE_LUMA], m_referenceBitDepth[CHANNEL_TYPE_LUMA]);
  iBitDepthForPSNRCalc[CHANNEL_TYPE_CHROMA] = std::max(m_outputBitDepth[CHANNEL_TYPE_CHROMA], m_referenceBitDepth[CHANNEL_TYPE_CHROMA]);
  iReferenceBitShift[CHANNEL_TYPE_LUMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_LUMA] - m_referenceBitDepth[CHANNEL_TYPE_LUMA];
  iReferenceBitShift[CHANNEL_TYPE_CHROMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_CHROMA] - m_referenceBitDepth[CHANNEL_TYPE_CHROMA];
  iOutputBitShift[CHANNEL_TYPE_LUMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_LUMA] - m_outputBitDepth[CHANNEL_TYPE_LUMA];
  iOutputBitShift[CHANNEL_TYPE_CHROMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_CHROMA] - m_outputBitDepth[CHANNEL_TYPE_CHROMA];

  memset(m_dSPSNR, 0, sizeof(Double)*3);
  TComPicYuv &picd=*pcPicD;
  Double SSDspsnr[3]={0, 0 ,0};

  for(Int chan=0; chan<pcPicD->getNumberValidComponents(); chan++)
  {
    const ComponentID ch=ComponentID(chan);
    const Pel*  pOrg       = pcOrgPicYuv->getAddr(ch);
    const Int   iOrgStride = pcOrgPicYuv->getStride(ch);
    const Pel*  pRec       = picd.getAddr(ch);
    const Int   iRecStride = picd.getStride(ch);

    for (Int np = 0; np < iNumPoints; np++)
    {
      if (!chan)
      {
        Int x_loc=(Int)(m_fpTable[np].x);
        Int y_loc=(Int)(m_fpTable[np].y);
        Intermediate_Int iDifflp=  (pOrg[x_loc+(y_loc*iOrgStride)]<<iReferenceBitShift[toChannelType(ch)]) - (pRec[x_loc+(y_loc*iRecStride)]<<iOutputBitShift[toChannelType(ch)]);
        SSDspsnr[chan]+= iDifflp*iDifflp;
      }
      else
      {
        Int x_loc = Int(m_fpTable[np].x >> pcPicD->getComponentScaleX(COMPONENT_Cb));
        Int y_loc = Int(m_fpTable[np].y >> pcPicD->getComponentScaleY(COMPONENT_Cb));
        Intermediate_Int iDifflp=  (pOrg[x_loc+(y_loc*iOrgStride)]<<iReferenceBitShift[toChannelType(ch)]) - (pRec[x_loc+(y_loc*iRecStride)]<<iOutputBitShift[toChannelType(ch)]);
        SSDspsnr[chan]+= iDifflp*iDifflp;
      }
    }
  }

  for (Int ch_indx = 0; ch_indx < pcPicD->getNumberValidComponents(); ch_indx++)
  {
    const ComponentID ch=ComponentID(ch_indx);
    const Int maxval = 255<<(iBitDepthForPSNRCalc[toChannelType(ch)]-8) ;

    Double fReflpsnr = Double(iNumPoints)*maxval*maxval;
    m_dSPSNR[ch_indx] = ( SSDspsnr[ch_indx] ? 10.0 * log10( fReflpsnr / (Double)SSDspsnr[ch_indx] ) : 999.99 );
  }
}

#if SVIDEO_CF_SPSNR_NN
Void TSPSNRMetric::initCFSPSNR(SVideoInfo& referenceVideoInfo, SVideoInfo& codingvideoInfo, InputGeoParam& geoParam)
{
  m_refVideoInfo = referenceVideoInfo;
  m_codingVideoInfo = codingvideoInfo;
  m_geoParam = geoParam;

  m_pcCodingGeometry    = TGeometry::create(m_codingVideoInfo, &m_geoParam);
  m_pcRefGeometry       = TGeometry::create(m_refVideoInfo, &m_geoParam);
}

#if SVIDEO_CF_SPSNR_NN_ENH
//use original first, and interpolate in rec;
Void TSPSNRMetric::createTableCFSPSNR(UInt uiXScale, UInt uiYScale)
{
  if(!m_bSPSNREnabled)
  {
    return;
  }

  Int iNumPoints = m_iSphNumPoints;
  CPos2D In2d;
  CPos3D Out3d;
  SPos posIn, posOut;
  m_pSamplePosTable  = (SPos2D*)malloc(iNumPoints*sizeof(SPos2D));
  m_pSamplePosRecTable  = (SPos2D*)malloc(iNumPoints*sizeof(SPos2D));
  m_pSamplePosCTable  = (SPos2D*)malloc(iNumPoints*sizeof(SPos2D));
  m_pSamplePosCRecTable  = (SPos2D*)malloc(iNumPoints*sizeof(SPos2D));
  TGeometry *pcCodingGeometry    = m_pcCodingGeometry;
  TGeometry *pcRefGeometry       = m_pcRefGeometry;

  for (Int np=0; np < iNumPoints; np++)
  {
    In2d.x=m_pCart2D[np].x;  // longitude
    In2d.y=m_pCart2D[np].y;  // latitude

    //get cartesian coordinates
    sphToCart(&In2d, &Out3d);  // 3D

    SPos  sCodingPos, sTempPos, sRefPos;
    sTempPos.x = Out3d.x;
    sTempPos.y = Out3d.y;
    sTempPos.z = Out3d.z;

    pcRefGeometry->map3DTo2D(&sTempPos, &sCodingPos);
    sCodingPos.x = TGeometry::round(sCodingPos.x);
    sCodingPos.y = TGeometry::round(sCodingPos.y);
    pcRefGeometry->map2DTo3D(sCodingPos, &sTempPos);
#if SVIDEO_ROT_FIX
    pcCodingGeometry->invRotate3D(sTempPos, -pcCodingGeometry->getSVideoInfo()->sVideoRotation.degree[0], -pcCodingGeometry->getSVideoInfo()->sVideoRotation.degree[1], -pcCodingGeometry->getSVideoInfo()->sVideoRotation.degree[2]);
#endif
    pcCodingGeometry->map3DTo2D(&sTempPos, &sRefPos);
    //Luma;
    m_pSamplePosTable[np].faceIdx = sCodingPos.faceIdx;
    m_pSamplePosTable[np].x = TGeometry::round(sCodingPos.x);
    m_pSamplePosTable[np].y = TGeometry::round(sCodingPos.y);
    m_pSamplePosRecTable[np].faceIdx = sRefPos.faceIdx;
    m_pSamplePosRecTable[np].x = (sRefPos.x);
    m_pSamplePosRecTable[np].y = (sRefPos.y);

    //Chroma;
    sCodingPos.x = TGeometry::round(sCodingPos.x / (1<<uiXScale)) <<uiXScale;
    sCodingPos.y = TGeometry::round(sCodingPos.y / (1<<uiYScale)) <<uiYScale;
    pcRefGeometry->map2DTo3D(sCodingPos, &sTempPos);
    sCodingPos.x = (sCodingPos.x / (1<<uiXScale));
    sCodingPos.y = (sCodingPos.y / (1<<uiYScale));
#if SVIDEO_ROT_FIX
    pcCodingGeometry->invRotate3D(sTempPos, -pcCodingGeometry->getSVideoInfo()->sVideoRotation.degree[0], -pcCodingGeometry->getSVideoInfo()->sVideoRotation.degree[1], -pcCodingGeometry->getSVideoInfo()->sVideoRotation.degree[2]);
#endif
    pcCodingGeometry->map3DTo2D(&sTempPos, &sRefPos);
    sRefPos.x = (sRefPos.x / (1<<uiXScale));
    sRefPos.y = (sRefPos.y / (1<<uiYScale));

    m_pSamplePosCTable[np].faceIdx = sCodingPos.faceIdx;
    m_pSamplePosCTable[np].x = TGeometry::round(sCodingPos.x);
    m_pSamplePosCTable[np].y = TGeometry::round(sCodingPos.y);
    m_pSamplePosCRecTable[np].faceIdx = sRefPos.faceIdx;
    m_pSamplePosCRecTable[np].x = (sRefPos.x);
    m_pSamplePosCRecTable[np].y = (sRefPos.y);
  } 
}
#else
//use rec first;
Void TSPSNRMetric::createTableCFSPSNR(UInt uiXScale, UInt uiYScale)
{
  if(!m_bSPSNREnabled)
  {
    return;
  }

  Int iNumPoints = m_iSphNumPoints;
  CPos2D In2d;
  CPos3D Out3d;
  SPos posIn, posOut;
  m_pSamplePosTable  = (IPos*)malloc(iNumPoints*sizeof(IPos));
  m_pSamplePosRecTable  = (IPos*)malloc(iNumPoints*sizeof(IPos));
  m_pSamplePosCTable  = (IPos*)malloc(iNumPoints*sizeof(IPos));
  m_pSamplePosCRecTable  = (IPos*)malloc(iNumPoints*sizeof(IPos));
  TGeometry *pcCodingGeometry    = m_pcCodingGeometry;
  TGeometry *pcRefGeometry       = m_pcRefGeometry;

  for (Int np=0; np < iNumPoints; np++)
  {
    In2d.x=m_pCart2D[np].x;  // longitude
    In2d.y=m_pCart2D[np].y;  // latitude

    //get cartesian coordinates
    sphToCart(&In2d, &Out3d);  // 3D

    SPos  sCodingPos, sTempPos, sRefPos;
    sTempPos.x = Out3d.x;
    sTempPos.y = Out3d.y;
    sTempPos.z = Out3d.z;
    pcCodingGeometry->map3DTo2D(&sTempPos, &sCodingPos);
    sCodingPos.x = round(sCodingPos.x);
    sCodingPos.y = round(sCodingPos.y);
    pcCodingGeometry->map2DTo3D(sCodingPos, &sTempPos);
    pcRefGeometry->map3DTo2D(&sTempPos, &sRefPos);
    //Luma;
    m_pSamplePosTable[np].faceIdx = sRefPos.faceIdx;
    m_pSamplePosTable[np].u = round(sRefPos.x);
    m_pSamplePosTable[np].v = round(sRefPos.y);
    m_pSamplePosRecTable[np].faceIdx = sCodingPos.faceIdx;
    m_pSamplePosRecTable[np].u = round(sCodingPos.x);
    m_pSamplePosRecTable[np].v = round(sCodingPos.y);

    //Chroma;
    sCodingPos.x = round(sCodingPos.x / (1<<uiXScale)) <<uiXScale;
    sCodingPos.y = round(sCodingPos.y / (1<<uiYScale)) <<uiYScale;
    pcCodingGeometry->map2DTo3D(sCodingPos, &sTempPos);
    sCodingPos.x = (sCodingPos.x / (1<<uiXScale));
    sCodingPos.y = (sCodingPos.y / (1<<uiYScale));
    pcRefGeometry->map3DTo2D(&sTempPos, &sRefPos);
    sRefPos.x = (sRefPos.x / (1<<uiXScale));
    sRefPos.y = (sRefPos.y / (1<<uiYScale));

    m_pSamplePosCTable[np].faceIdx = sRefPos.faceIdx;
    m_pSamplePosCTable[np].u = round(sRefPos.x);
    m_pSamplePosCTable[np].v = round(sRefPos.y);
    m_pSamplePosCRecTable[np].faceIdx = sCodingPos.faceIdx;
    m_pSamplePosCRecTable[np].u = round(sCodingPos.x);
    m_pSamplePosCRecTable[np].v = round(sCodingPos.y);
  } 
}
#endif

Void TSPSNRMetric::xCalculateCFSPSNR( TComPicYuv *pcOrigPicYuv, TComPicYuv* pcRecPicYuv)
{
  Int iNumPoints = m_iSphNumPoints;
  Int iBitDepthForPSNRCalc[MAX_NUM_CHANNEL_TYPE];
  Int iReferenceBitShift[MAX_NUM_CHANNEL_TYPE];
  Int iOutputBitShift[MAX_NUM_CHANNEL_TYPE];

  TGeometry  *pcCodingGeometry;
  TGeometry  *pcRefGeometry;

  Double SSDSPSNR[3]={0, 0 ,0};

  iBitDepthForPSNRCalc[CHANNEL_TYPE_LUMA]   = std::max(m_outputBitDepth[CHANNEL_TYPE_LUMA], m_referenceBitDepth[CHANNEL_TYPE_LUMA]);
  iBitDepthForPSNRCalc[CHANNEL_TYPE_CHROMA] = std::max(m_outputBitDepth[CHANNEL_TYPE_CHROMA], m_referenceBitDepth[CHANNEL_TYPE_CHROMA]);
  iReferenceBitShift[CHANNEL_TYPE_LUMA]     = iBitDepthForPSNRCalc[CHANNEL_TYPE_LUMA] - m_referenceBitDepth[CHANNEL_TYPE_LUMA];
  iReferenceBitShift[CHANNEL_TYPE_CHROMA]   = iBitDepthForPSNRCalc[CHANNEL_TYPE_CHROMA] - m_referenceBitDepth[CHANNEL_TYPE_CHROMA];
  iOutputBitShift[CHANNEL_TYPE_LUMA]        = iBitDepthForPSNRCalc[CHANNEL_TYPE_LUMA] - m_outputBitDepth[CHANNEL_TYPE_LUMA];
  iOutputBitShift[CHANNEL_TYPE_CHROMA]      = iBitDepthForPSNRCalc[CHANNEL_TYPE_CHROMA] - m_outputBitDepth[CHANNEL_TYPE_CHROMA];

  memset(m_dSPSNR, 0, sizeof(Double)*3);

  pcCodingGeometry = m_pcCodingGeometry;
  pcRefGeometry = m_pcRefGeometry;

  if((pcCodingGeometry->getSVideoInfo()->geoType == SVIDEO_OCTAHEDRON || pcCodingGeometry->getSVideoInfo()->geoType == SVIDEO_ICOSAHEDRON) && pcCodingGeometry->getSVideoInfo()->iCompactFPStructure) 
  {
    pcCodingGeometry->compactFramePackConvertYuv(pcRecPicYuv);
  }
  else
  {
    pcCodingGeometry->convertYuv(pcRecPicYuv);
  }
  pcCodingGeometry->spherePadding(true);

  if((pcRefGeometry->getSVideoInfo()->geoType == SVIDEO_OCTAHEDRON || pcRefGeometry->getSVideoInfo()->geoType == SVIDEO_ICOSAHEDRON) && pcRefGeometry->getSVideoInfo()->iCompactFPStructure) 
  {
    pcRefGeometry->compactFramePackConvertYuv(pcOrigPicYuv);
  }
  else
  {
    pcRefGeometry->convertYuv(pcOrigPicYuv);
  }
  pcRefGeometry->spherePadding(true);
  
  for(Int chan=0; chan<pcRecPicYuv->getNumberValidComponents(); chan++)
  {
    const ComponentID ch=ComponentID(chan);
#if SVIDEO_CF_SPSNR_NN_ENH
    SPos2D* pSamplePosRef = chan==0? m_pSamplePosTable : m_pSamplePosCTable;
    SPos2D* pSamplePosRec = chan==0? m_pSamplePosRecTable : m_pSamplePosCRecTable;
#else
    IPos* pSamplePosRef = chan==0? m_pSamplePosTable : m_pSamplePosCTable;
    IPos* pSamplePosRec = chan==0? m_pSamplePosRecTable : m_pSamplePosCRecTable;
#endif    
    for (Int np = 0; np < iNumPoints; np++)
    {
      SPos sCodingPos, sRefPos;

      sRefPos.faceIdx = pSamplePosRef[np].faceIdx;
#if SVIDEO_CF_SPSNR_NN_ENH
      sRefPos.x = pSamplePosRef[np].x;
      sRefPos.y = pSamplePosRef[np].y;
#else
      sRefPos.x = pSamplePosRef[np].u;
      sRefPos.y = pSamplePosRef[np].v;
#endif
      sRefPos.z = 0;
      Pel refPel    = pcRefGeometry->getPelValue(ch, sRefPos);

      sCodingPos.faceIdx = pSamplePosRec[np].faceIdx;
#if SVIDEO_CF_SPSNR_NN_ENH
      sCodingPos.x = pSamplePosRec[np].x;
      sCodingPos.y = pSamplePosRec[np].y;
#else
      sCodingPos.x = pSamplePosRec[np].u;
      sCodingPos.y = pSamplePosRec[np].v;
#endif
      sCodingPos.z = 0;
      Pel codingPel = pcCodingGeometry->getPelValue(ch, sCodingPos);      

      Intermediate_Int iDifflp =  (Intermediate_Int)((refPel<<iReferenceBitShift[toChannelType(ch)]) - (codingPel<<iOutputBitShift[toChannelType(ch)]) );
      SSDSPSNR[chan] += iDifflp*iDifflp;
    }
  }

  for (Int ch_indx = 0; ch_indx < pcRecPicYuv->getNumberValidComponents(); ch_indx++)
  {
    const ComponentID ch=ComponentID(ch_indx);
    const Int maxval    = 255<<(iBitDepthForPSNRCalc[toChannelType(ch)]-8) ;

    Double fReflpsnr   = Double(iNumPoints)*maxval*maxval;
    m_dSPSNR[ch_indx] = ( SSDSPSNR[ch_indx] ? 10.0 * log10( fReflpsnr / (Double)SSDSPSNR[ch_indx] ) : 999.99 );
  }
}

#endif

#endif