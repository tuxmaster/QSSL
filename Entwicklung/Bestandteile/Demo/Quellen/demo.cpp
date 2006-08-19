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

int main(int argc, char *argv[])
{
	QCoreApplication Programm(argc,argv);
	QFrankSSL Verbindung(&Programm);
	qDebug()<<"Wir haben folgeden Verschlüsselungsverfahren zur Auswahl:"<<Verbindung.VerfuegbareAlgorithmen().join(":");
	Verbindung.VerfuegbareAlgorithmenFestlegen(QString("DHE-RSA-AES256-SHA:EDH-RSA-DES-CBC3-SHA").split(":"));
	Verbindung.VerbindungHerstellen("localhost",1234);
	//Verbindung.VerbindungHerstellen("192.168.0.2",1234);
	return Programm.exec();
}