#ifndef ISISTYPE_HPP
#define ISISTYPE_HPP

#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include "log.hpp"

namespace isis{ 
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{

template<typename TYPE> class Type;
template<typename TYPE> class TypePtr;

/// @cond _internal
namespace _internal{
template<typename TYPE,typename T> TYPE __cast_to(Type<TYPE> *dest,const T& value){
	return boost::lexical_cast<TYPE>(value);
}
template<typename TYPE> TYPE __cast_to(Type<TYPE> *dest,const TYPE& value){
	return value;
}

class TypeBase{
	template<typename T> const T m_cast_to(T defaultVal) const{
		MAKE_LOG(CoreLog);
		const T* ret=dynamic_cast<const T* >(this);
		if(ret){
			return *ret;
		} else {
			LOG(CoreLog,error) << "Cannot cast " << typeName() << " to " << T::staticName() << ". Returning \"" << defaultVal.toString() << "\"." << std::endl;
			return defaultVal;
		}
	}
	template<typename T> T& m_cast_to() throw(std::bad_cast){
		MAKE_LOG(CoreLog);
		T* ret=dynamic_cast<T* >(this);
		if(!ret){
			LOG(CoreLog,0) << "Cannot cast " << typeName() << " to " << T::staticName() << ". Throwing bad_cast"<< std::endl;
			throw(std::bad_cast());
		}
		return *ret;
	}
	
public:
	/// Returns true if the stored value is of type T.
	template<typename T> bool is(){return is(typeid(T));}
	virtual bool is(const std::type_info & t)const = 0;
	/// Returns the value represented as text.
	virtual std::string toString(bool labeled=false)const=0;
	/// Returns the name of its actual type
	virtual std::string typeName()const=0;
	/// Returns the id of its actual type
	virtual unsigned short typeID()const=0;

	/**
	 * Lexically cast the TypeBase up to any Type\<T\>.
	 * This is a runtime-based cast via string. The value is converted into a string, which is then parsed as T.
	 * If you know the type of source and destination at compile time you should use Type<DEST_TYPE>((SOURCE_TYPE)src).
	 */
	template<class T> T as()const{
		MAKE_LOG(CoreLog);
		if(typeID() & 0xFF00){
			LOG(CoreLog,warning) 
				<< "You're trying to lexically cast a pointer (" 
				<< typeName() << ") to a value ("<< Type<T>::staticName() 
				<< ") this is most likely nonsense" << std::endl;
		}
		Type<T> ret(this->toString());
		return (T)ret;
	}

	/**
	 * Dynamically cast the TypeBase up to its actual Type\<T\>. Constant version.
	 * Will return a copy of the stored value.
	 * Will return T() if T is not the actual type.
	 * Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	 */
	template<typename T> const Type<T> cast_to_type() const{
		return m_cast_to<Type<T> >(Type<T>(T()));
	}
	/**
	 * Dynamically cast the TypeBase up to its actual TypePtr\<T\>. Constant version.
	 * Will return a copy of the pointer.
	 * Will return TypePtr\<T\\>(NULL) if T is not the actual type.
	 * Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	 */
	template<typename T> const TypePtr<T> cast_to_type_ptr() const{
		return m_cast_to<TypePtr<T> >(TypePtr<T>((T*)0));
	}
	/**
	 * Dynamically cast the TypeBase up to its actual Type\<T\>. Referenced version.
	 * Will return a reference of the stored value.
	 * Will throw std::bad_cast if T is not the actual type.
	 * Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	 */
	template<typename T> Type<T>& cast_to_type() throw(std::bad_cast){
		return m_cast_to<Type<T> >();
	}
	//@todo do we need this
	template<typename T> TypePtr<T>& cast_to_type_ptr() throw(std::bad_cast){
		return m_cast_to<TypePtr<T> >();
	}
};
}
/// @endcond

/// Generic class for type aware variables
template<typename TYPE> class Type: public _internal::TypeBase{
	template<typename T> Type(const TypePtr<T>& value); // Dont do this
	TYPE m_val;
	static std::string m_typeName;
	static unsigned short m_typeID;

public:
	template<typename T> Type(const T& value){
		m_val = __cast_to(this,value);
	}
	virtual bool is(const std::type_info & t)const{
		return t==typeid(TYPE);
	}
	virtual std::string toString(bool labeled=false)const{
		std::string ret=boost::lexical_cast<std::string>(m_val);
		if(labeled)ret+="("+staticName()+")";
		return ret;
	}
	virtual std::string typeName()const{
		return staticName();
	}
	virtual unsigned short typeID()const{
		return staticId();
	}
	static unsigned short staticId(){return m_typeID;}
	static std::string staticName(){return m_typeName;}
	operator TYPE()const{return m_val;}

	virtual ~Type(){}
};

/// Generic class for type (and length) - aware pointers
template<typename TYPE> class TypePtr: public _internal::TypeBase{
	boost::shared_ptr<TYPE> m_val;
	static std::string m_typeName;
	static unsigned short m_typeID;
	const size_t m_len;
	template<typename T> TypePtr(const Type<T>& value); // Dont do this
public:
	struct BasicDeleter{
		virtual void operator()(TYPE *p){
			MAKE_LOG(CoreDebug);
			LOG(CoreDebug,info) << "Freeing pointer " << p << " (" << TypePtr<TYPE>::staticName() << ") " << std::endl;
			free(p);
		};
	};
	struct ObjectArrayDeleter{
		virtual void operator()(TYPE *p){
			MAKE_LOG(CoreDebug);
			LOG(CoreDebug,info) << "Deleting object " << p << " (" << TypePtr<TYPE>::staticName() << ") " << std::endl;
			delete p;
		};
	};
	TypePtr(TYPE* ptr,size_t len):m_val(ptr,BasicDeleter()),m_len(len){}
	template<typename D> TypePtr(TYPE* ptr,size_t len,D d):m_val(ptr,d),m_len(len){}
	virtual bool is(const std::type_info & t)const{
		return t==typeid(TYPE);
	}
	virtual std::string toString(bool labeled=false)const{
		std::string ret;
		if(m_len){
			const TYPE* ptr=m_val.get();
			for(size_t i=0;i<m_len-1;i++)
				ret+=Type<TYPE>(ptr[i]).toString(false)+"|";
			ret+=Type<TYPE>(ptr[m_len-1]).toString(true);
		}
		//@todo implement me
		return boost::lexical_cast<std::string>(m_len) +"#"+ret;
	}
	virtual std::string typeName()const{
		return staticName();
	}
	virtual unsigned short typeID()const{
		return staticId();
	}
	static unsigned short staticId(){return m_typeID;}
	static std::string staticName(){return m_typeName;}
	
	/**
	Returns reference to element at at given index.
	If index is invalid, behaviour is undefined. Probably it will crash.
	If _ENABLE_DATA_DEBUG is true an error message will be send (but it will still crash).
	*/
	
	TYPE& operator[](size_t idx){
		return (m_val.get())[idx];
	}
	operator boost::shared_ptr<TYPE>(){return m_val;}
};

}
/** @} */
}

#endif //DATATYPE_INC
