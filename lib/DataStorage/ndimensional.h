//
// C++ Interface: ndimensional
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef NDIMENSIONAL_H
#define NDIMENSIONAL_H

#define	__need_size_t
#include <stddef.h>
#include <algorithm>
#include <boost/static_assert.hpp>
#include <string>
#include <sstream>
#include "CoreUtils/common.hpp"

namespace isis{ namespace data{ namespace _internal{
/// @cond _hidden
	
template<unsigned short DIM> size_t __dimStride(const size_t dim[])
{
	BOOST_STATIC_ASSERT(DIM>0);//Make sure recursion terminates
	return __dimStride<DIM-1>(dim)*dim[DIM-1];
}

template<unsigned short DIM> size_t __dim2Index(const size_t d[],const size_t dim[])
{
	BOOST_STATIC_ASSERT(DIM>0);//Make sure recursion terminates
	return d[DIM]*__dimStride<DIM>(dim)+__dim2Index<DIM-1>(d,dim);
}

template<unsigned short DIM> bool __rangeCheck(const size_t d[],const size_t dim[])
{
	return (d[DIM] < dim[DIM]) && __rangeCheck<DIM-1>(d,dim);
}

template<> size_t __dimStride<0> (                 const size_t dim[]);
template<> size_t __dim2Index<0> (const size_t d[],const size_t dim[]);
template<> bool   __rangeCheck<0>(const size_t d[],const size_t dim[]);
/// @endcond

/// Base class for anything that has dimensional size
template<unsigned short DIMS> class NDimensional{
	size_t dim[DIMS];
protected:
	NDimensional(){}
public:
	/**
	 * Initializes the size-vector.
	 * This must be done before anything else, or behaviour will be undefined.
	 * \param d array with sizes to use
	 */
	void init(const size_t d[DIMS]){
		std::copy(d,d+DIMS,dim);
		//@todo make validity check
	}
	NDimensional(const NDimensional &src){//@todo default copier should do the job
		init(src.dim);
	}
	/**
	 * Compute linear index from n-dimensional index,
	 * \param d array of indexes (d[0] is most iterating element / lowest dimension)
	 */
	size_t dim2Index(const size_t d[DIMS])const
	{
		return __dim2Index<DIMS-1>(d,dim);
	}
	/**
	 * Check if index fits into size of the object.
	 * \param d index to be checked (d[0] is most iterating element / lowest dimension)
	 * \returns true if given index will get a reasonable result when used for dim2index
	 */
	bool rangeCheck(const size_t d[DIMS])const{
		return __rangeCheck<DIMS-1>(d,dim);
	}
	///\returns the whole size of the object in elements of TYPE
	size_t volume()const
	{
	  return __dimStride<DIMS>(dim);
	}
	///\returns the size of the object in the given dimension
	size_t dimSize(size_t idx)const{
		return dim[idx];
	}
	
	/// generates a string representing the size
	std::string sizeToString(std::string delim="x")const{
		std::ostringstream ret;
		size_t rev[DIMS];
		std::reverse_copy(dim,dim+DIMS,rev);
		isis::util::write_list(rev,rev+DIMS,ret,delim);
		return ret.str();
	}
};

}}}

#endif // NDIMENSIONAL_H