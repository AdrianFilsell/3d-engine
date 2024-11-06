#pragma once

#include <Windows.h>
#include "3d.h"
#include "thread_taskscheduler.h"

namespace afdib
{

struct b8g8r8
{
	b8g8r8(){}
	b8g8r8(const unsigned char _b,const unsigned char _g,const unsigned char _r){b=_b;g=_g;r=_r;}
	unsigned char b;
	unsigned char g;
	unsigned char r;
	template <typename T> __forceinline static T getmaxrecip(void){return 1/T(255.0);}
};

struct b8g8r8a8
{
	b8g8r8a8(){}
	b8g8r8a8(const unsigned char _b,const unsigned char _g,const unsigned char _r,const unsigned char _a){b=_b;g=_g;r=_r;a=_a;}
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;
	template <typename T> __forceinline static T getmaxrecip(void){return 1/T(255.0);}
};

template <typename T> class pixel
{
public:
	__forceinline static void blendbgra(T& dDstB,T& dDstG,T& dDstR,T& dDstA,const T dB,const T dG,const T dR,const T dA)
	{
		// Check for special cases
		if( dA == 0 )
			return;
		if( dA == 1.0 )
		{
			return;
		}
		if( dDstA == 1 )
		{
			dDstB = ( dDstB * ( 1.0 - dA ) + dA * dB );
			dDstG = ( dDstG * ( 1.0 - dA ) + dA * dG );
			dDstR = ( dDstR * ( 1.0 - dA ) + dA * dR );
			return;
		}
		if( dDstA == 0 )
		{
			dDstB = dB;
			dDstG = dG;
			dDstR = dR;
			return;
		}
		dDstA = dA + dDstA - ( dA * dDstA );

		const T dNewTmp = (dA) / dDstA;
		dDstB = ( dDstB - ( ( dDstB - dB ) * dDstA ) );
		dDstG = ( dDstG - ( ( dDstG - dG ) * dDstA ) );
		dDstR = ( dDstR - ( ( dDstR - dR ) * dDstA ) );
	}

	template <typename PT> __forceinline static void blend(PT& dst,const T dB,const T dG,const T dR,const T dA);
	template <> __forceinline static void blend(b8g8r8& dst,const T dB,const T dG,const T dR,const T dA)
	{
		// Check for special cases
		if(dA==0)
			return;
		if(dA==1.0)
		{
			dst.b = af::posround<T,unsigned int>(dB*255.0);
			dst.g = af::posround<T,unsigned int>(dG*255.0);
			dst.r = af::posround<T,unsigned int>(dR*255.0);
			return;
		}
		dst.b = af::posround<T,unsigned int>( ( (dst.b / 255.0) * ( 1.0 - dA ) + dA * dB ) * 255.0 );
		dst.g = af::posround<T,unsigned int>( ( (dst.g / 255.0) * ( 1.0 - dA ) + dA * dG ) * 255.0 );
		dst.r = af::posround<T,unsigned int>( ( (dst.r / 255.0) * ( 1.0 - dA ) + dA * dR ) * 255.0 );
	}
	template <> __forceinline static void blend(b8g8r8a8& dst,const T dB,const T dG,const T dR,const T dA)
	{
		// Check for special cases
		if( dA == 0 )
			return;
		if( dA == 1.0 )
		{
			dst.b = af::posround<T,unsigned int>(dB*255.0);
			dst.g = af::posround<T,unsigned int>(dG*255.0);
			dst.r = af::posround<T,unsigned int>(dR*255.0);
			return;
		}
		if( dst.a == 255 )
		{
			dst.b = af::posround<T,unsigned int>( ( (dst.b / 255.0) * ( 1.0 - dA ) + dA * dB ) * 255.0 );
			dst.g = af::posround<T,unsigned int>( ( (dst.g / 255.0) * ( 1.0 - dA ) + dA * dG ) * 255.0 );
			dst.r = af::posround<T,unsigned int>( ( (dst.r / 255.0) * ( 1.0 - dA ) + dA * dR ) * 255.0 );
			return;
		}
		if( dst.a == 0 )
		{
			dst.b = af::posround<T,unsigned int>(dB*255.0);
			dst.g = af::posround<T,unsigned int>(dG*255.0);
			dst.r = af::posround<T,unsigned int>(dR*255.0);
			return;
		}
		const T dNewA = (dA * 255.0) + dst.a - ( (dA * 255.0) * dst.a ) / 255.0;
		dst.a = af::posround<T,unsigned int>( dNewA );

		const T dNewTmp = (dA * 255.0) / dNewA;
		dst.b = af::posround<T,unsigned int>( dst.b - ( ( dst.b - (dB * 255.0) ) * dNewTmp ) );
		dst.g = af::posround<T,unsigned int>( dst.g - ( ( dst.g - (dG * 255.0) ) * dNewTmp ) );
		dst.r = af::posround<T,unsigned int>( dst.r - ( ( dst.r - (dR * 255.0) ) * dNewTmp ) );
	}
};

class dib
{
public:
	enum pixeltype
	{
		pt_b8g8r8,pt_b8g8r8a8,
	};
	dib();
	~dib();
	
	__forceinline bool isempty(void) const { return m_vScanlines.size()==0; }
	__forceinline bool getopaque( void ) const { return getopaquepixel( m_PixelType ); }
	__forceinline int getwidth( void ) const { return m_nWidth; }
	__forceinline int getheight( void ) const { return m_nHeight; }
	__forceinline pixeltype getpixeltype( void ) const { return m_PixelType; }
	__forceinline int getbitsperchannel( void ) const { return getbitsperchannel( m_PixelType ); }
	__forceinline int getbitsperpixel( void ) const {	return getbitsperpixel( m_PixelType ); }
	__forceinline int getbytesperpixel( void ) const { return getbytesperpixel( m_PixelType ); }
	__forceinline int getbytesperscanline( void ) const { return getbytesperscanline( m_nWidth, m_PixelType ); }
	__forceinline int getallocsize( void ) const { return getheight() * getbytesperscanline(); }
	__forceinline const unsigned char *getscanline(const int n)const{return n<0||n>=m_nHeight?nullptr:(&m_vScanlines[0])+(getbytesperscanline()*n);}
	
	std::shared_ptr<dib> cut(const int nLeft,const int nTop,const int nWidth,const int nHeight)const;

	void getbmihdr( BITMAPINFOHEADER *pBMIHdr ) const;
	BITMAPINFO *dib::createbitmapinfo( void ) const;
	void tidybmi( BITMAPINFO *p ) const;
	
	void destroy( void );
	bool create( const int nWidth, const int nHeight, const pixeltype pt );
	__forceinline unsigned char *getscanline(const int n){return n<0||n>=m_nHeight?nullptr:(&m_vScanlines[0])+(getbytesperscanline()*n);}
	void clear(const afthread::taskscheduler *pSched,const af3d::rect& r,const b8g8r8a8& bgra);
	void clear(const afthread::taskscheduler *pSched,const af3d::vec2<int>& origin,const af3d::rect& r,const b8g8r8a8& chqA,const b8g8r8a8& chqB,const int nChqDim);
	void blt(const int x, const int y);
	void blt(HDC hDC,const af3d::rect& rSrcClip,const af3d::rect& rDstClip);
	void blt(const afthread::taskscheduler *pSched,dib *pDst,const af3d::rect& r)const;

	static bool getopaquepixel( const pixeltype pt );
	static int getbitsperchannel( const pixeltype pt );
	static int getbitsperpixel( const pixeltype pt );
	static int getbytesperpixel( const pixeltype pt );
	static int getbytesperscanline( const int nWidth, const int nBitsPerPixel );
	static int getbytesperscanline( const int nWidth, const pixeltype pt ) { return getbytesperscanline( nWidth, getbitsperpixel( pt ) ); }

protected:
	int m_nWidth;
	int m_nHeight;
	pixeltype m_PixelType;
	std::vector<unsigned char> m_vScanlines;

	struct chqinfo
	{
		int nPixels;
		int nPixelFrom;
		int nChunkFrom;
		int nTotalChunks;
		int nFirstChunkPixels;
		int nMidWholeChunks;
		int nLastChunkPixels;
		void getpixels(const int nChunk,const int nChqDim,int& nFrom,int& nCount)const
		{
			if(nChunk==0)
			{
				nFrom=nPixelFrom;
				nCount=nFirstChunkPixels;
			}
			else
			if(nChunk<(1+nMidWholeChunks))
			{
				nFrom = nPixelFrom + nFirstChunkPixels + ( ( nChunk - 1 ) * nChqDim );
				nCount = nChqDim;
			}
			else
			{
				nFrom = nPixelFrom + nFirstChunkPixels + ( nMidWholeChunks * nChqDim );
				nCount=nLastChunkPixels;
			}
		}
	};
	void getchqinfo(const int nChqOrigin,const int nFrom,const int nInclusiveTo,const int nChqDim,chqinfo& i)const
	{
		i.nPixels = nInclusiveTo - nFrom + 1;
		i.nPixelFrom = nFrom;
		
		const int nDelta = 0 - nChqOrigin;
		int nNormFrom = nFrom;
		if(nFrom < nChqOrigin)
			nNormFrom = nChqDim + nChqDim + ( nNormFrom + nDelta );
		else
			nNormFrom = ( nNormFrom + nDelta );
		i.nChunkFrom = nNormFrom / nChqDim;
		i.nFirstChunkPixels = (nChqDim - (nNormFrom - (i.nChunkFrom * nChqDim)));
		if(i.nFirstChunkPixels>i.nPixels)
			i.nFirstChunkPixels=i.nPixels;
		i.nMidWholeChunks= ((i.nPixels - i.nFirstChunkPixels) / nChqDim);
		i.nLastChunkPixels = i.nPixels - ((i.nMidWholeChunks* nChqDim) + i.nFirstChunkPixels);
		i.nTotalChunks = 1 + i.nMidWholeChunks + (i.nLastChunkPixels ? 1 : 0);
	}
	template <pixeltype PE> void clearchqrow(const chqinfo& horzinfo,const chqinfo& vertinfo, const int nVertChunk,const b8g8r8a8& chqA,const b8g8r8a8& chqB,const int nChqDim);
	template <pixeltype PE> void clarchqrowpixels(const af3d::vec2<int>& p,const int nPixels,const b8g8r8a8& pxl);
	bool getchqchunkcolourA(const int nChunkX,const int nChunkY)const;

	void setdibitstodevice( HDC dst, const int nXDest, const int nYDest, const int nWidth, const int nHeight, const BITMAPINFO *pSrc, unsigned char *pSrcdata, const int nXSrc, const int nYSrc, const int nStartScan, const int nScanLines ) const;

	bool createstg( const int nWidth, const int nHeight, const pixeltype pt );

	class fillop
	{
	public:
		fillop(dib *pBuf,const af3d::vec2<int>&tl,const int nWidth):m_pBuf(pBuf),m_TL(tl),m_nWidth(nWidth){}
		void operator()(const int nFrom,const int nInclusiveTo,const afthread::taskinfo *m_pTaskInfo)const
		{
			const int nBytesPerPixel=m_pBuf->getbytesperpixel();
			const int nBytesPerScanline=m_pBuf->getbytesperscanline();
			
			unsigned char *pSrc=m_pBuf->getscanline(m_TL[1]) + (nBytesPerPixel * m_TL[0]);
			unsigned char *pDst=m_pBuf->getscanline(m_TL[1] + nFrom) + (nBytesPerPixel * m_TL[0]);
			
			if(m_TL[0]==0 && m_nWidth==m_pBuf->getwidth())
			{				
				const int nScanlines=1;
				const int nBytes = nBytesPerScanline;
				memcpy(pDst,pSrc,nScanlines*nBytes);
				
				int nAvailable=nScanlines;
				int nRemaining=(nInclusiveTo-nFrom+1)-nScanlines;
				pSrc=pDst;
				pDst+=nBytesPerScanline;

				while(nRemaining)
				{
					const int nScanlines=nRemaining>nAvailable?nAvailable:nRemaining;
					memcpy(pDst,pSrc,nScanlines*nBytes);
					nAvailable+=nScanlines;
					nRemaining-=nScanlines;
					pDst+=nScanlines*nBytes;
				}
			}
			else
			{
				const int nBytes = m_nWidth*nBytesPerPixel;
				for(int n=nFrom;n<=nInclusiveTo;++n,pDst+=nBytesPerScanline)
					memcpy(pDst,pSrc,nBytes);
			}
		}
	protected:
		dib *m_pBuf;
		const af3d::vec2<int>& m_TL;
		const int m_nWidth;
	};

	class chunkop
	{
	public:
		chunkop(dib *pBuf,const af3d::vec2<int>&srctl,const af3d::vec2<int>&dsttl,const int nWidth,const int nChunkDim):m_nChunkDim(nChunkDim),m_pBuf(pBuf),m_srcTL(srctl),m_dstTL(dsttl),m_nWidth(nWidth){}
		void operator()(const int nFrom,const int nInclusiveTo,const afthread::taskinfo *m_pTaskInfo)const
		{
			const int nBytesPerPixel=m_pBuf->getbytesperpixel();
			const int nBytesPerScanline=m_pBuf->getbytesperscanline();
			
			bool bChunk_1 = nFrom % 2;

			unsigned char *pSrc_0=m_pBuf->getscanline(m_srcTL[1]) + (nBytesPerPixel * m_srcTL[0]);
			unsigned char *pSrc_1=pSrc_0 + (nBytesPerScanline * m_nChunkDim);
			unsigned char *pDst=m_pBuf->getscanline(m_dstTL[1] + (nFrom*m_nChunkDim)) + (nBytesPerPixel * m_dstTL[0]);

			const bool bWholeChunk=m_srcTL[0]==0 && m_nWidth==m_pBuf->getwidth();

			for(int nChunk=nFrom;nChunk<=nInclusiveTo;++nChunk)
			{
				unsigned char *pSrc=bChunk_1?pSrc_1:pSrc_0;
				if(bWholeChunk)
				{
					memcpy(pDst,pSrc,m_nChunkDim * nBytesPerScanline);
					pDst+=m_nChunkDim * nBytesPerScanline;
				}
				else
					for(int n=0;n<m_nChunkDim;++n,pDst+=nBytesPerScanline,pSrc+=nBytesPerScanline)
						memcpy(pDst,pSrc,m_nWidth * nBytesPerPixel);
				bChunk_1=!bChunk_1;
			}
		}
	protected:
		dib *m_pBuf;
		const af3d::vec2<int>& m_srcTL;
		const af3d::vec2<int>& m_dstTL;
		const int m_nWidth;
		const int m_nChunkDim;
	};

	class bltop
	{
	public:
		bltop(dib *pDstBuf,const dib *pSrcBuf,const af3d::vec2<int>&tl,const int nWidth):m_pDstBuf(pDstBuf),m_pSrcBuf(pSrcBuf),m_TL(tl),m_nWidth(nWidth){}
		void operator()(const int nFrom,const int nInclusiveTo,const afthread::taskinfo *m_pTaskInfo)const
		{
			const int nBytesPerPixel=m_pSrcBuf->getbytesperpixel();
			const int nBytesPerScanline=m_pSrcBuf->getbytesperscanline();

			const unsigned char *pSrc=m_pSrcBuf->getscanline(m_TL[1] + nFrom) + (nBytesPerPixel * m_TL[0]);
			unsigned char *pDst=m_pDstBuf->getscanline(m_TL[1] + nFrom) + (nBytesPerPixel * m_TL[0]);
			
			if(m_TL[0]==0 && m_nWidth==m_pSrcBuf->getwidth())
			{
				const int nScanlines=(nInclusiveTo-nFrom+1);
				const int nBytes = nBytesPerScanline;
				memcpy(pDst,pSrc,nScanlines*nBytes);
			}
			else
			{
				const int nBytes = m_nWidth*nBytesPerPixel;
				for(int n=nFrom;n<=nInclusiveTo;++n,pDst+=nBytesPerScanline,pSrc+=nBytesPerScanline)
					memcpy(pDst,pSrc,nBytes);
			}
		}
	protected:
		dib *m_pDstBuf;
		const dib *m_pSrcBuf;
		const af3d::vec2<int>& m_TL;
		const int m_nWidth;
	};

};

}
