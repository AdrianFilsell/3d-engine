#pragma once

#include <memory>
#include <cmath>
#include <vector>

namespace af
{

template <typename T> constexpr __forceinline T minval( const T a, const T b ){return a < b ? a : b;}
template <typename T> constexpr __forceinline T maxval( const T a, const T b ){return a > b ? a : b;}

template <typename T> constexpr __forceinline int posfloor( const T d ){return int( d );}
template <typename T> constexpr __forceinline int posround( const T d ){return int( 0.5 + d );}
template <typename T> constexpr __forceinline int posceil( const T d ){const int r = int( d );return T( r ) == d ? r : 1 + r;}

template <typename T> static __forceinline int postodiscrete( const T dContinuous ) { return posfloor<T>( dContinuous ); }
template <typename T,typename R> constexpr __forceinline R posround( const T d ) { return R( 0.5 + d ); }

template <typename T> static __forceinline T tocontinuous( const int nDiscrete ) { return nDiscrete + 0.5; }

template <typename T> std::shared_ptr<T> createmutable(std::shared_ptr<const T> sp)
{
	std::shared_ptr<T> spMutable(std::make_unique<T>());
	if(sp)
		*spMutable=*sp;
	return spMutable;
}
template <typename T,typename R> std::shared_ptr<T> pushbackmutable(std::shared_ptr<const T> sp,R r)
{
	auto m = createmutable<T>(sp);
	m->push_back(r);
	return m;
}

template <typename T> constexpr __forceinline T getrecip_2( void )
{
	return T( 0.5 );
}
template <typename T> constexpr __forceinline T getrecip_3( void )
{
	return T( 1 / 3.0 );
}
template <typename T> constexpr __forceinline T getrecip_4( void )
{
	return T( 0.25 );
}

template <typename T> constexpr __forceinline T getpi( void )
{
	return T( 3.1415926535897932384626433832795 );
}
template <typename T> constexpr __forceinline T getradian( const T dDeg )
{
	return ( dDeg / 180.0 ) * getpi<T>();
}

template <typename T> constexpr __forceinline T getdegree( const T dRad )
{
	return ( dRad * 180.0 ) / getpi<T>();
}

template <typename T> __forceinline void posbase_pow_multipliers(const T nonnegexpo,std::vector<bool>& vMultipliers)
{
	long integerExponent = static_cast<long>(nonnegexpo);
	while (integerExponent)
	{
		vMultipliers.push_back((integerExponent % 2 == 1));
		integerExponent /= 2;
	}
}

template <typename T> class nonnegpowerexp
{
public:
	nonnegpowerexp():m_dFracPart(0),m_dExp(0),m_nMultipliers(0){}
	nonnegpowerexp(const T t):nonnegpowerexp()
	{
		m_dExp=t;
		posbase_pow_multipliers(t,m_vMultipliers);
		m_nMultipliers=m_vMultipliers.size();
		const long integerExponent = static_cast<long>(t);
		m_dFracPart=t-T(integerExponent);
	}
	~nonnegpowerexp(){}
	__forceinline T getexp(void)const{return m_dExp;}
	__forceinline T getfracpart(void)const{return m_dFracPart;}
	__forceinline const std::vector<bool>& getmultipliers(void)const{return m_vMultipliers;}
	__forceinline size_t size(void)const{return m_nMultipliers;}
	nonnegpowerexp& operator =(const nonnegpowerexp& o)
	{
		m_dExp=o.m_dExp;
		m_dFracPart=o.m_dFracPart;
		m_nMultipliers=o.m_nMultipliers;
		m_vMultipliers=o.m_vMultipliers;
		return *this;
	}
protected:
	T m_dExp;
	T m_dFracPart;
	size_t m_nMultipliers;
	std::vector<bool> m_vMultipliers;
};

template <typename T> __forceinline T nonneg_pow(const T nonnegbase, const T nonnegexpo)
{
	// Handle special cases
	if (nonnegexpo == 0) return 1;
	if (nonnegbase == 0) return 0;
	if (nonnegbase == 1) return 1;

	// Split exp into integer and fractional parts
	T intPart,fracPart;
//	fracPart = modf(expo, &intPart);
	long integerExponent = static_cast<long>(nonnegexpo);
	intPart=T(integerExponent);
	fracPart=nonnegexpo-intPart;

	// Compute the integer part using exponentiation by squaring
	T result = 1.0;
	T currentMultiplier = nonnegbase;
//	long integerExponent = static_cast<long>(intPart);
//	if (integerExponent < 0)
//	{
//		integerExponent = -integerExponent;
//		currentMultiplier = 1 / nonnegbase;
//	}

//	while (integerExponent > 0)
	while (integerExponent)
	{
		if (integerExponent % 2 == 1)
		{
			result *= currentMultiplier;
		}
		currentMultiplier *= currentMultiplier;
		integerExponent /= 2;
	}

	// Compute the fractional part using standard library function
	if (fracPart != 0)
	{
		result *= exp(log(nonnegbase) * fracPart);
	}

	return result;
}

template <typename T> __forceinline T nonneg_pow(const T nonnegbase, const nonnegpowerexp<T>& nonnegexpo)
{
	// Handle special cases
	if (nonnegexpo.getexp() == 0) return 1;
	if (nonnegbase == 0) return 0;
	if (nonnegbase == 1) return 1;

	// Compute the integer part using exponentiation by squaring
	T result = 1.0;
	T currentMultiplier = nonnegbase;

	for(size_t n=0;n<nonnegexpo.size();++n)
	{
		if (nonnegexpo.getmultipliers()[n])
			result *= currentMultiplier;
		currentMultiplier *= currentMultiplier;
	}

	// Compute the fractional part using standard library function
	if (nonnegexpo.getfracpart() != 0)
		result *= exp(log(nonnegbase) * nonnegexpo.getfracpart());

	return result;
}

}
