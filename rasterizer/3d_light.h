#pragma once

#include "3d_frame.h"
#include "3d_tbuffer.h"

namespace af3d
{

template <typename T=RAS_FLTTYPE> class light
{
public:
	enum type {t_null,t_spot,t_directional,t_point};
	light()
	{
		m_Type=t_point;
		m_Diffuse = {1,1,1};
		m_Ambient = {0,0,0};
		m_Specular = {0.35,0.35,0.35};
		m_WorldPos = {0,0,-getfwd<T>()*40};
		m_WorldDir = {0,0,getfwd<T>()};
		m_InvWorldDir = -m_WorldDir;
		m_WorldUp = {0,1,0};
		m_dRange = 256;
		m_dConstAttenuation = 1;
		m_dLinearAttenuation = 0;
		m_dQuadraticAttenuation = 0;
		m_dUmbraAngle=20;
		m_dPenumbraAngle=30;
		m_dTheta = cos(af::getradian(m_dUmbraAngle));
		m_dPhi = cos(af::getradian(m_dPenumbraAngle));
	}
	__forceinline type gettype(void)const{return m_Type;}
	__forceinline const vec3<T>& getdiffuse(void)const{return m_Diffuse;}
	__forceinline const vec3<T>& getspecular(void)const{return m_Specular;}
	__forceinline const vec3<T>& getambient(void)const{return m_Ambient;}
	__forceinline const vec3<T>& getworldpos(void)const{return m_WorldPos;}
	__forceinline const vec3<T>& getworlddir(void)const{return m_WorldDir;}
	__forceinline const vec3<T>& getinvworlddir(void)const{return m_InvWorldDir;}
	__forceinline const vec3<T>& getworldup(void)const{return m_WorldUp;}
	__forceinline T getrange(void)const{return m_dRange;}
	__forceinline T getconstattenuation(void)const{return m_dConstAttenuation;}
	__forceinline T getlinearattenuation(void)const{return m_dLinearAttenuation;}
	__forceinline T getquadraticattenuation(void)const{return m_dQuadraticAttenuation;}
	__forceinline T getumbra(void)const{return m_dUmbraAngle;}
	__forceinline T getpenumbra(void)const{return m_dPenumbraAngle;}
	__forceinline T gettheta(void)const{return m_dTheta;}
	__forceinline T getphi(void)const{return m_dPhi;}
	
	void settype(const type t){m_Type=t;}
	void setdiffuse(const vec3<T>& c){m_Diffuse=c;}
	void setspecular(const vec3<T>& c){m_Specular=c;}
	void setambient(const vec3<T>& c){m_Ambient=c;}
	void setworldpos(const vec3<T>& w){m_WorldPos=w;}
	void setworlddir(const vec3<T>& w){m_WorldDir=w;m_InvWorldDir=-m_WorldDir;}
	void setworldup(const vec3<T>& w){m_WorldUp=w;}
	void setrange(const T d){m_dRange=d;}
	void setconstattenuation(const T d){m_dConstAttenuation=d;}
	void setlinearattenuation(const T d){m_dLinearAttenuation=d;}
	void setquadraticattenuation(const T d){m_dQuadraticAttenuation=d;}
	void setumbra(const T d){m_dUmbraAngle=d;m_dTheta = cos(af::getradian(m_dUmbraAngle));}
	void setpenumbra(const T d){m_dPenumbraAngle=d;m_dPhi = cos(af::getradian(m_dPenumbraAngle));}
		
	light& operator =(const light& o)
	{
		m_Diffuse = 0.m_Diffuse;
		m_Ambient = o.m_Ambient;
		m_Specular = o.m_Specular;
		m_WorldPos = o.m_WorldPos;
		m_WorldDir = o.m_WorldDir;
		m_WorldUp = o.m_WorldUp;
		m_dRange = 0.m_dRange;
		m_dConstAttenuation = o.m_dConstAttenuation;
		m_dLinearAttenuation = o.m_dLinearAttenuation;
		m_dQuadraticAttenuation = o.m_dQuadraticAttenuation;
		m_dTheta = o.m_dTheta;
		m_dPhi = o.m_dPhi;
		m_dUmbraAngle = o.m_dUmbraAngle;
		m_dPenumbraAngle = o.m_dPenumbraAngle;
		return *this;
	}

    template <bool QUANTIZE> __forceinline void modulate_inline(const shadowmap<T> *pShadowMap,const vec3<T>& campos,const vec3<T>& fragpos,const vec3<T>& fragnorm,const T dNDCspaceZ,
																const vec3<T>& diffuse,const vec3<T>& ambient,const vec3<T>& specular,const af::nonnegpowerexp<T>& shininess,
																const bool bSum,
																const quantize_static_3<T>& diffuseQ,const quantize_static_3<T>& specularQ,
																T& out_0,T& out_1,T& out_2)const
    {
		switch(m_Type)
		{
			case t_null:break;
			case t_point:point_modulate<QUANTIZE>(pShadowMap,campos,fragpos,fragnorm,dNDCspaceZ,diffuse,ambient,specular,shininess,bSum,diffuseQ,specularQ,out_0,out_1,out_2);break;
			case t_spot:spot_modulate<QUANTIZE>(pShadowMap,campos,fragpos,fragnorm,dNDCspaceZ,diffuse,ambient,specular,shininess,bSum,diffuseQ,specularQ,out_0,out_1,out_2);break;
			case t_directional:directional_modulate<QUANTIZE>(pShadowMap,campos,fragpos,fragnorm,dNDCspaceZ,diffuse,ambient,specular,shininess,bSum,diffuseQ,specularQ,out_0,out_1,out_2);break;
		}
	}
    template <bool QUANTIZE> void modulate_call(const shadowmap<T> *pShadowMap,const vec3<T>& campos,const vec3<T>& fragpos,const vec3<T>& fragnorm,const T dNDCspaceZ,
												const vec3<T>& diffuse,const vec3<T>& ambient,const vec3<T>& specular,const af::nonnegpowerexp<T>& shininess,
												const bool bSum,
												const quantize_static_3<T>& diffuseQ,const quantize_static_3<T>& specularQ,
												T& out_0,T& out_1,T& out_2)const
    {
		switch(m_Type)
		{
			case t_null:break;
			case t_point:point_modulate<QUANTIZE>(pShadowMap,campos,fragpos,fragnorm,dNDCspaceZ,diffuse,ambient,specular,shininess,bSum,diffuseQ,specularQ,out_0,out_1,out_2);break;
			case t_spot:spot_modulate<QUANTIZE>(pShadowMap,campos,fragpos,fragnorm,dNDCspaceZ,diffuse,ambient,specular,shininess,bSum,diffuseQ,specularQ,out_0,out_1,out_2);break;
			case t_directional:directional_modulate<QUANTIZE>(pShadowMap,campos,fragpos,fragnorm,dNDCspaceZ,diffuse,ambient,specular,shininess,bSum,diffuseQ,specularQ,out_0,out_1,out_2);break;
		}
	}

	__forceinline static void reflect(const vec3<T>& I, const vec3<T>& N,vec3<T>& r){reflect(I[0],I[1],I[2],N,r);}
	__forceinline static void reflect(const T IX,const T IY,const T IZ, const vec3<T>& N,vec3<T>& r)
	{
		// Incident Vector I: This is the incoming light direction
		// Normal Vector N: This is the normal at the fragment
		// Reflection Calculation: The reflected vector is calculated by subtracting twice the dot product of N and I scaled by N from I.
		// Ensures that the specular highlights are calculated correctly by determining the reflection direction based on the surface normal and the direction of the incoming light.
		const T d2=N.dot(IX,IY,IZ) * 2.0;
		r[0] = IX - d2 * N[0];
		r[1] = IY - d2 * N[1];
		r[2] = IZ - d2 * N[2];
	}
protected:
	type m_Type;									// light type

	vec3<T> m_Diffuse;								// diffuse color
	vec3<T> m_Specular;								// specular color
	vec3<T> m_Ambient;								// ambient color
	vec3<T> m_WorldPos;								// world space position
	vec3<T> m_WorldDir;								// world space direction
	vec3<T> m_InvWorldDir;							// inverse world space direction
	vec3<T> m_WorldUp;								// world space up
	T m_dRange;										// after this distance the light has no effect ( not used for direction lights )
	T m_dConstAttenuation;							// constant attenuation
	T m_dLinearAttenuation;							// linear attenuation
	T m_dQuadraticAttenuation;						// quadratic attenuation
	T m_dTheta;										// cosine inner angle ( umbra ) of spotlight cone
	T m_dPhi;										// cosine outer angle ( penumbra ) of spotlight cone
	T m_dUmbraAngle;								// inner angle ( umbra ) of spotlight cone
	T m_dPenumbraAngle;								// outer angle ( penumbra ) of spotlight cone
	
	template <type LT,bool QUANTIZE> __forceinline void generic_modulate(const shadowmap<T> *pShadowMap,
																		 const vec3<T>& campos,const vec3<T>& fragpos,const vec3<T>& fragnorm,const T dNDCspaceZ,
																		 const vec3<T>& diffuse,const vec3<T>& ambient,const vec3<T>& specular,const af::nonnegpowerexp<T>& shininess,
																		 const bool bSum,
																		 const quantize_static_3<T>& diffuseQ,const quantize_static_3<T>& specularQ,
																		 T& out_0,T& out_1,T& out_2)const
	{
		// Calculate shadow map intensity
		T dShaodwMapIntensity=1.0;
		if(pShadowMap)
		{
			// depth
			T dShaodwMapDepth=2.0;
			pShadowMap->getdepth(fragpos,dShaodwMapDepth);

			// bias, typical constant values [0.0005,0.005]
			const T dBias=0.003;
			if(dShaodwMapDepth+dBias < dNDCspaceZ)
			{
				// in shadow
				dShaodwMapIntensity=0;
			}
		}
		
		// Diffuse shading
		T dClampedDot,distance,attenuation;
		const bool bSpecular=shininess.getexp()>0;
		vec3<T> reflectDir,spot_pnt_lightDir,spot_pnt_lightDirNorm;
		switch(LT)
		{
			case t_point:
			case t_spot:
			{
				spot_pnt_lightDir[0]=m_WorldPos[0] - fragpos[0];
				spot_pnt_lightDir[1]=m_WorldPos[1] - fragpos[1];
				spot_pnt_lightDir[2]=m_WorldPos[2] - fragpos[2];
				distance = spot_pnt_lightDir.getlength();
				if(distance>m_dRange)
				{
					if(!bSum)
					{
						out_0 = 0;
						out_1 = 0;
						out_2 = 0;
					}
					return;
				}
				spot_pnt_lightDirNorm=spot_pnt_lightDir;
				if(distance)
				{
					const T dRecip=1/distance;
					spot_pnt_lightDirNorm[0]=spot_pnt_lightDir[0]*dRecip;
					spot_pnt_lightDirNorm[1]=spot_pnt_lightDir[1]*dRecip;
					spot_pnt_lightDirNorm[2]=spot_pnt_lightDir[2]*dRecip;
				}
				dClampedDot=fragnorm.dot(spot_pnt_lightDirNorm);
				
				if(bSpecular)
					reflect(-spot_pnt_lightDirNorm[0],-spot_pnt_lightDirNorm[1],-spot_pnt_lightDirNorm[2],fragnorm,reflectDir); // Using the custom reflect function

				attenuation = 1.0 / (m_dConstAttenuation + m_dLinearAttenuation * distance + m_dQuadraticAttenuation  * (distance * distance));
			}
			break;
			case t_directional:
			{
				dClampedDot=fragnorm.dot(m_InvWorldDir);

				if(bSpecular)
					reflect(m_WorldDir[0],m_WorldDir[1],m_WorldDir[2],fragnorm,reflectDir); // Using the custom reflect function
			}
			break;
			default:return;
		}

		// Quantization threshold
		if(QUANTIZE && !diffuseQ.isempty())
			dClampedDot=diffuseQ.quantize(dClampedDot);
		else
		if(dClampedDot<0)
			dClampedDot=0;

		// Ambient
		vec3<T> a;
		a[0]=m_Ambient[0] * ambient[0];
		a[1]=m_Ambient[1] * ambient[1];
		a[2]=m_Ambient[2] * ambient[2];
		
		// Diffuse
		vec3<T> d;
		d[0]=( m_Diffuse[0] * diffuse[0] ) * dClampedDot;
		d[1]=( m_Diffuse[1] * diffuse[1] ) * dClampedDot;
		d[2]=( m_Diffuse[2] * diffuse[2] ) * dClampedDot;

		// Specular, and result
		vec3<T> s;
		if(bSum)
		{
			vec3<T> result;
			if(bSpecular)
			{
				vec3<T> viewDir;
				viewDir[0]=campos[0] - fragpos[0];
				viewDir[1]=campos[1] - fragpos[1];
				viewDir[2]=campos[2] - fragpos[2];
				viewDir.normalize();

				T dClampedt=viewDir.dot(reflectDir);
				if(dClampedt<0)
					dClampedt=0;
				//const T spec = pow(dt<0.0?0.0:dt,shininess.getexp());
				const T spec = QUANTIZE && !specularQ.isempty()?specularQ.quantize(af::nonneg_pow(dClampedt,shininess)):af::nonneg_pow(dClampedt,shininess); // Quantize the specular intensity

				s[0] = spec * m_Specular[0] * specular[0];
				s[1] = spec * m_Specular[1] * specular[1];
				s[2] = spec * m_Specular[2] * specular[2];
			
				switch(LT)
				{
					case t_spot:
					{
						const T theta = spot_pnt_lightDirNorm.dot(m_InvWorldDir);
						const T epsilon = m_dTheta - m_dPhi;
						T intensity=(theta - m_dPhi) / epsilon;
						if(intensity<0)
							intensity=0;
						else
						if(intensity>1)
							intensity=1;

						result[0]=( a[0] + intensity * (d[0] + s[0]) ) * attenuation * dShaodwMapIntensity;
						result[1]=( a[1] + intensity * (d[1] + s[1]) ) * attenuation * dShaodwMapIntensity;
						result[2]=( a[2] + intensity * (d[2] + s[2]) ) * attenuation * dShaodwMapIntensity;
					}
					break;
					case t_point:
					{
						result[0]=( a[0] + (d[0] + s[0]) ) * attenuation * dShaodwMapIntensity;
						result[1]=( a[1] + (d[1] + s[1]) ) * attenuation * dShaodwMapIntensity;
						result[2]=( a[2] + (d[2] + s[2]) ) * attenuation * dShaodwMapIntensity;
					}
					break;
					case t_directional:
					{
						result[0]=( a[0] + (d[0] + s[0]) ) * dShaodwMapIntensity;
						result[1]=( a[1] + (d[1] + s[1]) ) * dShaodwMapIntensity;
						result[2]=( a[2] + (d[2] + s[2]) ) * dShaodwMapIntensity;
					}
					break;
				}
				posclamp(1,result);
			}
			else
			{
				switch(LT)
				{
					case t_spot:
					{
						const T theta = spot_pnt_lightDirNorm.dot(m_InvWorldDir);
						const T epsilon = m_dTheta - m_dPhi;
						T intensity=(theta - m_dPhi) / epsilon;
						if(intensity<0)
							intensity=0;
						else
						if(intensity>1)
							intensity=1;

						result[0]=( a[0] + intensity * d[0] ) * attenuation * dShaodwMapIntensity;
						result[1]=( a[1] + intensity * d[1] ) * attenuation * dShaodwMapIntensity;
						result[2]=( a[2] + intensity * d[2] ) * attenuation * dShaodwMapIntensity;
					}
					break;
					case t_point:
					{
						result[0]=( a[0] + d[0] ) * attenuation * dShaodwMapIntensity;
						result[1]=( a[1] + d[1] ) * attenuation * dShaodwMapIntensity;
						result[2]=( a[2] + d[2] ) * attenuation * dShaodwMapIntensity;
					}
					break;
					case t_directional:
					{
						result[0]=( a[0] + d[0] ) * dShaodwMapIntensity;
						result[1]=( a[1] + d[1] ) * dShaodwMapIntensity;
						result[2]=( a[2] + d[2] ) * dShaodwMapIntensity;
					}
					break;
				}
			}

			// Final color
			out_0 += result[0];
			out_1 += result[1];
			out_2 += result[2];
		
			return;
		}

		if(bSpecular)
		{
			vec3<T> viewDir;
			viewDir[0]=campos[0] - fragpos[0];
			viewDir[1]=campos[1] - fragpos[1];
			viewDir[2]=campos[2] - fragpos[2];
			viewDir.normalize();

			T dClampedt=viewDir.dot(reflectDir);
			if(dClampedt<0)
				dClampedt=0;
			//const T spec = pow(dt<0.0?0.0:dt,shininess.getexp());
			const T spec = QUANTIZE && !specularQ.isempty()?specularQ.quantize(af::nonneg_pow(dClampedt,shininess)):af::nonneg_pow(dClampedt,shininess); // Quantize the specular intensity

			s[0] = spec * m_Specular[0] * specular[0];
			s[1] = spec * m_Specular[1] * specular[1];
			s[2] = spec * m_Specular[2] * specular[2];
			
			switch(LT)
			{
				case t_spot:
				{
					const T theta = spot_pnt_lightDirNorm.dot(m_InvWorldDir);
					const T epsilon = m_dTheta - m_dPhi;
					T intensity=(theta - m_dPhi) / epsilon;
					if(intensity<0)
						intensity=0;
					else
					if(intensity>1)
						intensity=1;
					
					out_0=( a[0] + intensity * (d[0] + s[0]) ) * attenuation * dShaodwMapIntensity;
					out_1=( a[1] + intensity * (d[1] + s[1]) ) * attenuation * dShaodwMapIntensity;
					out_2=( a[2] + intensity * (d[2] + s[2]) ) * attenuation * dShaodwMapIntensity;
				}
				break;
				case t_point:
				{
					out_0=( a[0] + (d[0] + s[0]) ) * attenuation * dShaodwMapIntensity;
					out_1=( a[1] + (d[1] + s[1]) ) * attenuation * dShaodwMapIntensity;
					out_2=( a[2] + (d[2] + s[2]) ) * attenuation * dShaodwMapIntensity;
				}
				break;
				case t_directional:
				{
					out_0=( a[0] + (d[0] + s[0]) ) * dShaodwMapIntensity;
					out_1=( a[1] + (d[1] + s[1]) ) * dShaodwMapIntensity;
					out_2=( a[2] + (d[2] + s[2]) ) * dShaodwMapIntensity;
				}
				break;
			}
			posclamp(1,out_0,out_1,out_2);
		}
		else
		{
			switch(LT)
			{
				case t_spot:
				{
					const T theta = spot_pnt_lightDirNorm.dot(m_InvWorldDir);
					const T epsilon = m_dTheta - m_dPhi;
					T intensity=(theta - m_dPhi) / epsilon;
					if(intensity<0)
						intensity=0;
					else
					if(intensity>1)
						intensity=1;
					
					out_0=( a[0] + intensity * d[0] ) * attenuation * dShaodwMapIntensity;
					out_1=( a[1] + intensity * d[1] ) * attenuation * dShaodwMapIntensity;
					out_2=( a[2] + intensity * d[2] ) * attenuation * dShaodwMapIntensity;
				}
				break;
				case t_point:
				{
					out_0=( a[0] + d[0] ) * attenuation * dShaodwMapIntensity;
					out_1=( a[1] + d[1] ) * attenuation * dShaodwMapIntensity;
					out_2=( a[2] + d[2] ) * attenuation * dShaodwMapIntensity;
				}
				break;
				case t_directional:
				{
					out_0=( a[0] + d[0] ) * dShaodwMapIntensity;
					out_1=( a[1] + d[1] ) * dShaodwMapIntensity;
					out_2=( a[2] + d[2] ) * dShaodwMapIntensity;
				}
				break;
			}
		}
	}
	template <bool QUANTIZE> __forceinline void point_modulate(const af3d::shadowmap<T> *pShadowMap,const vec3<T>& campos,const vec3<T>& fragpos,const vec3<T>& fragnorm,const T dNDCspaceZ,
															   const vec3<T>& diffuse,const vec3<T>& ambient,const vec3<T>& specular,const af::nonnegpowerexp<T>& shininess,
															   const bool bSum,
															   const quantize_static_3<T>& diffuseQ,const quantize_static_3<T>& specularQ,
															   T& out_0,T& out_1,T& out_2)const
	{
		// emits light equally in all directions from a single point in space
		generic_modulate<t_point,QUANTIZE>(pShadowMap,campos,fragpos,fragnorm,dNDCspaceZ,diffuse,ambient,specular,shininess,bSum,diffuseQ,specularQ,out_0,out_1,out_2);
	}
	template <bool QUANTIZE> __forceinline void spot_modulate(const shadowmap<T> *pShadowMap,const vec3<T>& campos,const vec3<T>& fragpos,const vec3<T>& fragnorm,const T dNDCspaceZ,
															  const vec3<T>& diffuse,const vec3<T>& ambient,const vec3<T>& specular,const af::nonnegpowerexp<T>& shininess,
															  const bool bSum,
															  const quantize_static_3<T>& diffuseQ,const quantize_static_3<T>& specularQ,
															  T& out_0,T& out_1,T& out_2)const
	{
		// emits light cone from a single point in space
		generic_modulate<t_spot,QUANTIZE>(pShadowMap,campos,fragpos,fragnorm,dNDCspaceZ,diffuse,ambient,specular,shininess,bSum,diffuseQ,specularQ,out_0,out_1,out_2);
	}
	template <bool QUANTIZE> __forceinline void directional_modulate(const shadowmap<T> *pShadowMap,const vec3<T>& campos,const vec3<T>& fragpos,const vec3<T>& fragnorm,const T dNDCspaceZ,
																	 const vec3<T>& diffuse,const vec3<T>& ambient,const vec3<T>& specular,const af::nonnegpowerexp<T>& shininess,
																	 const bool bSum,
																	 const quantize_static_3<T>& diffuseQ,const quantize_static_3<T>& specularQ,
																	 T& out_0,T& out_1,T& out_2)const
	{
		// emits light equally in a directions from an infinite distance from fragment
		generic_modulate<t_directional,QUANTIZE>(pShadowMap,campos,fragpos,fragnorm,dNDCspaceZ,diffuse,ambient,specular,shininess,bSum,diffuseQ,specularQ,out_0,out_1,out_2);
	}
	__forceinline void posclamp(const T d,vec3<T>& r)const{posclamp(d,r[0],r[1],r[2]);}
	__forceinline void posclamp(const T d,T& r0,T& r1,T& r2)const
	{
		if(r0>d)r0=d;
		if(r1>d)r1=d;
		if(r2>d)r2=d;
	}
};

template <typename T> class lightcache
{
public:
	lightcache(light<T> *pLight)
	{
		m_nLights=pLight?1:0;
		if(m_nLights)
		{
			m_vLights.resize(1);
			m_vLights[0]=pLight;
		}
		m_pShadowMaps=nullptr;
	}
	lightcache(const std::vector<vertexattsframe<T>*> *pLights,const std::vector<std::pair<vertexattsframe<T>*,std::shared_ptr<shadowmap<T>>>> *pShadowMaps)
	{
		m_nLights=pLights?pLights->size():0;
		m_vLights.resize(m_nLights);
		for(int n=0;n<m_nLights;++n)
			m_vLights[n]=dynamic_cast<af3d::lightmeshcache<T>*>((*pLights)[n])->getlight();
		m_pShadowMaps=pLights && pShadowMaps && pShadowMaps->size()>=pLights->size()?pShadowMaps:nullptr;
	}
	~lightcache(){}
    template <bool QUANTIZE> __forceinline void modulate(const vec3<T>& campos,const vec3<T>& fragpos,const vec3<T>& fragnorm,const T dNDCspaceZ,
														 const vec3<T>& diffuse,const vec3<T>& ambient,const vec3<T>& specular,const af::nonnegpowerexp<T>& shininess,
														 const quantize_static_3<T>& diffuseQ,const quantize_static_3<T>& specularQ,
														 vec3<T>& out)const
    {
		if(m_nLights<1)
		{
			out[0]=0;
			out[1]=0;
			out[2]=0;
			return;
		}

		const bool bMultiple=(m_nLights>1);

		vec3<T> diff;
		if(bMultiple)
		{
			diff[0]=diffuse[0];
			diff[1]=diffuse[1];
			diff[2]=diffuse[2];
		}

		for(int n=0;n<m_nLights;++n)
		{
			const bool bSum=n>0;
			const shadowmap<T> *pShadowMap=m_pShadowMaps?(*m_pShadowMaps)[n].second.get():nullptr;
			
			#ifdef _DEBUG
				vec3<T> pre;
				if(bSum)
					pre=out;
			#endif

			m_vLights[n]->modulate_inline<QUANTIZE>(pShadowMap,campos,fragpos,fragnorm,dNDCspaceZ,bMultiple?diff:diffuse,ambient,specular,shininess,bSum,diffuseQ,specularQ,out[0],out[1],out[2]);

			// clamp
			if(bSum)
			{
				#ifdef _DEBUG
					ASSERT(!(out[0]<pre[0] || out[1]<pre[1] || out[2]<pre[2]));
				#endif

				int nBGRCeil=0;
				if(out[0]>=1) { ++nBGRCeil; }
				if(out[1]>=1) { ++nBGRCeil; }
				if(out[2]>=1) { ++nBGRCeil; }
				if(nBGRCeil==3)
					break;
			}
		}

		if(m_nLights>1)
		{
			if(out[0]>1) { out[0]=1; }
			if(out[1]>1) { out[1]=1; }
			if(out[2]>1) { out[2]=1; }
		}
	}
protected:
	size_t m_nLights;
	std::vector<light<T>*> m_vLights;
	const std::vector<std::pair<vertexattsframe<T>*,std::shared_ptr<shadowmap<T>>>> *m_pShadowMaps;
};

}
