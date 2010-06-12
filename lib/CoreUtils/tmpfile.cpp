/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "tmpfile.h"
#include <stdio.h>
#include "CoreUtils/message.hpp"
#include "CoreUtils/common.hpp"
#include <fstream>
#include <boost/filesystem/operations.hpp>
namespace isis
{
namespace util
{

TmpFile::TmpFile( std::string prefix, std::string sufix )
{
	// @todo critical block - should be locked
	#pragma warning(suppress:4996)
	boost::filesystem::path dummy( tmpnam( NULL ) );
	boost::filesystem::path::operator=( dummy.branch_path() / boost::filesystem::path( prefix + dummy.leaf() + sufix ) );
	LOG( Debug, info ) << "Creating temporary file " << string();
	std::ofstream( string().c_str() );
}

TmpFile::~TmpFile()
{
	if ( boost::filesystem::exists( *this ) ) {
		boost::filesystem::remove( *this );
	} else {
		LOG( Debug, warning ) << "Temporary file " << string() << " does not exist, won't delete it";
	}
}
}
}
