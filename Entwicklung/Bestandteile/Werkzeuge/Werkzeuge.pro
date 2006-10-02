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

PROJEKTTEIL		= Werkzeuge
TEMPLATE		= app
include (../../Vorgaben.pri)
QT				+= gui
TARGET			= qsslzertkonfig
LIBS			+= -lqssl -L$$DESTDIR

RESOURCES	 	= Resourcen.qrc

INCLUDEPATH		+= ../Bibliothek/Quellen

TRANSLATIONS	= Uebersetzungen/qsslkonfig_en.ts\
				  Uebersetzungen/qsslkonfig_XX.ts
FORMS			= Dialoge/KonfigBasis.ui\
				  Dialoge/GPLBasis.ui\
				  Dialoge/DateiauswahlBasis.ui
HEADERS			= Quellen/DlgGPL.h\
				  Quellen/Ereignisfilter.h\
				  Quellen/DlgDateiauswahl.h\
				  Quellen/DlgHaupt.h
SOURCES			= Quellen/DlgGPL.cpp\
				  Quellen/Ereignisfilter.cpp\
				  Quellen/DlgDateiauswahl.cpp\
				  Quellen/DlgHaupt.cpp\
				  Quellen/haupt.cpp