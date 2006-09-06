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

#include "Datenstromfilter.h"

QFrankDatenstromfilter::QFrankDatenstromfilter(QIODevice *quelldatenstrom):Quelldatenstrom(quelldatenstrom)
{
	//Warnung bei DEBUG
#ifndef QT_NO_DEBUG
	qWarning(trUtf8("WARNUNG Debugversion wird benutzt.\r\nEs können sicherheitsrelevante Daten ausgegeben werden!!","debug").toLatin1().constData());
#endif
}

bool QFrankDatenstromfilter::open(OpenMode strommodus)
{
	/*
		Der Quelldatenstrom muss im selben Modus geöffnet sei wie das Filter.
		Wenn er nicht geöffnet ist, wird versucht ihn in dem selben Modus zu öffnen wie das Filter
	*/
	bool QuelldatenstromBereit;
	if(Quelldatenstrom->isOpen())
		QuelldatenstromBereit=(Quelldatenstrom->openMode() !=strommodus);
	else
		QuelldatenstromBereit=Quelldatenstrom->open(strommodus);
	if(QuelldatenstromBereit)
	{
		setOpenMode(strommodus);
		return true;
	}
	return false;
}

void QFrankDatenstromfilter::close()
{
	//Wenn der Filter geschlossen wird, machen wir auch die darunterliegene Quelle zu.
	Quelldatenstrom->close();
	setOpenMode(QIODevice::NotOpen);
}

qint64 QFrankDatenstromfilter::readData(char *daten,qint64 maximaleLaenge)
{
	return -1;
}

qint64 QFrankDatenstromfilter::writeData(const char *daten, qint64 maximaleLaenge)
{
	return -1;
}
//Beschränkungen für den sequenziellen Datenstrom
bool QFrankDatenstromfilter::seek(qint64 position)
{
	//muss false liefern, da man sequenzielle Ströme nicht durchsuchen kann
	setErrorString(tr("Ein Sequenzieller Datenstrom kennt kein seek!"));
	return false;	
}

bool QFrankDatenstromfilter::reset()
{
	//Squenzieller Datenstrom kennt kein reset
	setErrorString(tr("Ein Sequenzieller Datenstrom kennt kein Reset!"));
	return false;
}
