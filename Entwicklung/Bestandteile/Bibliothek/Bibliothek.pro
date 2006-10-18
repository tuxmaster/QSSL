# Copyright (C) 2006 Frank Büttner frank-buettner@gmx.net
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version
# 2 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

PROJEKTTEIL	= Bibliothek
TEMPLATE	= lib
CONFIG		+= dll
QT			+= xml
include (../../Vorgaben.pri)
VERSION		= 0.1.0
TARGET		= qssl
win32{
	DEFINES		+= DLL_BAUEN	
	INCLUDEPATH	+=../../../../../OpenSSL/0.9.8d/include
	LIBS		+= -L../../../../../OpenSSL/0.9.8d/lib -lssleay32 -llibeay32
}
unix{
	INCLUDEPATH	+= /usr/include
	LIBS		+= -L/usr/lib -lssl
}

TRANSLATIONS	= Uebersetzungen/qssl_en.ts\
				  Uebersetzungen/qssl_XX.ts
				  
!win32:HEADERS =Quellen/Datenstromfilter.h
HEADERS		  +=Quellen/Zertifikatsspeicher.h\ 
			    Quellen/qssl.h
			   
!win32:SOURCES =Quellen/Datenstromfilter.cpp
SOURCES		  +=Quellen/Zertifikatsspeicher.cpp\
			    Quellen/qssl.cpp 