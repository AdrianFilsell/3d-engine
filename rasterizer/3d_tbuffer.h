#pragma once

#include "3d.h"
#include <atomic>
#include <thread>
#include <chrono>

namespace af3d
{

template <typename T=RAS_FLTTYPE> class tbuffer
{
public:
	tbuffer():m_nWidth(0),m_nHeight(0){}
	~tbuffer(){}
	
	__forceinline bool isempty(void) const { return m_vScanlines.size()==0; }
	__forceinline int getwidth(void) const { return m_nWidth; }
	__forceinline int getheight(void) const { return m_nHeight; }
	__forceinline const T *getscanline(const int n)const{return n<0||n>=m_nHeight?nullptr:(&m_vScanlines[0])+(getwidth()*n);}
	__forceinline int getbytesperpixel(void) const {return s_nT;}
	__forceinline int getbytesperscanline(void) const {return m_nWidth*getbytesperpixel();}
	
	void destroy(void){m_nWidth=0;m_nHeight=0;m_vScanlines.clear();}
	bool create(const int nWidth, const int nHeight)
	{
		if(getwidth()==nWidth && getheight()==nHeight)
			return true;
		destroy();
		if(createstg(nWidth,nHeight))
		{
			m_nWidth = nWidth;
			m_nHeight = nHeight;
			return true;
		}
		destroy();
		return false;
	}
	__forceinline T *getscanline(const int n){return n<0||n>=m_nHeight?nullptr:(&m_vScanlines[0])+(getwidth()*n);}
protected:
	static const int s_nT=sizeof(T);
	int m_nWidth;
	int m_nHeight;
	std::vector<T> m_vScanlines;

	class op
	{
	public:
		op(tbuffer *pBuf,const vec2<int>&tl,int nWidth):m_pBuf(pBuf),m_TL(tl),m_nWidth(nWidth){}
		void operator()(const int nFrom,const int nInclusiveTo,const afthread::taskinfo *m_pTaskInfo)const
		{
			T *pSrc=m_pBuf->getscanline(m_TL[1]) + m_TL[0];
			T *pDst=m_pBuf->getscanline(m_TL[1] + nFrom) + m_TL[0];
			if(m_TL[0]==0 && m_nWidth==m_pBuf->getwidth())
			{
				const int nScanlines=1;
				const int nBytes = m_pBuf->getbytesperscanline();
				memcpy(pDst,pSrc,nScanlines*nBytes);
				
				int nAvailable=nScanlines;
				int nRemaining=(nInclusiveTo-nFrom+1)-nScanlines;
				pSrc=pDst;
				pDst+=nScanlines*m_nWidth;

				while(nRemaining)
				{
					const int nScanlines=nRemaining>nAvailable?nAvailable:nRemaining;
					memcpy(pDst,pSrc,nScanlines*nBytes);
					nAvailable+=nScanlines;
					nRemaining-=nScanlines;
					pDst+=nScanlines*m_nWidth;
				}
			}
			else
			{
				const int nBytes = m_nWidth*m_pBuf->getbytesperpixel();
				for(int n=nFrom;n<=nInclusiveTo;++n,pDst+=m_pBuf->getwidth())
					memcpy(pDst,pSrc,nBytes);
			}
		}
	protected:
		tbuffer *m_pBuf;
		const vec2<int>& m_TL;
		const int m_nWidth;
	};

	bool createstg(const int nWidth,const int nHeight)
	{
		const int nAlloc = nWidth*nHeight;
		if( nAlloc == 0 )
			return false;
		m_vScanlines.resize(nAlloc);
		return true;
	}
};

template <typename T=RAS_FLTTYPE> class zvertex
{
public:
	zvertex(){/*count.store(0, std::memory_order_relaxed);*/count=0;}
	zvertex(const zvertex& o){/*count.store(0, std::memory_order_relaxed);*/count=0;}
	
	__forceinline T get(void)const{return m_d;}
	__forceinline void set(const T d){m_d=d;}
	__forceinline void acquire(void)
	{
		//int n = count.fetch_add(1, std::memory_order_acquire);
		LONG n = _InterlockedExchangeAdd(&count, 1);
		while (n > 0)
		{
			release();
			std::this_thread::sleep_for(std::chrono::milliseconds(0)); // Yield to other threads
			//n = count.fetch_add(1, std::memory_order_acquire);
			n = _InterlockedExchangeAdd(&count, 1);
		}
	}
	__forceinline void release(void)
	{
		//int n = count.fetch_sub(1, std::memory_order_release);
		const LONG n = _InterlockedExchangeAdd(&count, -1);
	}
	zvertex& operator=(const zvertex& o); // not implemented because of atomic
protected:
	T m_d;
	mutable volatile LONG count;
	//std::atomic<int> count;
};

template <typename T=RAS_FLTTYPE> class zbuffer:public tbuffer<zvertex<T>>
{
public:
	zbuffer(){}
	~zbuffer(){}

	void clear(const afthread::taskscheduler *pSched,const rect& r,const T d)
	{
		const rect i=r.getintersect({{0,0},{m_nWidth,m_nHeight}});
		if(i.isempty())
			return;
	
		const int nWidth=i.getwidth();
		zvertex<T> *pSrc=getscanline(i.get(rect::v_tl)[1]) + i.get(rect::v_tl)[0];
		for(int nX=0;nX<nWidth;++nX)
			pSrc[nX].set(d);
		
		const int nHeight=i.getheight();
		if(pSched)
			pSched->parallel_for(1,nHeight-1,pSched->getcores(),op(this,i.get(rect::v_tl),nWidth));
		else
			op(this,i.get(rect::v_tl),nWidth)(1,1+nHeight-2,nullptr);
	}
};

template <typename T=RAS_FLTTYPE> class shadowmap:public zbuffer<T>
{
public:
	shadowmap(){}
	shadowmap(const shadowmap& o):zbuffer<T>(o){}
	
	__forceinline mat4<T>& getworldtoclipspace(void)const{return m_WorldToClipSpace;}
	void setworldtoclipspace(const mat4<T>& t){m_WorldToClipSpace=t;}

	__forceinline const af3d::rect& getdstndcclip(void)const{return m_rDstNDCClip;}
	void setdstndcclip(const af3d::rect& r){m_rDstNDCClip=r;}

	__forceinline void getdepth(const vec3<T>& worldpos,T& dDepth)const
	{
		vec4<T> clipspacepos;
		m_WorldToClipSpace.mul(worldpos,clipspacepos);

		vec3<T> shadowmapNDCspace;
		clipspacepos.getdevice(m_rDstNDCClip,shadowmapNDCspace);

		if(shadowmapNDCspace[0]<0.0 || shadowmapNDCspace[1]<0.0)
			return;

		const int nX=af::postodiscrete<T>(shadowmapNDCspace[0])-m_rDstNDCClip.get(rect::v_tl)[0];
		const int nY=af::postodiscrete<T>(shadowmapNDCspace[1])-m_rDstNDCClip.get(rect::v_tl)[1];
		
		if(nX<0 || nY<0)
			return;

		if(nX>=getwidth() || nY>=getheight())
			return;

		dDepth=m_vScanlines[nY*getwidth()+nX].get();
	}
protected:
	mat4<T> m_WorldToClipSpace;
	rect m_rDstNDCClip;
};

template <typename T=RAS_FLTTYPE> class gvertex
{
public:
	gvertex(){m_bFrag=false;}
	gvertex(const gvertex& o){m_bFrag=o.m_bFrag;}
	
	__forceinline const vec3<T>& getfrag(void)const{return m_Frag;}
	__forceinline bool isfrag(void)const{return m_bFrag;}

	__forceinline vec3<T>& getfrag(void){return m_Frag;}
	__forceinline void setfrag(void){m_bFrag=true;}
	__forceinline void clear(void){m_bFrag=false;}
	gvertex& operator=(const gvertex& o); // not implemented because of atomic
protected:
	bool m_bFrag;
	vec3<T> m_Frag;
};

template <typename T=RAS_FLTTYPE> class gbuffer:public tbuffer<gvertex<T>>
{
public:
	gbuffer(){}
	~gbuffer(){}
};

}
