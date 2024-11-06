#pragma once

#include "3d.h"
#include "3d_face_pos_aux.h"

namespace af3d
{

template <bool H=false> class projectionclipper
{
public:
	enum extenttype {et_left=0x1,et_right=0x2,et_top=0x4,et_bottom=0x8,et_near=0x10,et_far=0x20,
					 et_all=(et_left|et_right|et_top|et_bottom|et_near|et_far)};

	projectionclipper(){}
	~projectionclipper(){}
	
	template <int EXTENTS,typename P> static __forceinline int getinvalidoutcodes(const P& v)
	{
		int nRet=0;
		if(EXTENTS & et_left)
			nRet=nRet|getinvalidoutcode<et_left>(v);
		if(EXTENTS & et_right)
			nRet=nRet|getinvalidoutcode<et_right>(v);
		if(EXTENTS & et_bottom)
			nRet=nRet|getinvalidoutcode<et_bottom>(v);
		if(EXTENTS & et_top)
			nRet=nRet|getinvalidoutcode<et_top>(v);
		if(EXTENTS & et_near)
			nRet=nRet|getinvalidoutcode<et_near>(v);
		if(EXTENTS & et_far)
			nRet=nRet|getinvalidoutcode<et_far>(v);
		return nRet;
	}
	template <int EXTENTS,typename SRC> static __forceinline void getinsideoutside(const SRC& in,bool& bAllInside,bool& bAllOutside)
	{
		bAllInside=false;
		bAllOutside=false;

		if(EXTENTS & et_left)
		{
			int nInside=0;
			if(SRC::size()>1)
			{
				if( getinvalidoutcode<et_left>(in.getclippos().getpos()[0])==0 )++nInside;
				if( getinvalidoutcode<et_left>(in.getclippos().getpos()[1])==0 )++nInside;
			}
			if(SRC::size()>2)
				if( getinvalidoutcode<et_left>(in.getclippos().getpos()[2])==0 )++nInside;
			if(nInside==0) { bAllOutside=true; return; }
			else if(nInside<SRC::size()) return;
		}

		if(EXTENTS & et_right)
		{
			int nInside=0;
			if(SRC::size()>1)
			{
				if( getinvalidoutcode<et_right>(in.getclippos().getpos()[0])==0 )++nInside;
				if( getinvalidoutcode<et_right>(in.getclippos().getpos()[1])==0 )++nInside;
			}
			if(SRC::size()>2)
				if( getinvalidoutcode<et_right>(in.getclippos().getpos()[2])==0 )++nInside;
			if(nInside==0) { bAllOutside=true; return; }
			else if(nInside<SRC::size()) return;
		}

		if(EXTENTS & et_bottom)
		{
			int nInside=0;
			if(SRC::size()>1)
			{
				if( getinvalidoutcode<et_bottom>(in.getclippos().getpos()[0])==0 )++nInside;
				if( getinvalidoutcode<et_bottom>(in.getclippos().getpos()[1])==0 )++nInside;
			}
			if(SRC::size()>2)
				if( getinvalidoutcode<et_bottom>(in.getclippos().getpos()[2])==0 )++nInside;
			if(nInside==0) { bAllOutside=true; return; }
			else if(nInside<SRC::size()) return;
		}

		if(EXTENTS & et_top)
		{
			int nInside=0;
			if(SRC::size()>1)
			{
				if( getinvalidoutcode<et_top>(in.getclippos().getpos()[0])==0 )++nInside;
				if( getinvalidoutcode<et_top>(in.getclippos().getpos()[1])==0 )++nInside;
			}
			if(SRC::size()>2)
				if( getinvalidoutcode<et_top>(in.getclippos().getpos()[2])==0 )++nInside;
			if(nInside==0) { bAllOutside=true; return; }
			else if(nInside<SRC::size()) return;
		}
		
		if(EXTENTS & et_near)
		{
			int nInside=0;
			if(SRC::size()>1)
			{
				if( getinvalidoutcode<et_near>(in.getclippos().getpos()[0])==0 )++nInside;
				if( getinvalidoutcode<et_near>(in.getclippos().getpos()[1])==0 )++nInside;
			}
			if(SRC::size()>2)
				if( getinvalidoutcode<et_near>(in.getclippos().getpos()[2])==0 )++nInside;
			if(nInside==0) { bAllOutside=true; return; }
			else if(nInside<SRC::size()) return;
		}

		if(EXTENTS & et_far)
		{
			int nInside=0;
			if(SRC::size()>1)
			{
				if( getinvalidoutcode<et_far>(in.getclippos().getpos()[0])==0 )++nInside;
				if( getinvalidoutcode<et_far>(in.getclippos().getpos()[1])==0 )++nInside;
			}
			if(SRC::size()>2)
				if( getinvalidoutcode<et_far>(in.getclippos().getpos()[2])==0 )++nInside;
			if(nInside==0) { bAllOutside=true; return; }
			else if(nInside<SRC::size()) return;
		}

		bAllInside=true;
	}
	template <typename int EXTENTS,typename SRC,typename CLIPPED> static __forceinline void clip(const SRC& in,CLIPPED *pOut,CLIPPED *pLocal)
	{
		// sutherland-hodgman
		// assuming vertices are in cuboid clip space i.e. non normalised projection space
		
		// flatten vertices
		pOut->clear();
		if(SRC::size()>1)
		{
			pOut->grow().set(in,0);
			pOut->grow().set(in,1);
		}
		if(SRC::size()>2)
			pOut->grow().set(in,2);
		
		// clip
		clip_ex<EXTENTS,SRC,CLIPPED>(in,pOut,pLocal);
	}
	template <typename SRC,typename CLIPPED,typename TRIANGULATED> static __forceinline void fantriangulate(const SRC& src,
																											const CLIPPED *pIn,
																											TRIANGULATED *pTriangles)
	{
		pTriangles->clear();
		if(pIn->size() < 3) return;
		
		const SRC::template vertex& base = pIn->get()[0];
		for(size_t i = 1; i < pIn->size() - 1; ++i)
			pTriangles->grow().set(src,base,pIn->get()[i],pIn->get()[i+1]);
	}
protected:
	template <typename int EXTENTS,typename SRC,typename CLIPPED> static __forceinline void clip_ex(const SRC& in,CLIPPED *pOut,CLIPPED *pLocal)
	{
		// cuboid plane clip order
		if(EXTENTS&et_left)
			plane_clip<et_left,SRC,CLIPPED>(in,pOut,pLocal);
		if(EXTENTS&et_right)
			plane_clip<et_right,SRC,CLIPPED>(in,pOut,pLocal);
		if(EXTENTS&et_bottom)
			plane_clip<et_bottom,SRC,CLIPPED>(in,pOut,pLocal);
		if(EXTENTS&et_top)
			plane_clip<et_top,SRC,CLIPPED>(in,pOut,pLocal);
		if(EXTENTS&et_near)
			plane_clip<et_near,SRC,CLIPPED>(in,pOut,pLocal);
		if(EXTENTS&et_far)
			plane_clip<et_far,SRC,CLIPPED>(in,pOut,pLocal);
	}
	template <extenttype E,typename SRC,typename CLIPPED> static __forceinline void plane_clip(const SRC& in,CLIPPED *pOut,CLIPPED *pLocal)
	{
		// generic implementation
		(*pLocal)=(*pOut);
		pOut->clear();

		const size_t nLocals=pLocal->size();
		if(nLocals==0)
			return;

		const SRC::template vertex *pLocals=pLocal->get();
		SRC::template vertex const *pS=pLocals+(nLocals-1);

		int nSOutCodes=getinvalidoutcode<E>(pS->clippos.pos.t);

		for(size_t nV=0;nV<nLocals;++nV)
		{
			SRC::template vertex const *pE=pLocals+nV;
			const int nEOutCodes = getinvalidoutcode<E>(pE->clippos.pos.t);

			if(!nEOutCodes)
			{
				// E inside
				if(nSOutCodes)
				{
					// S outside
					pOut->push_back(plane_intersection<E>(in,*pS,*pE));
				}
				pOut->push_back(*pE);
			}
			else
			{
				// E outside
				if(!nSOutCodes)
				{
					// S inside
					pOut->push_back(plane_intersection<E>(in,*pE,*pS));
				}
			}
			pS=pE;
			nSOutCodes=nEOutCodes;
		}
	}
	template <extenttype E,typename SRC> static __forceinline SRC::template vertex plane_intersection(const SRC& in,
																									  const SRC::template vertex& from/*outside*/,
																									  const SRC::template vertex& to/*inside*/)
	{
		// plane Pl from point P and normal N
		// Pl = aP.x + bP.y + cP.z + dP.w = N dot P

		// parametric form of point P on vector (outside-inside)
		// P(t)=from + t * (to-from)

		// find 't' so P(t) lies on plane Pl
		// dir = to - from
		
		// (pl.N dot dir) 0 or close to zero implies (from-to) parallel to plane or lies within the plane
		
		using t_flt=SRC::template t_flt;
		
	    const t_flt epsilon = std::numeric_limits<t_flt>::epsilon();
		const vec4<t_flt>& from_pos=from.clippos.pos.t;
		const vec4<t_flt>& to_pos=to.clippos.pos.t;
		const vec4<t_flt> dir=to_pos-from_pos;

		const t_flt dN_dot_dir = planeN_dot_P<vec4<t_flt>,E>(dir);
		if((dN_dot_dir<0)?(dN_dot_dir>=-epsilon):(dN_dot_dir<=epsilon))
		{
			// clamp
			SRC::template vertex clamped=from;
			switch(E)
			{
				case et_left:clamped.clippos.pos.t[0]=-from_pos[3];break;
				case et_right:clamped.clippos.pos.t[0]=from_pos[3];break;
				case et_bottom:clamped.clippos.pos.t[1]=-from_pos[3];break;
				case et_top:clamped.clippos.pos.t[1]=from_pos[3];break;
				case et_far:clamped.clippos.pos.t[2]=from_pos[3];break;
				case et_near:
					#if (RAS_PARADIGM==RAS_DX_PARADIGM)
						clamped.clippos.pos.t[2]=0;break;
					#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
						clamped.clippos.pos.t[2]=-from_pos[3];break;
					#endif
				break;
				default:break;
			}
			return clamped;
		}

		// t = -(pl.N dot from) / (pl.N dot dir)
		// from if a point on the clip plane
		SRC::template vertex r;
		const t_flt dN_dot_from = planeN_dot_P<vec4<t_flt>,E>(from_pos);
		const t_flt t = (-dN_dot_from)/dN_dot_dir;
		r.lerp(in,from,to,t);
		return r;
	}
	template <extenttype E,typename P> static __forceinline int getinvalidoutcode(const P& p)
	{
		// dont try and visualise 4d cuboid, just think of every point having its own cuboid clipspace whoose extents are described by its w value

		// x								perspective range							-w <= x <= w
		// x								orthographic range							-1 <= x <= 1
		
		// y								perspective range							-w <= y <= w
		// y								orthographic range							-1 <= y <= 1

		// DX  z							perspective range							0 <= z <= w
		// DX  z							orthographic  range							0 <= z <= 1

		// OGL z							perspective range							-w <= z <= w
		// OGL z							orthographic range							-1 <= z <= 1
		const P::template t_flt w=p[3];
		switch(E)
		{
			case et_left:
				if(H)
					return (p[0]>=-w) ? 0 : E;			// homogenous
				return (p[0]>=-1) ? 0 : E;
			case et_right:
				if(H)
					return (p[0]<=w) ? 0 : E;			// homogenous
				return (p[0]<=1) ? 0 : E;
			case et_top:
				if(H)
					return (p[1]<=w) ? 0 : E;			// homogenous
				return (p[1]<=1) ? 0 : E;
			case et_bottom:
				if(H)
					return (p[1]>=-w) ? 0 : E;			// homogenous
				return (p[1]>=-1) ? 0 : E;
			case et_near:
				#if (RAS_PARADIGM==RAS_DX_PARADIGM)
					if(H)
						return (p[2]>=0) ? 0 : E;		// homogenous
					return (p[2]>=0) ? 0 : E;
				#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
					if(H)
						return (p[2]>=-w) ? 0 : E;		// homogenous
					return (p[2]>=-1) ? 0 : E;
				#endif
			break;
			case et_far:
				if(H)
					return (p[2]<=w) ? 0 : E;			// homogenous
				return (p[2]<=1) ? 0 : E;
		}
		return 0;
	}
	template <typename V,extenttype E> static __forceinline V::template t_flt planeN_dot_P(const V& p)
	{
		// x
		// left plane						point		[-w, 0, 0, 0]		normal		[ 1, 0, 0, 1]		N dot P		-W
		// right plane						point		[ w, 0, 0, 0]		normal		[-1, 0, 0, 1]		N dot P		-W

		// y
		// top plane						point		[ 0, w, 0, 0]		normal		[ 0,-1, 0, 1]		N dot P		-W
		// bottom plane						point		[ 0,-w, 0, 0]		normal		[ 0, 1, 0, 1]		N dot P		-W

		// DX  z
		// near plane						point		[ 0, 0, 0, 0]		normal		[ 0, 0, 1, 0]		N dot P		 0
		// far plane						point		[ 0, 0, w, 0]		normal		[ 0, 0,-1, 1]		N dot P		-W

		// OGL z
		// near plane						point		[ 0, 0,-w, 0]		normal		[ 0, 0, 1, 1]		N dot P		-W
		// far plane						point		[ 0, 0, w, 0]		normal		[ 0, 0,-1, 1]		N dot P		-W

		// generalised 4d dot product axbx + ayby + azbz + awbw
		switch(E)
		{
			case et_left:return p[0] + p[3];
			case et_right:return -p[0] + p[3];
			case et_bottom:return p[1] + p[3];
			case et_top:return -p[1] + p[3];
			case et_far:return -p[2] + p[3];
			case et_near:
				#if (RAS_PARADIGM==RAS_DX_PARADIGM)
					return p[2] + 0;
				#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
					return p[2] + p[3];
				#endif
			default:return 0;
		}
	}
};

}
