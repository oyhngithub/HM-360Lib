#include <assert.h>
#include <math.h>
#include "TNewUniformMap.h"
#define PI 3.1415926

#if EXTENSION_360_VIDEO
TNewUniformMap::TNewUniformMap(SVideoInfo& sVideoInfo, InputGeoParam *pInGeoParam) : TGeometry()
{
	Bool bGeoTypeChecking = (sVideoInfo.geoType == SVIDEO_NEWUNIFORMMAP);

#if SVIDEO_CPPPSNR
	bGeoTypeChecking = bGeoTypeChecking || (sVideoInfo.geoType == SVIDEO_CRASTERSPARABOLIC);
#endif
#if SVIDEO_ADJUSTED_EQUALAREA
	bGeoTypeChecking = bGeoTypeChecking || (sVideoInfo.geoType == SVIDEO_ADJUSTEDEQUALAREA);
#else
	bGeoTypeChecking = bGeoTypeChecking || (sVideoInfo.geoType == SVIDEO_EQUALAREA);
#endif

	assert(bGeoTypeChecking);
	//assert(sVideoInfo.iNumFaces == 1);
	geoInit(sVideoInfo, pInGeoParam);
}

TNewUniformMap::~TNewUniformMap()
{
}


Void TNewUniformMap::map3DTo2D(SPos *pSPosIn, SPos *pSPosOut)//replaced
{
	POSType X = (pSPosIn->x);
	POSType Y = (pSPosIn->y);
	POSType Z = (pSPosIn->z); //É¾³ýÁËsfabs()
	pSPosOut->z = 0;
	pSPosOut->faceIdx = 0;
	POSType u, v, r, u1, v1;
	POSType yaw, pitch, x, y, x1, y1;

	yaw = atan2(-Z, X);
	double tmp = X * X + Y * Y + Z * Z;
	pitch = asin(Y / sqrt(X * X + Y * Y + Z * Z));
	//##calculate u, v in the disc
	if (pitch < 0) {//south sphere
		r = sqrt(1 + sin(pitch));
		u1 = r * cos(yaw); 
		v1 = r * sin(yaw);
		u1 *= -1;
	}
	else {//north sphere
		r = sqrt(1 - sin(pitch));
		u1 = cos(yaw) * r;
		v1 = sin(yaw) * r;
	}
	//##calculate x, y in the square
	if (u1 * u1 > v1 * v1) {
		float sgn = 1;
		if (u1 < 0) {
			sgn *= -1;
		}
		if (abs(u1) < 0.0001) {//The special case of v1 / u1.Change2
			x = y = 0;
		}
		else {
			x = sgn * sqrt(u1 * u1 + v1 * v1);
			y = sgn * sqrt(u1 * u1 + v1 * v1) * 4 / PI * atan(v1 / u1);
		}

	}
	else {
		float sgn = 1;
		if (v1 < 0) {
			sgn *= -1;
		}
		if (abs(v1) < 0.0001) {//The special case of u1 / v1.Change2
			x = y = 0;
		}
		else {
			x = sgn * sqrt(u1 * u1 + v1 * v1) * 4 / PI * atan(u1 / v1);
			y = sgn * sqrt(u1 * u1 + v1 * v1);
		}
	}
	if (pitch < 0) {//south sphere
		x1 = x + 3;
		y1 = y + 1;
	}
	else {
		x1 = x + 1;
		y1 = y + 1;
	}
	u = x1 / 4;
	v = y1 / 2;

	pSPosOut->x = u * m_sVideoInfo.iFaceWidth;//Change3,Bigchange , swap(u, v)
	pSPosOut->y = v * m_sVideoInfo.iFaceHeight;
	pSPosOut->x -= 0.5;
	pSPosOut->y -= 0.5;


}

Void TNewUniformMap::map2DTo3D(SPos& IPosIn, SPos *pSPosOut)
{
	POSType u, v, x, y;
	POSType yaw, pitch;
	POSType south = 1;

	u = (IPosIn.x + (POSType)(0.5)) / m_sVideoInfo.iFaceWidth;
	v = (IPosIn.y + (POSType)(0.5)) / m_sVideoInfo.iFaceHeight;

	//##square to big centralized square
	if (u > 0.5) //south sphere
	{
		south = -1;
		u = 4 * u - 3;
		v = 2 * v - 1;
		u = -u;
	}
	else //north sphere
	{
		u = 4 * u - 1;
		v = 2 * v - 1;
	}

	//##square to disc
	if (fabsf(u) >= fabsf(v))
	{
		yaw = PI * v / (4 * u);
		x = u * cos(yaw);
		y = u * sin(yaw);
	}
	else
	{
		yaw = PI * u / (4 * v);
		x = v * sin(yaw);
		y = v * cos(yaw);
	}

	yaw = atan2(y, x); //yaw: [-pi, pi]

					   //flip if south 
	pitch = south * asin(1 - (x*x + y*y)); //pitch: [-pi/2, pi/2]

	pSPosOut->faceIdx = IPosIn.faceIdx;

	pSPosOut->x = (POSType)(scos(pitch)*scos(yaw));
	pSPosOut->y = (POSType)(ssin(pitch));
	pSPosOut->z = -(POSType)(scos(pitch)*ssin(yaw));

}

Void TNewUniformMap::convertYuv(TComPicYuv *pSrcYuv)
{
	Int nWidth = m_sVideoInfo.iFaceWidth;
	Int nHeight = m_sVideoInfo.iFaceHeight;

#if !SVIDEO_ERP_PADDING
	assert(pSrcYuv->getWidth(ComponentID(0)) == nWidth && pSrcYuv->getHeight(ComponentID(0)) == nHeight);
	assert(pSrcYuv->getNumberValidComponents() == getNumChannels());
#endif

	if (pSrcYuv->getChromaFormat() == CHROMA_420)
	{
		//memory allocation;
		Int nMarginSizeTmpBuf = std::max(std::max(m_filterUps[0].nTaps, m_filterUps[1].nTaps), std::max(m_filterUps[2].nTaps, m_filterUps[3].nTaps)) >> 1; //2, depends on the chroma upsampling filter size;  
		assert(nMarginSizeTmpBuf <= std::min(pSrcYuv->getMarginX((ComponentID)1), pSrcYuv->getMarginY((ComponentID)1)));

		for (Int ch = 0; ch<pSrcYuv->getNumberValidComponents(); ch++)
		{
			ComponentID chId = ComponentID(ch);
			Int iStrideTmpBuf = pSrcYuv->getStride(chId);
			nWidth = m_sVideoInfo.iFaceWidth >> pSrcYuv->getComponentScaleX(chId);
			nHeight = m_sVideoInfo.iFaceHeight >> pSrcYuv->getComponentScaleY(chId);

			//fill;
			Pel *pSrc = pSrcYuv->getAddr(chId);
			Pel *pDst;
#if SVIDEO_ERP_PADDING
			Int iPadWidth_L = SVIDEO_ERP_PAD_L >> getComponentScaleX(chId);
			if (m_sVideoInfo.bPERP)
				pSrc += iPadWidth_L;
#endif
			if (!ch || (m_chromaFormatIDC == CHROMA_420 && !m_bResampleChroma))
			{
				pDst = m_pFacesOrig[0][ch];
#if SVIDEO_BLENDING
				if (m_sVideoInfo.bPERP)
				{
					Int iPadWidth = iPadWidth_L + (SVIDEO_ERP_PAD_R >> getComponentScaleX(chId));

					Pel *pSrcPadL = pSrcYuv->getAddr(chId);
					Pel *pSrcPadR = pSrcYuv->getAddr(chId) + nWidth;
					Pel *pDstR = pDst + nWidth - iPadWidth_L;

					for (Int j = 0; j<nHeight; j++)
					{
						memcpy(pDst, pSrc, nWidth * sizeof(Pel));

						for (Int i = 0; i < iPadWidth_L; i++)
							pDstR[i] = ((i * pSrcPadL[i] + (iPadWidth - i) * pSrcPadR[i]) + (iPadWidth >> 1)) / iPadWidth;

						for (Int i = iPadWidth_L, k = 0; i < iPadWidth; i++, k++)
							pDst[k] = ((i * pSrcPadL[i] + (iPadWidth - i) * pSrcPadR[i]) + (iPadWidth >> 1)) / iPadWidth;

						pDst += getStride(chId);
						pDstR += getStride(chId);
						pSrc += pSrcYuv->getStride(chId);
						pSrcPadL += pSrcYuv->getStride(chId);
						pSrcPadR += pSrcYuv->getStride(chId);
					}
				}
				else
#endif
				{
					for (Int j = 0; j<nHeight; j++)
					{
						memcpy(pDst, pSrc, nWidth * sizeof(Pel));
						pDst += getStride(chId);
						pSrc += pSrcYuv->getStride(chId);
					}
				}
				continue;
			}

			//padding;
			//left and right; 
			pSrc = pSrcYuv->getAddr(chId);
#if SVIDEO_ERP_PADDING
			if (m_sVideoInfo.bPERP)
				pSrc += iPadWidth_L;
#endif
			pDst = pSrc + nWidth;
			for (Int i = 0; i<nHeight; i++)
			{
				sPadH(pSrc, pDst, nMarginSizeTmpBuf);
				pSrc += iStrideTmpBuf;
				pDst += iStrideTmpBuf;
			}
			//top;
			pSrc = pSrcYuv->getAddr(chId) - nMarginSizeTmpBuf;
#if SVIDEO_ERP_PADDING
			if (m_sVideoInfo.bPERP)
				pSrc += iPadWidth_L;
#endif
			pDst = pSrc + nWidth / 2;
			for (Int i = -nMarginSizeTmpBuf; i<nWidth / 2 + nMarginSizeTmpBuf; i++)
			{
				sPadV(pSrc, pDst, iStrideTmpBuf, nMarginSizeTmpBuf);
				pSrc++;
				pDst++;
			}
			//bottom;
			pSrc = pSrcYuv->getAddr(chId) + (nHeight - 1)*iStrideTmpBuf - nMarginSizeTmpBuf;
#if SVIDEO_ERP_PADDING
			if (m_sVideoInfo.bPERP)
				pSrc += iPadWidth_L;
#endif
			pDst = pSrc + nWidth / 2;
			for (Int i = -nMarginSizeTmpBuf; i<nWidth / 2 + nMarginSizeTmpBuf; i++)
			{
				sPadV(pSrc, pDst, -iStrideTmpBuf, nMarginSizeTmpBuf);
				pSrc++;
				pDst++;
			}
			if (m_chromaFormatIDC == CHROMA_444)
			{
				//420->444;
				chromaUpsample(pSrcYuv->getAddr(chId), nWidth, nHeight, iStrideTmpBuf, 0, chId);
			}
			else
			{
				chromaResampleType0toType2(pSrcYuv->getAddr(chId), nWidth, nHeight, iStrideTmpBuf, m_pFacesOrig[0][ch], getStride(chId));
			}
		}
	}
	else if (pSrcYuv->getChromaFormat() == CHROMA_444 || pSrcYuv->getChromaFormat() == CHROMA_400)
	{
		if (m_chromaFormatIDC == pSrcYuv->getChromaFormat())
		{
			//copy;      
			for (Int ch = 0; ch<pSrcYuv->getNumberValidComponents(); ch++)
			{
				ComponentID chId = ComponentID(ch);
				Pel *pSrc = pSrcYuv->getAddr(chId);
#if SVIDEO_ERP_PADDING
				Int iPadWidth_L = SVIDEO_ERP_PAD_L >> getComponentScaleX(chId);
				if (m_sVideoInfo.bPERP)
					pSrc += iPadWidth_L;
#endif
				Pel *pDst = m_pFacesOrig[0][ch];
				for (Int j = 0; j<nHeight; j++)
				{
					memcpy(pDst, pSrc, nWidth * sizeof(Pel));
					pDst += getStride(chId);
					pSrc += pSrcYuv->getStride(chId);
				}
			}
		}
		else
			assert(!"Not supported yet");
	}
	else
		assert(!"Not supported yet");

	//set padding flag;
	setPaddingFlag(false);
}

Void TNewUniformMap::sPadH(Pel *pSrc, Pel *pDst, Int iCount)
{
	for (Int i = 1; i <= iCount; i++)
	{
		pDst[i - 1] = pSrc[i - 1];
		pSrc[-i] = pDst[-i];
	}
}

Void TNewUniformMap::sPadV(Pel *pSrc, Pel *pDst, Int iStride, Int iCount)
{
	for (Int i = 1; i <= iCount; i++)
	{
		pSrc[-i*iStride] = pDst[(i - 1)*iStride];
		pDst[-i*iStride] = pSrc[(i - 1)*iStride];
	}
}

Void TNewUniformMap::spherePadding(Bool bEnforced)
{
	if (!bEnforced && m_bPadded)
	{
		return;
	}
	m_bPadded = false;

#if SVIDEO_DEBUG
	//dump to file;
	static Bool bFirstDumpBeforePading = true;
	dumpAllFacesToFile("equirect_before_padding", false, !bFirstDumpBeforePading);
	bFirstDumpBeforePading = false;
#endif

	for (Int ch = 0; ch<(getNumChannels()); ch++)
	{
		ComponentID chId = (ComponentID)ch;
		Int nWidth = m_sVideoInfo.iFaceWidth >> getComponentScaleX(chId);
		Int nHeight = m_sVideoInfo.iFaceHeight >> getComponentScaleY(chId);
		Int nMarginX = m_iMarginX >> getComponentScaleX(chId);
		Int nMarginY = m_iMarginY >> getComponentScaleY(chId);

		//left and right;
		Pel *pSrc = m_pFacesOrig[0][ch];
		Pel *pDst = pSrc + nWidth;
		for (Int i = 0; i<nHeight; i++)
		{
			sPadH(pSrc, pDst, nMarginX);
			pSrc += getStride(ComponentID(ch));
			pDst += getStride(ComponentID(ch));
		}
		//top;
		pSrc = m_pFacesOrig[0][ch] - nMarginX;
		pDst = pSrc + (nWidth >> 1);
		for (Int i = -nMarginX; i<((nWidth >> 1) + nMarginX); i++)  //only top and bottom padding is necessary for the first stage vertical upsampling;
		{
			sPadV(pSrc, pDst, getStride(ComponentID(ch)), nMarginY);
			pSrc++;
			pDst++;
		}
		//bottom;
		pSrc = m_pFacesOrig[0][ch] + (nHeight - 1)*getStride(ComponentID(ch)) - nMarginX;
		pDst = pSrc + (nWidth >> 1);
		for (Int i = -nMarginX; i<((nWidth >> 1) + nMarginX); i++) //only top and bottom padding is necessary for the first stage vertical upsampling;
		{
			sPadV(pSrc, pDst, -getStride(ComponentID(ch)), nMarginY);
			pSrc++;
			pDst++;
		}
	}
	m_bPadded = true;

#if SVIDEO_DEBUG
	//dump to file;
	static Bool bFirstDumpAfterPading = true;
	dumpAllFacesToFile("equirect_after_padding", true, !bFirstDumpAfterPading);
	bFirstDumpAfterPading = false;
#endif

}

Void TNewUniformMap::framePack(TComPicYuv *pDstYuv)
{
	if (pDstYuv->getChromaFormat() == CHROMA_420)
	{
		if ((m_chromaFormatIDC == CHROMA_444) || (m_chromaFormatIDC == CHROMA_420 && m_bResampleChroma))
			spherePadding();

		assert(m_sVideoInfo.framePackStruct.chromaFormatIDC == CHROMA_420);
		if (m_chromaFormatIDC == CHROMA_444)
		{
			//1: 444->420;  444->422, H:[1, 6, 1]; 422->420, V[1,1];
			//(Wc*2Hc) and (Wc*Hc) temporal buffer; the resulting buffer is for rotation;
			Int nWidthC = m_sVideoInfo.iFaceWidth >> 1;
			Int nHeightC = m_sVideoInfo.iFaceHeight >> 1;
			Int nMarginSize = (m_filterDs[1].nTaps - 1) >> 1; //0, depending on V filter and considering south pole;
			Int nHeightC422 = m_sVideoInfo.iFaceHeight + nMarginSize * 2;
			Int iStride422 = nWidthC;
			Int iStride420 = nWidthC;
			if (!m_pDS422Buf)
			{
				m_pDS422Buf = (Pel*)xMalloc(Pel, nHeightC422*iStride422);
			}
			if (!m_pDS420Buf)
			{
				m_pDS420Buf = (Pel*)xMalloc(Pel, nHeightC*iStride420);
			}
			//1: 444->422;
			for (Int ch = 1; ch<getNumChannels(); ch++)
			{
				chromaDonwsampleH(m_pFacesOrig[0][ch] - nMarginSize*getStride((ComponentID)ch), m_sVideoInfo.iFaceWidth, nHeightC422, getStride((ComponentID)ch), 1, m_pDS422Buf, iStride422);
				chromaDonwsampleV(m_pDS422Buf + nMarginSize*iStride422, nWidthC, m_sVideoInfo.iFaceHeight, iStride422, 1, m_pDS420Buf, iStride420);
				rotOneFaceChannel(m_pDS420Buf, nWidthC, nHeightC, iStride420, 1, ch, m_sVideoInfo.framePackStruct.faces[0][0].rot, pDstYuv, 0, 0, 0, 0);
			}
			//luma;
			rotOneFaceChannel(m_pFacesOrig[0][0], m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, getStride((ComponentID)0), 1, 0, m_sVideoInfo.framePackStruct.faces[0][0].rot, pDstYuv, 0, 0, 0, (m_nBitDepth - m_nOutputBitDepth));
		}
		else //if(m_chromaFormatIDC == CHROMA_420)
		{
			//the resulting buffer is for chroma resampling;
			Int nWidthC = m_sVideoInfo.iFaceWidth >> getComponentScaleX((ComponentID)1);
			Int nHeightC = m_sVideoInfo.iFaceHeight >> getComponentScaleY((ComponentID)1);
			Int iStride420 = nWidthC;
			//chroma resample
			if (!m_pDS420Buf && (m_chromaFormatIDC == CHROMA_420 && m_bResampleChroma))
			{
				m_pDS420Buf = (Pel*)xMalloc(Pel, nHeightC*nWidthC);
			}
			for (Int ch = 1; ch<getNumChannels(); ch++)
			{
				if (m_bResampleChroma)
				{
					chromaResampleType2toType0(m_pFacesOrig[0][ch], m_pDS420Buf, nWidthC, nHeightC, getStride((ComponentID)ch), nWidthC);
					rotOneFaceChannel(m_pDS420Buf, nWidthC, nHeightC, iStride420, 1, ch, m_sVideoInfo.framePackStruct.faces[0][0].rot, pDstYuv, 0, 0, 0, 0);
				}
				else
					rotOneFaceChannel(m_pFacesOrig[0][ch], nWidthC, nHeightC, getStride((ComponentID)ch), 1, ch, m_sVideoInfo.framePackStruct.faces[0][0].rot, pDstYuv, 0, 0, 0, (m_nBitDepth - m_nOutputBitDepth));
			}
			//luma;
			rotOneFaceChannel(m_pFacesOrig[0][0], m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, getStride((ComponentID)0), 1, 0, m_sVideoInfo.framePackStruct.faces[0][0].rot, pDstYuv, 0, 0, 0, (m_nBitDepth - m_nOutputBitDepth));
		}
	}
	else if (pDstYuv->getChromaFormat() == CHROMA_444 || pDstYuv->getChromaFormat() == CHROMA_400)
	{
		if (m_chromaFormatIDC == pDstYuv->getChromaFormat())
		{
			for (Int ch = 0; ch<getNumChannels(); ch++)
			{
				ComponentID chId = (ComponentID)ch;
				rotOneFaceChannel(m_pFacesOrig[0][ch], m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, getStride(chId), 1, ch, m_sVideoInfo.framePackStruct.faces[0][0].rot, pDstYuv, 0, 0, 0, (m_nBitDepth - m_nOutputBitDepth));
			}
		}
		else
			assert(!"Not supported yet");
	}
#if SVIDEO_ERP_PADDING
	if (m_sVideoInfo.bPERP)
	{
		for (Int ch = 0; ch < getNumChannels(); ch++)
		{
			ComponentID chId = (ComponentID)ch;
			Pel *pDstBuf = pDstYuv->getAddr(chId);
			Int iHeight = pDstYuv->getHeight(chId);
			Int iStride = pDstYuv->getStride(chId);
			Int iFaceWidth = m_sVideoInfo.iFaceWidth >> getComponentScaleX(chId);
			Int iPadWidth_L = SVIDEO_ERP_PAD_L >> getComponentScaleX(chId);
			Int iPadWidth_R = SVIDEO_ERP_PAD_R >> getComponentScaleX(chId);
#if 1
			Pel* BuffTemp = NULL;
			if (iPadWidth_L)
				BuffTemp = new Pel[iFaceWidth + iPadWidth_R];
			for (Int j = 0; j < iHeight; j++)
			{
				//padding at right side;
				memcpy(pDstBuf + j*iStride + iFaceWidth, pDstBuf + j*iStride, sizeof(Pel)*iPadWidth_R);

				if (iPadWidth_L)
				{
					memcpy(BuffTemp, pDstBuf + j*iStride, sizeof(Pel)*(iFaceWidth + iPadWidth_R));
					//padding at left side;
					memcpy(pDstBuf + j*iStride, BuffTemp + iFaceWidth - iPadWidth_L, sizeof(Pel)*iPadWidth_L);
					memcpy(pDstBuf + j*iStride + iPadWidth_L, BuffTemp, sizeof(Pel)*(iFaceWidth + iPadWidth_R));
				}
			}
			if (BuffTemp)
				delete[] BuffTemp;
#else
			Int iWidth = pDstYuv->getWidth(chId);
			for (Int j = 0; j < iHeight; j++)
				memcpy(pDstBuf + j*iStride + iFaceWidth, pDstBuf + j*iStride, sizeof(Pel)*iPadWidth_R);

			if (iPadWidth_L)
			{
				Pel* BuffTemp = new Pel[iWidth * iHeight];
				for (Int j = 0; j < iHeight; j++)
					memcpy(BuffTemp + j*iWidth, pDstBuf + j*iStride, sizeof(Pel)*iWidth);

				for (Int j = 0; j < iHeight; j++)
				{
					memcpy(pDstBuf + j*iStride, BuffTemp + j*iWidth + iFaceWidth - iPadWidth_L, sizeof(Pel)*iPadWidth_L);
					memcpy(pDstBuf + j*iStride + iPadWidth_L, BuffTemp + j*iWidth, sizeof(Pel)*(iFaceWidth + iPadWidth_R));
				}
				delete[] BuffTemp;
			}
#endif
		}
	}
#endif
}

#if SVIDEO_ERP_PADDING
Void TNewUniformMap::geoToFramePack(IPos* posIn, IPos2D* posOut)
{
	TGeometry::geoToFramePack(posIn, posOut);
	if (m_sVideoInfo.bPERP)
	{
		assert(m_sVideoInfo.framePackStruct.faces[m_facePos[posIn->faceIdx][0]][m_facePos[posIn->faceIdx][1]].rot == 0);
		posOut->x += SVIDEO_ERP_PAD_L;
	}
}
#endif

#endif