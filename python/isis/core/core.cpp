#include <boost/python.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include "common.hpp"
#include "_application.hpp"
#include "_vector.hpp"
#include "CoreUtils/selection.hpp"

using namespace boost::python;
using namespace isis::python;

BOOST_PYTHON_MODULE( _core )
{
//#######################################################################################
//	Application
//#######################################################################################
	class_<isis::util::Application, _Application>( "Application", init<const char *>() )
		//virtual void printHelp()const;
		.def( "printHelp", &isis::util::Application::printHelp )
		//static const std::string getCoreVersion( void );
		.def( "getCoreVersion", &isis::util::Application::getCoreVersion )
		.staticmethod( "getCoreVersion" )
		//virtual bool init( int argc, char **argv, bool exitOnError = true );
		.def( "init", &_Application::init )
		.def( "addParameter", &_Application::addParameter)
		.def( "setNeeded", &_Application::setNeeded)
		.def( "setHidden", &_Application::setHidden)
	;


//#######################################################################################
//	Selection
//#######################################################################################
	class_<isis::util::Selection>("Selection", init<const char*>())
		.def( init<>())
			;

//#######################################################################################
//	Vector4
//#######################################################################################
	class_<isis::util::fvector4, _Vector4<float> >("fvector4", init<float, float, float, float>())
		.def( init<>())
		.def( init<isis::util::fvector4>())
		.def("__setitem__", &_Vector4<float>::setItem)
		.def("__getitem__", &_Vector4<float>::getItem)
		;
	class_<isis::util::ivector4, _Vector4<int32_t> >("ivector4", init<int32_t, int32_t, int32_t, int32_t>())
		.def( init< isis::util::ivector4 >())
		.def( init<>())
		.def("__setitem__", &_Vector4<int32_t>::setItem)
		.def("__getitem__", &_Vector4<int32_t>::getItem)
		;
	class_<isis::util::dvector4, _Vector4<double> >("dvector4", init<double, double, double, double>())
		.def( init< isis::util::dvector4>() )
		.def("__setitem__", &_Vector4<double>::setItem)
		.def("__getitem__", &_Vector4<double>::getItem)
		.def( init<>())
		;

}
