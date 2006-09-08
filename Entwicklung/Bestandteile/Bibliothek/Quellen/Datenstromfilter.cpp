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
#include <openssl/evp.h>

QFrankDatenstromfilter::QFrankDatenstromfilter(QIODevice *quelldatenstrom,QString *schluessel):K_Quelldatenstrom(quelldatenstrom)
{
	//Warnung bei DEBUG
#ifndef QT_NO_DEBUG
	qWarning(trUtf8("WARNUNG Debugversion wird benutzt.\r\nEs können sicherheitsrelevante Daten ausgegeben werden!!","debug").toLatin1().constData());
#endif
	//OpenSSL_add_all_algorithms();

	unsigned char K_IV[]={1,2,3,4};

	K_Verschluesseln=new EVP_CIPHER_CTX();
	K_Entschluesseln=new EVP_CIPHER_CTX();
	EVP_CIPHER_CTX_init(K_Verschluesseln);
	EVP_CIPHER_CTX_init(K_Entschluesseln);
	//Es wird eine AES256 Verschlüsselung im CBC Modus benutzt.
	if(EVP_EncryptInit_ex(K_Verschluesseln,EVP_aes_256_cbc(),NULL,(uchar*)qPrintable(*schluessel),K_IV)!=1 ||
		EVP_DecryptInit_ex(K_Entschluesseln,EVP_aes_256_cbc(),NULL,(uchar*)qPrintable(*schluessel),K_IV)!=1)
		qFatal(qPrintable(trUtf8("QFrankDatenstromfilter der Verschlüsselungsalgorithmus konnte nicht initialisiert werden.","debug")));
	schluessel->clear();
}

QFrankDatenstromfilter::~QFrankDatenstromfilter()
{
	if(EVP_CIPHER_CTX_cleanup(K_Verschluesseln)!=1 || EVP_CIPHER_CTX_cleanup(K_Entschluesseln)!=1)
		qFatal("QFrankDatenstromfilter Sicherheitssystem konnte ich entsorgt werden.");
	EVP_cleanup();
	delete K_Verschluesseln;
	delete K_Entschluesseln;
}

bool QFrankDatenstromfilter::open(OpenMode strommodus)
{
	/*
		Der Quelldatenstrom muss im selben Modus geöffnet sei wie das Filter.
		Wenn er nicht geöffnet ist, wird versucht ihn in dem selben Modus zu öffnen wie das Filter
	*/
	bool QuelldatenstromBereit;
	if(K_Quelldatenstrom->isOpen())
		QuelldatenstromBereit=(K_Quelldatenstrom->openMode() !=strommodus);
	else
		QuelldatenstromBereit=K_Quelldatenstrom->open(strommodus);
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
	K_Quelldatenstrom->close();
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
