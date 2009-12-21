//
// C++ Interface: chunk
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef CHUNK_H
#define CHUNK_H

#include "CoreUtils/type.hpp"
#include "CoreUtils/log.hpp"
#include "CoreUtils/propmap.hpp"
#include "common.hpp"
#include <string.h>
#include <list>
#include "ndimensional.h"
#include "propertyobject.h"
#include "CoreUtils/vector.hpp"

namespace isis{namespace data{

namespace _internal{
class ChunkBase :public NDimensional<4>,public PropertyObject{
protected:
	static const char* needed;
public:
	enum dimensions{read=0,phase,slice,time,n_dims};
	static const dimensions dimension[n_dims];
	typedef isis::util::_internal::TypeReference <ChunkBase > Reference;

	ChunkBase(size_t firstDim,size_t secondDim,size_t thirdDim,size_t fourthDim);
	virtual ~ChunkBase(); //needed to make it polymorphic

	isis::util::fvector4 size()const;
};
}
	
/**
 * Main class for four-dimensional random-access data blocks.
 * Like in TypePtr, the copy of a Chunk will reference the same data.
 * (If you want to make a memory based deep copy of a Chunk create a MemChunk from it)
 */
class Chunk : public _internal::ChunkBase, public util::_internal::TypeReference<util::_internal::TypePtrBase>{
protected:
	/**
	 * Creates an data-block from existing data.
	 * \param src is a pointer to the existing data. This data will automatically be deleted. So don't use this pointer afterwards.
	 * \param d is the deleter to be used for deletion of src. It must define operator(TYPE *), which than shall free the given pointer.
	 * \param firstDim size in the first dimension (usually read-encoded dim)
	 * \param secondDim size in the second dimension (usually phase-encoded dim)
	 * \param thirdDim size in the third dimension (usually slice-encoded dim)
	 * \param fourthDim size in the fourth dimension
	 */
	template<typename TYPE,typename D> Chunk(TYPE* src,D d,size_t firstDim,size_t secondDim,size_t thirdDim,size_t fourthDim):
	_internal::ChunkBase(firstDim,secondDim,thirdDim,fourthDim),
	util::_internal::TypeReference<util::_internal::TypePtrBase>(new util::TypePtr<TYPE>(src,volume()))
	{}
public:
	/**
	 * Gets a reference to the element at a given index.
	 * If index is invalid, behaviour is undefined. Most probably it will crash.
	 * If _ENABLE_DATA_DEBUG is true an error message will be send (but it will still crash).
	 */
	template<typename TYPE> TYPE &voxel(size_t firstDim,size_t secondDim=0,size_t thirdDim=0,size_t fourthDim=0){
		const size_t idx[]={firstDim,secondDim,thirdDim,fourthDim};
		if(!rangeCheck(idx)){
			LOG(DataDebug,isis::util::error)
				<< "Index " << firstDim << "|" << secondDim << "|" << thirdDim << "|" << fourthDim
				<< " is out of range (" << sizeToString() << ")";
		}
		util::TypePtr<TYPE> &ret=asTypePtr<TYPE>();
		return ret[dim2Index(idx)];
	}
	/**
	 * Gets a copy of the element at a given index.
	 * \copydetails Chunk::voxel
	 */
	template<typename TYPE> TYPE voxel(size_t firstDim,size_t secondDim=0,size_t thirdDim=0,size_t fourthDim=0)const{
		const size_t idx[]={firstDim,secondDim,thirdDim,fourthDim};
		if(!rangeCheck(idx)){
			LOG(DataDebug,isis::util::error)
			<< "Index " << firstDim << "|" << secondDim << "|" << thirdDim << "|" << fourthDim
			<< " is out of range (" << sizeToString() << ")";
		}
		const util::TypePtr<TYPE> &ret=getTypePtr<TYPE>();
		return ret[dim2Index(idx)];
	}
	template<typename TYPE> util::TypePtr<TYPE> &asTypePtr(){
		return operator*().cast_to_TypePtr<TYPE>();
	}
	template<typename TYPE> const util::TypePtr<TYPE>& getTypePtr()const{
		return operator*().cast_to_TypePtr<TYPE>();
	}
};

/// @cond _internal
namespace _internal{

struct chunk_comarison : public std::binary_function< Chunk, Chunk, bool>{
	virtual bool operator() (const Chunk& a, const Chunk& b)const=0;
};
}
/// @endcond

typedef std::list<Chunk> ChunkList;

/// Chunk class for memory-based buffers
template<typename TYPE> class MemChunk : public Chunk{
public:
	MemChunk(size_t firstDim,size_t secondDim=1,size_t thirdDim=1,size_t fourthDim=1):
	Chunk(
		(TYPE*)malloc(sizeof(TYPE)*fourthDim*thirdDim*secondDim*firstDim),
		typename ::isis::util::TypePtr<TYPE>::BasicDeleter(),
		  firstDim,secondDim,thirdDim,fourthDim
	){}
	/// Creates a deep copy of the given Chunk
	MemChunk(const Chunk &ref):Chunk(ref)
	{
		util::TypePtr<TYPE> rep(
			(TYPE*)malloc(sizeof(TYPE)*ref.volume()),ref.volume(),
			typename ::isis::util::TypePtr<TYPE>::BasicDeleter()
		);
		ref.getTypePtr<TYPE>().deepCopy(rep);
		getTypePtr<TYPE>()=rep;
	}
};
}}
#endif // CHUNK_H
