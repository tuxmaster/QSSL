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


CONFIG	+= qt release
QT		-= gui
QT		+= network
#Mit Debug Infos? ja/nein
Debuginfos	= ja
#strip unter unix/mingw? ja/nein
Strip		= nein

DESTDIR		= ../../bin
MOC_DIR		= ../../tmp/moc
OBJECTS_DIR		= ../../tmp/obj
UI_HEADERS_DIR	= ../../tmp/ui_headers
RCC_DIR		= ../../tmp/resourcen

contains(Debuginfos, ja) {
	message(Erstelle $$PROJEKTTEIL mit Debugmeldungen)
	CONFIG	 -= release
	CONFIG	 += debug
	win32:CONFIG += console

}
contains(Strip, ja) {
#Müll entfernen
QMAKE_POST_LINK	=strip -s $$DESTDIR/$(TARGET)
}
#damit die Datei .dll und nicht 0.dll heist.
win32 {
	contains(TEMPLATE, lib) {
		TARGET_EXT = .dll
}
}
QMAKE_TARGET_COMPANY = Frank Büttner
QMAKE_TARGET_PRODUCT = QSSL
QMAKE_TARGET_DESCRIPTION = C++ Bibliothek für SSL/TLS Unterstützung unter Qt.
QMAKE_TARGET_COPYRIGHT = Copyright (c) 2006 Frank Büttner
