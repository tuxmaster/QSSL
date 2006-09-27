/*
 *  Copyright (C) 2006 Frank Büttner frank-buettner@gmx.net
 * 
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *  
 */

#include <QtCore>
#include <qssl.h>

int main(int anzahlArgumente, char *Argumente[]) 
{
	QCoreApplication Programm(anzahlArgumente,Argumente);
	QTranslator QtSystem;
	QTranslator Meine;
	// Ein Gruss an die Doku von Qt 4.1
	QtSystem.load("qt_" + QLocale::system().name().left(QLocale::system().name().indexOf("_")),QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	Meine.load("qsslkonfig_"+QLocale::system().name().left(QLocale::system().name().indexOf("_")),QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	//Meine.load("qsslkonfig_en","bin");
	Programm.installTranslator(&QtSystem);
	Programm.installTranslator(&Meine);
	qDebug(qPrintable(QObject::trUtf8("QSSL Zertifikatsspeicher Konfiguration Version %1, Urheberrecht(©) 2006 Frank Büttner.").arg(QFrankSSL::VersionText())));
	qDebug(qPrintable(QObject::tr("QSSL Zertifikatsspeicher Konfiguration wird OHNE JEGLICHE GARANTIE bereitgestellt.\r\nWeitere Hinweise unter dem Punkt L.")));
	qDebug(qPrintable(QObject::trUtf8("Dies ist freie Software, Sie können sie unter Beachtung der GPL Lizenz weitergeben.")));
	return 0;
}