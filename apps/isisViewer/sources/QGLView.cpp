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

#include "QGLView.h"

namespace isis {
namespace viewer {
	
QGLView::QGLView( std::list<data::Image> imgList, QWidget *parent )
	: QGLWidget( parent ), 
	texname(),
	m_Image( imgList.front() )
{
	x=-1.0;
	
	QObject::connect( this, SIGNAL( mousePressEvent(QMouseEvent*) ), this, SLOT( paint() ) );
}


void QGLView::initializeGL()
{	
	boost::shared_ptr<data::Chunk> ch = m_Image.getChunksAsVector().at(0);
	size_t x = ch->getSizeAsVector()[0];
	size_t y = ch->getSizeAsVector()[1];
	size_t z = ch->getSizeAsVector()[2];
	GLshort *ptr = (GLshort*)calloc(x * y * z , sizeof(GLshort));
	isis::data::scaling_pair scaling = ch->getScalingTo(ch->getTypeID());
	size_t index=-1;
	for (unsigned int k = 0; k<z; k++) {
		for (unsigned int j = 0; j<y;j++) {
			for (unsigned int i = 0; i<x;i++) {
				GLshort val = ch->voxel<GLshort>(i,j,k);
				ptr[index++] = val;
			}
		}
	}
	// 	ch->copyToMem<uint8_t>(ptr,scaling);
	
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texname);
	glBindTexture(GL_TEXTURE_3D, texname);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexImage3D(GL_TEXTURE_3D, 0, 3, x, 
					y, z, 0, GL_LUMINANCE, GL_SHORT, 
					&ch->voxel<GLshort>(0));
					

	
}

void QGLView::paint( )
{
	x+=0.1;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBindTexture(GL_TEXTURE_3D, texname);
	glBegin(GL_QUADS);
	glTexCoord3f(x, 0.0,0.0); glVertex3f(-1.0, -1.0, 0.0);
	glTexCoord3f(x, 0.0,1.0); glVertex3f(-1.0, 1.0, 0.0);
	glTexCoord3f(x, 1.0,1.0); glVertex3f(1.0, 1.0, 0.0);
	glTexCoord3f(x, 1.0,0.0); glVertex3f(1.0, -1.0, 0.0);
	
	glEnd();
	glFlush();
	glDisable(GL_TEXTURE_3D);
	updateGL();
}
	
}}