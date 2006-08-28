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

#include "Zertifikatsspeicher.h"

QFrankSSLZertifikatspeicher::QFrankSSLZertifikatspeicher(QObject* eltern):QObject(eltern)
{
	//Warnung bei Debug
#ifndef QT_NO_DEBUG
	qWarning(trUtf8("WARNUNG Debugversion wird benutzt.\r\nEs können sicherheitsrelevante Daten ausgegeben werden!!").toLatin1().constData());
#endif
	QSettings EinstellungenSystem(QSettings::IniFormat,QSettings::SystemScope,"QSSL","tmp");
	QSettings EinstellungenNutzer(QSettings::IniFormat,QSettings::UserScope,"QSSL","tmp");
	K_SpeicherortSystemweit=EinstellungenSystem.fileName().left(EinstellungenSystem.fileName().lastIndexOf("/"))+"/Zertifikate.db";
	K_SpeicherortBenutzer=EinstellungenNutzer.fileName().left(EinstellungenNutzer.fileName().lastIndexOf("/"))+"/Zertifikate.db";
#ifndef QT_NO_DEBUG
	qDebug("Ablageort des Zertifikatsspeichers:\r\n\tSystemweit:%s\r\n\tBenutzer:%s",K_SpeicherortSystemweit.toAscii().constData(),
																					K_SpeicherortBenutzer.toAscii().constData());
#endif	
}
