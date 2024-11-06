
#include "pch.h"
#include "dib.h"

namespace afdib
{

dib::dib()
{
	m_nWidth = 0;
	m_nHeight = 0;
	m_PixelType = pt_b8g8r8;
}

dib::~dib()
{
	destroy();
}

std::shared_ptr<dib> dib::cut(const int nLeft,const int nTop,const int nWidth,const int nHeight)const
{
	std::shared_ptr<dib> sp(new dib);
	if(!sp->create(nWidth,nHeight,getpixeltype()))
		return nullptr;
	const int nBytesPerPixel=getbytesperpixel();
	for(int nY=0;nY<nHeight;++nY)
		memcpy(sp->getscanline(nY),getscanline(nY+nTop)+(nBytesPerPixel*nLeft),nBytesPerPixel*nWidth);
	return sp;
}

bool dib::create( const int nWidth, const int nHeight, const pixeltype pt )
{
	if(getwidth()==nWidth && getheight()==nHeight && getpixeltype()==pt)
		return true;
	destroy();
	if( createstg( nWidth, nHeight, pt ) )
	{
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		m_PixelType = pt;
		return true;
	}
	destroy();
	return false;
}

void dib::destroy( void )
{
	m_nWidth = 0;
	m_nHeight = 0;
	m_PixelType = pt_b8g8r8;
	m_vScanlines.clear();
}

bool dib::createstg( const int nWidth, const int nHeight, const pixeltype pt )
{
	const int nAlloc = getbytesperscanline( nWidth, pt ) * nHeight;
	if( nAlloc == 0 )
		return false;
	m_vScanlines.resize(nAlloc);
	return true;
}

bool dib::getopaquepixel( const pixeltype pt )
{
	switch( pt )
	{
		case pt_b8g8r8:return true;
		case pt_b8g8r8a8:return false;
		default:
		{
			ASSERT( false );
			return false;
		}
	}
}

int dib::getbitsperchannel( const pixeltype pt )
{
	switch( pt )
	{
		case pt_b8g8r8:
		case pt_b8g8r8a8:return 8;
		default:
		{
			ASSERT( false );
			return 0;
		}
	}
}

int dib::getbitsperpixel( const pixeltype pt )
{
	switch( pt )
	{
		case pt_b8g8r8:return 24;
		case pt_b8g8r8a8:return 32;
		default:
		{
			ASSERT( false );
			return 0;
		}
	}
}

int dib::getbytesperpixel( const pixeltype pt )
{
	switch( pt )
	{
		case pt_b8g8r8:return 3;
		case pt_b8g8r8a8:return 4;
		default:
		{
			ASSERT( false );
			return 0;
		}
	}
}

int dib::getbytesperscanline( const int nWidth, const int nBitsPerPixel )
{
	// Determine width in bits
	int nRetVal = nWidth * nBitsPerPixel;
	
	// Determine width in bytes, rounding up to the nearest byte
	nRetVal = ( nRetVal + 7 ) >> 3;
	
	// Add on any additional padding to make return correct multiple
	return ( ( ( nRetVal + 3 ) >> 2 ) << 2 );
}

void dib::getbmihdr( BITMAPINFOHEADER *pBMIHdr ) const
{
	memset( pBMIHdr, 0, sizeof(BITMAPINFOHEADER) );
	pBMIHdr->biSize = sizeof(BITMAPINFOHEADER);
	pBMIHdr->biWidth = getwidth();
	const bool bTopLeftOrigin = true;
	pBMIHdr->biHeight = bTopLeftOrigin ? -getheight() : getheight();
	pBMIHdr->biPlanes = 1;
	pBMIHdr->biBitCount = getbitsperpixel();
	pBMIHdr->biCompression = BI_RGB;
	pBMIHdr->biSizeImage = getbytesperscanline() * getheight();
}

void dib::tidybmi( BITMAPINFO *p ) const
{
	if( p )
		delete (char*)( p );
}

BITMAPINFO *dib::createbitmapinfo( void ) const
{
	// Check the pixel format
	switch( getpixeltype() )
	{
		case pt_b8g8r8:break;
		default:ASSERT( false );return nullptr;
	}

	const int nColours = 0;
	const int nBmpInfoSize = sizeof( BITMAPINFOHEADER ) + ( nColours * sizeof( RGBQUAD ) );
	BITMAPINFO *pBmpInfo = reinterpret_cast<BITMAPINFO*>( new char[nBmpInfoSize] );
		
	memset( pBmpInfo, 0, nBmpInfoSize );
	pBmpInfo->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	pBmpInfo->bmiHeader.biWidth = getwidth();
	pBmpInfo->bmiHeader.biHeight = LONG( getheight() ) * -1; // bottom left origin need positive height
	pBmpInfo->bmiHeader.biPlanes = 1;
	pBmpInfo->bmiHeader.biBitCount = getbitsperpixel();
	pBmpInfo->bmiHeader.biCompression = BI_RGB;
	pBmpInfo->bmiHeader.biSizeImage = getheight() * getbytesperscanline();
	
	if( nColours > 0 )
	{
		ASSERT( false );
	}

	return pBmpInfo;
}

void dib::blt(const afthread::taskscheduler *pSched,dib *pDst,const af3d::rect& r)const
{
	const af3d::rect i=r.getintersect({{0,0},{m_nWidth,m_nHeight}});
	if(i.isempty())
		return;

	if(!pDst || getpixeltype()!=pDst->getpixeltype() || pDst->getwidth()!=getwidth() || pDst->getheight()!=getheight())
		return;

	const int nWidth=i.getwidth();
	const int nHeight=i.getheight();
	if(pSched)
		pSched->parallel_for(0,nHeight,pSched->getcores(),bltop(pDst,this,i.get(af3d::rect::v_tl),nWidth));
	else
		bltop(pDst,this,i.get(af3d::rect::v_tl),nWidth)(0,nHeight-1,nullptr);
}

void dib::blt(HDC hDC,const af3d::rect& rSrcClip,const af3d::rect& rDstClip)
{
	BITMAPINFO *pInfo = createbitmapinfo();

	const af3d::vec2<int>& dstTL=rDstClip.get(af3d::rect::v_tl);
	const af3d::vec2<int>& srcTL=rSrcClip.get(af3d::rect::v_tl);

	setdibitstodevice( hDC,
					   dstTL[0], dstTL[1],
					   rSrcClip.getwidth(), rSrcClip.getheight(),
					   pInfo, getscanline(0),
					   srcTL[0] - 0, rSrcClip.get(af3d::rect::v_br)[1] - 0 - 1,
					   rSrcClip.get(af3d::rect::v_br)[1] - 0 - 1, rSrcClip.get(af3d::rect::v_br)[1] - 0 );

	tidybmi(pInfo);
}

void dib::blt( const int x, const int y )
{
	HWND hWnd = ::GetDesktopWindow();
	HDC hDC = ::GetDC(hWnd);

	BITMAPINFO *pInfo = createbitmapinfo();

	setdibitstodevice( hDC,
					   x, y,
					   getwidth(),getheight(),
					   pInfo, getscanline(0),
					   0 - 0, getheight() - 0 - 1,
					   getheight() - 0 - 1, getheight() - 0 );
	
	tidybmi(pInfo);
	::ReleaseDC(hWnd,hDC);
}

void dib::setdibitstodevice( HDC dst, const int nXDest, const int nYDest, const int nWidth, const int nHeight, const BITMAPINFO *pSrc, unsigned char *pSrcdata, const int nXSrc, const int nYSrc, const int nStartScan, const int nScanLines ) const
{
	int n = SetDIBitsToDevice( dst, nXDest, nYDest, nWidth, nHeight, nXSrc, nYSrc, nStartScan, nScanLines, pSrcdata, pSrc, DIB_RGB_COLORS );
}

void dib::clear(const afthread::taskscheduler *pSched,const af3d::rect& r,const b8g8r8a8& bgra)
{
	const af3d::rect i=r.getintersect({{0,0},{m_nWidth,m_nHeight}});
	if(i.isempty())
		return;
	
	const int nWidth=i.getwidth();
	const int nBytesPerPixel=getbytesperpixel();
	unsigned char *pSrc=getscanline(i.get(af3d::rect::v_tl)[1]) + ( nBytesPerPixel * i.get(af3d::rect::v_tl)[0] );
	switch(m_PixelType)
	{
		case pt_b8g8r8:
		{
			b8g8r8 *pPixelSrc=reinterpret_cast<b8g8r8*>(pSrc);
			for(int nX=0;nX<nWidth;++nX)
			{
				pPixelSrc[nX].b=bgra.b;
				pPixelSrc[nX].g=bgra.g;
				pPixelSrc[nX].r=bgra.r;
			}

			const int nHeight=i.getheight();
			if(pSched)
				pSched->parallel_for(1,nHeight-1,pSched->getcores(),fillop(this,i.get(af3d::rect::v_tl),nWidth));
			else
				fillop(this,i.get(af3d::rect::v_tl),nWidth)(1,1+nHeight-2,nullptr);
		}
		break;
		case pt_b8g8r8a8:
		{
			b8g8r8a8 *pPixelSrc=reinterpret_cast<b8g8r8a8*>(pSrc);
			for(int nX=0;nX<nWidth;++nX)
				pPixelSrc[nX]=bgra;

			const int nHeight=i.getheight();
			if(pSched)
				pSched->parallel_for(1,nHeight-1,pSched->getcores(),fillop(this,i.get(af3d::rect::v_tl),nWidth));
			else
				fillop(this,i.get(af3d::rect::v_tl),nWidth)(1,1+nHeight-2,nullptr);
		}
		break;
		default:return;
	}
}

void dib::clear(const afthread::taskscheduler *pSched,const af3d::vec2<int>& origin,const af3d::rect& r,const b8g8r8a8& chqA,const b8g8r8a8& chqB,const int nChqDim)
{
	if(r.isempty())return;
	const af3d::rect i=r.getintersect({{0,0},{m_nWidth,m_nHeight}});

	chqinfo horzinfo,vertinfo;
	getchqinfo(origin[0],i.get(af3d::rect::v_tl)[0],i.get(af3d::rect::v_br)[0]-1,nChqDim,horzinfo);
	getchqinfo(origin[1],i.get(af3d::rect::v_tl)[1],i.get(af3d::rect::v_br)[1]-1,nChqDim,vertinfo);
	if(horzinfo.nPixels==0 || vertinfo.nPixels==0)
		return;

	switch(m_PixelType)
	{
		case pt_b8g8r8:clearchqrow<pt_b8g8r8>(horzinfo,vertinfo,0,chqA,chqB,nChqDim);break;
		case pt_b8g8r8a8:clearchqrow<pt_b8g8r8a8>(horzinfo,vertinfo,0,chqA,chqB,nChqDim);break;
		default:return;
	}

	if(vertinfo.nMidWholeChunks)
	{
		for(int n=0;n<2 && n <vertinfo.nMidWholeChunks;++n)
			switch(m_PixelType)
			{
				case pt_b8g8r8:clearchqrow<pt_b8g8r8>(horzinfo,vertinfo,1+n,chqA,chqB,nChqDim);break;
				case pt_b8g8r8a8:clearchqrow<pt_b8g8r8a8>(horzinfo,vertinfo,1+n,chqA,chqB,nChqDim);break;
				default:return;
			}

		const af3d::vec2<int> srcTL(horzinfo.nPixelFrom,vertinfo.nPixelFrom+vertinfo.nFirstChunkPixels);
		const af3d::vec2<int> dstTL(srcTL[0],srcTL[1]+nChqDim+nChqDim);
		const int nCopyChunks = vertinfo.nMidWholeChunks > 2 ? vertinfo.nMidWholeChunks - 2 : 0;
		
		if(pSched)
			pSched->parallel_for(0,nCopyChunks,pSched->getcores(),chunkop(this,srcTL,dstTL,horzinfo.nPixels,nChqDim));
		else
			chunkop(this,srcTL,dstTL,horzinfo.nPixels,nChqDim)(0,nCopyChunks-1,nullptr);
	}

	if(vertinfo.nLastChunkPixels)
		switch(m_PixelType)
		{
			case pt_b8g8r8:clearchqrow<pt_b8g8r8>(horzinfo,vertinfo,vertinfo.nTotalChunks-1,chqA,chqB,nChqDim);break;
			case pt_b8g8r8a8:clearchqrow<pt_b8g8r8a8>(horzinfo,vertinfo,vertinfo.nTotalChunks-1,chqA,chqB,nChqDim);break;
			default:return;
		}
}

template <dib::pixeltype PE> void dib::clearchqrow(const chqinfo& horzinfo,const chqinfo& vertinfo, const int nVertChunk,const b8g8r8a8& chqA,const b8g8r8a8& chqB,const int nChqDim)
{
	const int nBytesPerPixel=getbytesperpixel();
	const int nBytesPerScanline=getbytesperscanline();

	int nPixelFromY,nPixelCountY;
	vertinfo.getpixels(nVertChunk,nChqDim,nPixelFromY,nPixelCountY);

	int nPixelFromX,nPixelCountX;
	horzinfo.getpixels(0,nChqDim,nPixelFromX,nPixelCountX);
	clarchqrowpixels<PE>({nPixelFromX,nPixelFromY},nPixelCountX,getchqchunkcolourA(horzinfo.nChunkFrom+0,vertinfo.nChunkFrom+nVertChunk)?chqA:chqB);

	if(horzinfo.nLastChunkPixels>0)
	{
		int nPixelFromX,nPixelCountX;
		horzinfo.getpixels(horzinfo.nTotalChunks-1,nChqDim,nPixelFromX,nPixelCountX);
		clarchqrowpixels<PE>({nPixelFromX,nPixelFromY},nPixelCountX,getchqchunkcolourA(horzinfo.nChunkFrom+horzinfo.nTotalChunks-1,vertinfo.nChunkFrom+nVertChunk)?chqA:chqB);
	}

	if(horzinfo.nMidWholeChunks)
	{
		int nRemainingMidWholeChunks=horzinfo.nMidWholeChunks;

		int nPixelFromX,nPixelCountX;
		horzinfo.getpixels(1,nChqDim,nPixelFromX,nPixelCountX);
		clarchqrowpixels<PE>({nPixelFromX,nPixelFromY},nPixelCountX,getchqchunkcolourA(horzinfo.nChunkFrom+1,vertinfo.nChunkFrom+nVertChunk)?chqA:chqB);
		--nRemainingMidWholeChunks;

		if(nRemainingMidWholeChunks)
		{
			int nPixelDstX=nPixelFromX+nChqDim;
			clarchqrowpixels<PE>({nPixelDstX,nPixelFromY},nPixelCountX,getchqchunkcolourA(horzinfo.nChunkFrom+2,vertinfo.nChunkFrom+nVertChunk)?chqA:chqB);
			--nRemainingMidWholeChunks;
			nPixelDstX+=nChqDim;

			int nAvailableCopyChunks=2;
			unsigned char *pScanline=getscanline(nPixelFromY);
			while(nRemainingMidWholeChunks>0)
			{
				const int nCopyChunks=nAvailableCopyChunks > nRemainingMidWholeChunks ? nRemainingMidWholeChunks : nAvailableCopyChunks;
			
				const int nCopiedPixels = (nAvailableCopyChunks * nChqDim);
				memcpy(pScanline+(nPixelDstX*nBytesPerPixel),pScanline+(nPixelFromX*nBytesPerPixel),nBytesPerPixel*nChqDim*nCopyChunks);

				nPixelDstX += nChqDim*nCopyChunks;
				nAvailableCopyChunks += nCopyChunks;
				nRemainingMidWholeChunks -= nCopyChunks;
			}
		}
	}
	unsigned char *pScanline=getscanline(nPixelFromY);
	for(int n=1;n<nPixelCountY;++n)
		memcpy(pScanline+(nPixelFromX*nBytesPerPixel)+(n*nBytesPerScanline),pScanline+(nPixelFromX*nBytesPerPixel),nBytesPerPixel*horzinfo.nPixels);
}

template <dib::pixeltype PE> void dib::clarchqrowpixels(const af3d::vec2<int>& p,const int nPixels,const b8g8r8a8& pxl)
{
	switch(PE)
	{
		case pt_b8g8r8:
		{
			b8g8r8 *pScanline=reinterpret_cast<b8g8r8*>(getscanline(p[1]));
			pScanline += p[0];
			for(int n=0;n<nPixels;++n)
			{
				pScanline[n].b=pxl.b;
				pScanline[n].g=pxl.g;
				pScanline[n].r=pxl.r;
			}
		}
		break;
		case pt_b8g8r8a8:
		{
			b8g8r8a8 *pScanline=reinterpret_cast<b8g8r8a8*>(getscanline(p[1]));
			pScanline += p[0];
			for(int n=0;n<nPixels;++n)
				pScanline[n]=pxl;
		}
		break;
	}
}

bool dib::getchqchunkcolourA(const int nChunkX,const int nChunkY)const
{
	if(nChunkX % 2)
	{
		if(nChunkY % 2)
			return true;
		return false;
	}
	if(nChunkY % 2)
		return false;
	return true;
}

}
