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


#include "TExt360EncAnalyze.h"

TExt360EncAnalyze::TExt360EncAnalyze()
{
  clear();
}

TExt360EncAnalyze::~TExt360EncAnalyze()
{
}

Void TExt360EncAnalyze::clear()
{
#if SVIDEO_SPSNR_NN
    m_bSPSNREnabled = false;
#if SVIDEO_CODEC_SPSNR_NN
    m_bCodecSPSNREnabled = false;
#endif
#endif
#if SVIDEO_WSPSNR
    m_bWSPSNREnabled = false;
#if SVIDEO_WSPSNR_E2E
    m_bE2EWSPSNREnabled = false;
#endif
#endif
#if SVIDEO_SPSNR_I
    m_bSPSNRIEnabled = false;
#endif
#if SVIDEO_CPPPSNR
    m_bCPPPSNREnabled = false;
#endif
#if SVIDEO_VIEWPORT_PSNR
    m_bViewPortPSNREnabled = false;
#endif
#if SVIDEO_CF_SPSNR_NN
    m_bCFSPSNREnabled = false;
#endif
#if SVIDEO_CF_SPSNR_I
    m_bCFSPSNRIEnabled = false;
#endif
#if SVIDEO_CF_CPPPSNR
    m_bCFCPPPSNREnabled = false;
#endif
#if SVIDEO_DYNAMIC_VIEWPORT_PSNR
    m_bDynamicViewPortPSNREnabled = false;
#endif

#if SVIDEO_SPSNR_NN
  m_SPSNRSum.clear();
#if SVIDEO_CODEC_SPSNR_NN
  m_CodecSPSNRSum.clear();
#endif
#endif
#if SVIDEO_WSPSNR
  m_WSPSNRSum.clear();
#if SVIDEO_WSPSNR_E2E
  m_E2EWSPSNRSum.clear();
#endif
#endif
#if SVIDEO_SPSNR_I
  m_SPSNRISum.clear();
#endif
#if SVIDEO_CPPPSNR
  m_CPPPSNRSum.clear();
#endif
#if SVIDEO_VIEWPORT_PSNR
  for(std::size_t i=0; i<m_pdViewPortPSNRSum.size(); i++)
  {
    m_pdViewPortPSNRSum[i].clear();
  }
#endif
#if SVIDEO_CF_SPSNR_NN
  m_CFSPSNRSum.clear();
#endif
#if SVIDEO_CF_SPSNR_I
  m_CFSPSNRISum.clear();
#endif
#if SVIDEO_CF_CPPPSNR
  m_CFCPPPSNRSum.clear();
#endif
#if SVIDEO_DYNAMIC_VIEWPORT_PSNR
  for(std::size_t i=0; i<m_pdDynamicViewPortPSNRSum.size(); i++)
  {
    m_pdDynamicViewPortPSNRSum[i].clear();
  }
#endif
}

Void TExt360EncAnalyze::printHeader()
{
#if SVIDEO_E2E_METRICS
#if SVIDEO_WSPSNR
  if(m_bWSPSNREnabled)
  {
    printf( " Y-WSPSNR  "  "U-WSPSNR  "  "V-WSPSNR   " );
  }
#endif
#endif
#if SVIDEO_SPSNR_NN
#if SVIDEO_CODEC_SPSNR_NN
  if(m_bCodecSPSNREnabled)
  {
    printf( " Y-C_SPSNR_NN  "  "U-C_SPSNR_NN  "  "V-C_SPSNR_NN " );
  }
#endif
  if(m_bSPSNREnabled)
  {
#if SVIDEO_E2E_METRICS
    printf( " Y-E2ESPSNR_NN "  "U-E2ESPSNR_NN "  "V-E2ESPSNR_NN " );
#else
    printf( " Y-SPSNR_NN "  "U-SPSNR_NN "  "V-SPSNR_NN " );
#endif
  }
#endif
#if !SVIDEO_E2E_METRICS
#if SVIDEO_WSPSNR
  if(m_bWSPSNREnabled)
  {
    printf( " Y-WSPSNR  "  "U-WSPSNR  "  "V-WSPSNR   " );
  }
#endif
#endif
#if SVIDEO_SPSNR_I
  if(m_bSPSNRIEnabled)
  {
#if SVIDEO_E2E_METRICS
    printf( " Y-E2ESPSNR_I  "  "U-E2ESPSNR_I  "  "V-E2ESPSNR_I  " );
#else
    printf( " Y-SPSNR_I  "  "U-SPSNR_I  "  "V-SPSNR_I  " );
#endif
  }
#endif
#if SVIDEO_CPPPSNR
  if(m_bCPPPSNREnabled)
  {
#if SVIDEO_E2E_METRICS
    printf( " Y-E2ECPPPSNR  "  "U-E2ECPPPSNR  "  "V-E2ECPPPSNR  " );
#else
    printf( " Y-CPPPSNR  "  "U-CPPPSNR  "  "V-CPPPSNR  " );
#endif
  }
#endif
#if SVIDEO_WSPSNR_E2E
  if(m_bE2EWSPSNREnabled)
  {
    printf( " Y-E2EWSPSNR "  "U-E2EWSPSNR "  "V-E2EWSPSNR " );
  }
#endif
#if SVIDEO_VIEWPORT_PSNR
  if(m_bViewPortPSNREnabled)
  {
    for(Int i=0; i<m_pdViewPortPSNRSum.size(); i++)
    {
      printf( " Y-PSNR_VP%d "  "U-PSNR_VP%d "  "V-PSNR_VP%d ", i, i, i );
    }
  }
#endif
#if SVIDEO_DYNAMIC_VIEWPORT_PSNR
  if(m_bDynamicViewPortPSNREnabled)
  {
    for(Int i=0; i<m_pdDynamicViewPortPSNRSum.size(); i++)
      printf( " Y-PSNR_DYN_VP%d "  "U-PSNR_DYN_VP%d "  "V-PSNR_DYN_VP%d ", i, i, i);
  }
#endif
#if SVIDEO_CF_SPSNR_NN
  if(m_bCFSPSNREnabled)
  {
    printf( " Y-CFSPSNR_NN "  "U-CFSPSNR_NN "  "V-CFSPSNR_NN " );
  }
#endif
#if SVIDEO_CF_SPSNR_I
  if(m_bCFSPSNRIEnabled)
  {
    printf( " Y-CFSPSNR_I "  "U-CFSPSNR_I "  "V-CFSPSNR_I " );
  }
#endif
#if SVIDEO_CF_CPPPSNR
  if(m_bCFCPPPSNREnabled)
  {
    printf( " Y-CFCPPPSNR  "  "U-CFCPPPSNR  "  "V-CFCPPPSNR  " );
  }
#endif
}

Void TExt360EncAnalyze::printPSNRs(const UInt numPic)
{
#if SVIDEO_E2E_METRICS
#if SVIDEO_WSPSNR
  if(m_bWSPSNREnabled)
  {
    printf( " %8.4lf  "   "%8.4lf  "    "%8.4lf   ",
            getWSpsnr(COMPONENT_Y ) / (Double)numPic,
            getWSpsnr(COMPONENT_Cb ) / (Double)numPic,
            getWSpsnr(COMPONENT_Cr )  / (Double)numPic
          );
  } 
#endif
#endif
#if SVIDEO_SPSNR_NN
#if SVIDEO_CODEC_SPSNR_NN
  if(m_bCodecSPSNREnabled)
  {
    printf( " %8.4lf      "   "%8.4lf      "    "%8.4lf     ",
            getCodecSpsnr(COMPONENT_Y ) / (Double)numPic,
            getCodecSpsnr(COMPONENT_Cb ) / (Double)numPic,
            getCodecSpsnr(COMPONENT_Cr )  / (Double)numPic
          );
  }
#endif
  if(m_bSPSNREnabled)
  {
    printf( " %8.4lf      "   "%8.4lf      "    "%8.4lf     ",
            getSpsnr(COMPONENT_Y ) / (Double)numPic,
            getSpsnr(COMPONENT_Cb ) / (Double)numPic,
            getSpsnr(COMPONENT_Cr )  / (Double)numPic
          );
  }
#endif
#if !SVIDEO_E2E_METRICS
#if SVIDEO_WSPSNR
  if(m_bWSPSNREnabled)
  {
    printf( "  %8.4lf  "   "%8.4lf  "    "%8.4lf  ",
            getWSpsnr(COMPONENT_Y ) / (Double)numPic,
            getWSpsnr(COMPONENT_Cb ) / (Double)numPic,
            getWSpsnr(COMPONENT_Cr )  / (Double)numPic
          );
  }
#endif
#endif
#if SVIDEO_SPSNR_I
  if(m_bSPSNRIEnabled)
  {
    printf( "  %8.4lf      "   "%8.4lf      "    "%8.4lf     ",
            getSpsnrI(COMPONENT_Y )  / (Double)numPic,
            getSpsnrI(COMPONENT_Cb ) / (Double)numPic,
            getSpsnrI(COMPONENT_Cr ) / (Double)numPic
          );
  }
#endif
#if SVIDEO_CPPPSNR
  if(m_bCPPPSNREnabled)
  {
    printf( "  %8.4lf      "   "%8.4lf      "    "%8.4lf     ",
            getCPPpsnr(COMPONENT_Y )  / (Double)numPic,
            getCPPpsnr(COMPONENT_Cb ) / (Double)numPic,
            getCPPpsnr(COMPONENT_Cr ) / (Double)numPic
          );
  }
#endif
#if SVIDEO_WSPSNR_E2E
  if(m_bE2EWSPSNREnabled)
  {
    printf( "  %8.4lf    "   "%8.4lf    "    "%8.4lf ",
            getE2EWSpsnr(COMPONENT_Y ) / (Double)numPic,
            getE2EWSpsnr(COMPONENT_Cb ) / (Double)numPic,
            getE2EWSpsnr(COMPONENT_Cr )  / (Double)numPic
          );
  }
#endif
#if SVIDEO_VIEWPORT_PSNR
  if(m_bViewPortPSNREnabled)
  {
    for(std::size_t i=0; i<m_pdViewPortPSNRSum.size(); i++)
    {
      printf( "    %8.4lf   "   "%8.4lf   "    "%8.4lf",
              getViewPortPsnr((UInt)i, COMPONENT_Y ) / (Double)numPic,
              getViewPortPsnr((UInt)i, COMPONENT_Cb ) / (Double)numPic,
              getViewPortPsnr((UInt)i, COMPONENT_Cr )  / (Double)numPic
            );
    }
  }
#endif
#if SVIDEO_DYNAMIC_VIEWPORT_PSNR
  if(m_bDynamicViewPortPSNREnabled)
  {
    for(Int i=0; i<m_pdDynamicViewPortPSNRSum.size(); i++)
      printf( "    %8.4lf       "   "%8.4lf       "    "%8.4lf    ",
              getDynamicViewPortPsnr(i, COMPONENT_Y ) / (Double)numPic,
              getDynamicViewPortPsnr(i, COMPONENT_Cb ) / (Double)numPic,
              getDynamicViewPortPsnr(i, COMPONENT_Cr )  / (Double)numPic
            );
  }
#endif
#if SVIDEO_CF_SPSNR_NN
  if(m_bCFSPSNREnabled)
  {
    printf( "    %8.4lf     "   "%8.4lf     "    "%8.4lf  ",
            getCFSpsnr(COMPONENT_Y ) / (Double)numPic,
            getCFSpsnr(COMPONENT_Cb ) / (Double)numPic,
            getCFSpsnr(COMPONENT_Cr )  / (Double)numPic
          );
  } 
#endif
#if SVIDEO_CF_SPSNR_I
  if(m_bCFSPSNRIEnabled)
  {
    printf( "    %8.4lf    "   "%8.4lf    "    "%8.4lf  ",
            getCFSpsnrI(COMPONENT_Y ) / (Double)numPic,
            getCFSpsnrI(COMPONENT_Cb ) / (Double)numPic,
            getCFSpsnrI(COMPONENT_Cr )  / (Double)numPic
          );
  } 
#endif
#if SVIDEO_CF_CPPPSNR
  if(m_bCFCPPPSNREnabled)
  {
    printf( "   %8.4lf     "   "%8.4lf     "    "%8.4lf     ",
            getCFCPPpsnr(COMPONENT_Y )  / (Double)numPic,
            getCFCPPpsnr(COMPONENT_Cb ) / (Double)numPic,
            getCFCPPpsnr(COMPONENT_Cr ) / (Double)numPic
          );
  } 
#endif
}
