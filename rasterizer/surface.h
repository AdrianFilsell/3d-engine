#pragma once

#include "dib.h"
#include "hittest.h"
#include "3d_rendercontext.h"

struct cameraorient
{
	af3d::vec3<> up;
	af3d::vec3<> dir;
	af3d::vec3<> origin;
	bool operator ==(const cameraorient& o)const{return up==o.up && dir==o.dir && origin==o.origin;}
};

class surface
{
public:	
	surface(){}
	~surface();

	af3d::camera<> *getcamera(void)const{return m_spCamera.get();}
	const af3d::perspectiveprojection<> *getperspectiveproj(void)const{return m_spPerspectiveProjection.get();}
	void getprojmat(af3d::mat4<>& m)const{if(getperspectiveproj())m=getperspectiveproj()->gettrns();}
	const af3d::rect& getdstndc(void)const{return m_rDstNDC;}
	af3d::rect getdev(void)const{return m_spDev ? af3d::rect({0,0},{m_spDev->getwidth(),m_spDev->getheight()}) : af3d::rect();}
	af3d::rect getdevbbox(const af3d::facetrnsbbox<>& bbox)const;

	void start(void);
	void stop(void);

	void compose(const af3d::scene<> *pScene,const af3d::rect& rDeviceClip,const int nTypes);
	void resize(const af3d::scene<> *pScene,const int nW,const int nH);

	void clearshadowmaps(void){m_vShadowMaps.clear();}
	void composeshadowmaps(const af3d::scene<> *pScene,const af3d::vertexattsframe<>* pSpecificLight,const af3d::rect& rDeviceClip,const int nTypes);

	void flip(CDC *pDC,const af3d::rect& rDeviceClip) const;
	void flip(afdib::dib *pDst,const af3d::rect& rDeviceClip) const;
	
	void rendercircle(afdib::dib *pDst,const af3d::vec3<>& origin,const int nR,const af3d::vec4<>& bgra,const af3d::mat4<>& worldtoclip)const;
	void renderline(afdib::dib *pDst,const af3d::line_pos3<>& l,const af3d::vec4<>& bgra,const af3d::mat4<>& worldtoclip)const;

	void getworldray(const af3d::vec2<int>& ptDevice,const af3d::mat4<>& cliptoworld,af3d::vec3<>& worldposFront,af3d::vec3<>& worldposBack,af3d::vec3<>& worlddir)const;
	void ht(af3d::scene<> *pScene,const af3d::vec2<int>& ptDevice,const af3d::mat4<>& cliptoworld,const int nTypes,hittest<>& res)const;
	bool htcircle(const af3d::vec2<int>& rtpt,std::vector<af3d::vec3<>>& v,const int nR,const af3d::mat4<>& worldtoclip,int& nHandle)const;

	void setcameraorient(const cameraorient& o);
	cameraorient getcameraorient(void)const;

	// camera
	static const RAS_FLTTYPE s_dDefDistance;
	static const cameraorient s_DefFront;
	static const cameraorient s_DefBack;
	static const cameraorient s_DefAbove;
	static const cameraorient s_DefBelow;
	static const cameraorient s_DefLeft;
	static const cameraorient s_DefRight;
	
	// projection
	static const RAS_FLTTYPE s_dDefNear;
	static const RAS_FLTTYPE s_dDefFar;
	static const RAS_FLTTYPE s_dDefPerspectiveFov;
	static const RAS_FLTTYPE s_dDefSceneAspect;
	static const af3d::orthographicprojection<>::subtype s_dDefOrthographicSubType;

	static const COLORREF s_grey;

	static std::shared_ptr<const af3d::mat4<>> s_spIdentity;
protected:
	std::shared_ptr<af3d::zbuffer<>> m_spZBuffer;
	std::shared_ptr<af3d::gbuffer<>> m_spGBuffer;
	std::shared_ptr<afdib::dib> m_spDev;
	af3d::rect m_rDstNDC;

	RAS_FLTTYPE m_dSceneAspect;
	
	std::shared_ptr<af3d::camera<>> m_spCamera;
	std::shared_ptr<af3d::perspectiveprojection<>> m_spPerspectiveProjection;
	std::shared_ptr<af3d::vertexshaderscratch<>> m_spVSScratchCache;
	
	std::vector<std::pair<af3d::vertexattsframe<>*,std::shared_ptr<af3d::shadowmap<>>>> m_vShadowMaps;

	void tidyup(void);

	void createcamera(void);
	void createprojection(void);

	void ht(af3d::vertexattsframe<> *pFrame,const af3d::plane<>& worldcameraplane,const af3d::vec3<>& worldpos,const af3d::vec3<>& worlddir,const int nTypes,hittest<>& res)const;
	template <typename F> bool faceht(af3d::vertexattsframe<> *pFrame,const af3d::plane<>& worldcameraplane,const af3d::vec3<>& worldpos,const af3d::vec3<>& worlddir,hittest<>& res)const;

	struct bresenhamClipSpace
	{
		RAS_FLTTYPE z;
		bool bValidate;
		RAS_FLTTYPE dLengthSq;
		const af3d::vec2<int> *pA;
		const af3d::vec2<int> *pB;
		const af3d::vec2<RAS_FLTTYPE> *pClipSpaceA;
		const af3d::vec2<RAS_FLTTYPE> *pClipSpaceB;
	};
	__forceinline void validate(const int nX,const int nY,bresenhamClipSpace& ws)const;
	template <typename PT> __forceinline void bresenham_line(afdib::dib *pDst,const af3d::vec2<int>& a,const af3d::vec2<int>& b,const af3d::vec2<RAS_FLTTYPE>& clipspaceA,const af3d::vec2<RAS_FLTTYPE>& clipspaceB,const af3d::vec4<>& bgra)const;
	template <typename PT> __forceinline bool bresenham_line(int& x, int& y,const int x1, const int y1,const int dx, const int dy,const int sx, const int sy,int& err)const;
	template <typename PT> __forceinline void bresenham_line_plot(afdib::dib *pDst,const af3d::zvertex<> *pZScanline,PT *pScanline,const int x, const int y,bresenhamClipSpace& clipspace,const af3d::vec4<>& bgra)const;
	__forceinline void renderline(afdib::dib *pDst,const af3d::vec4<>& l0,const af3d::vec4<>& l1,const af3d::vec4<>& bgra)const;
	
	template <typename PT> __forceinline void rendercircle(afdib::dib *pDst,const af3d::vec3<>& origin,const int nDim,const af3d::vec4<>& bgra,const af3d::mat4<>& worldtoclip)const;
};
