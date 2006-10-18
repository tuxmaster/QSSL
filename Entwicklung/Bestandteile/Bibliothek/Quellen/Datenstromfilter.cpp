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
	//SHA256 vom PW=Schlüssel passt pima, da dieser 256 Bit lang sein muss
	EVP_MD_CTX Hash;
	EVP_MD_CTX_init(&Hash);
	QByteArray Passworthash;
	Passworthash.resize(32);//32Byte=256 Bbit
	if(EVP_DigestInit_ex(&Hash,EVP_sha256(),NULL)!=1)
		qFatal("QFrankDatenstromfilter der Hashalgorithmus konnte nicht initialisiert werden.");
	if(EVP_DigestUpdate(&Hash,schluessel->constData(),schluessel->size())!=1)
		qFatal("QFrankDatenstromfilter Passwort konnte nicht in den Hashspeicher geschrieben werden");
	if(EVP_DigestFinal_ex(&Hash,(uchar*)Passworthash.data(),NULL)!=1)
		qFatal("QFrankDatenstromfilter Hash vom PW konnte nicht berechnet werden.");
	EVP_MD_CTX_cleanup(&Hash);
#ifndef QT_NO_DEBUG
	qDebug(qPrintable(QString("QFrankDatenstromfilter: Hash vom Passwort \"%1\":\r\n%2").arg(*schluessel).arg(K_FeldNachHex(Passworthash))));
#endif
	uchar K_IV[]={1,2,3,4,0,0,0,0,0,0,0,0,0,0,0,0};
	K_Verschluesseln=new EVP_CIPHER_CTX();
	K_Entschluesseln=new EVP_CIPHER_CTX();
	EVP_CIPHER_CTX_init(K_Verschluesseln);
	EVP_CIPHER_CTX_init(K_Entschluesseln);
	//Es wird eine AES256 Verschlüsselung im CFB8 Modus benutzt.
	if(EVP_EncryptInit_ex(K_Verschluesseln,EVP_aes_256_cfb8(),NULL,(uchar*)Passworthash.constData(),K_IV)!=1 ||
		EVP_DecryptInit_ex(K_Entschluesseln,EVP_aes_256_cfb8(),NULL,(uchar*)Passworthash.constData(),K_IV)!=1)
		qFatal(qPrintable(trUtf8("QFrankDatenstromfilter der Verschlüsselungsalgorithmus konnte nicht initialisiert werden.","debug")));
	open(K_Quelldatenstrom->openMode());
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
#ifndef QT_NO_DEBUG
	qDebug(qPrintable(trUtf8("QFrankDatenstromfilter öffnen: Modus 0x%1","debug").arg(strommodus,0,16)));
#endif
	bool QuelldatenstromBereit;
	if(K_Quelldatenstrom->isOpen())
	{
		QuelldatenstromBereit=K_Quelldatenstrom->openMode()!=strommodus? false:true;
		if(!QuelldatenstromBereit)
		{
			setErrorString(trUtf8("Der Quelldatenstrom muss im selben Modus geöffnet sein."));
#ifndef QT_NO_DEBUG
			qWarning(qPrintable(trUtf8("QFrankDatenstromfilter öffnen: Quelldatenstrom ist anders geöffnet Modus: 0x%1","debug")
										.arg(K_Quelldatenstrom->openMode(),0,16)));
#endif
		}
	}
	else
	{
		QuelldatenstromBereit=K_Quelldatenstrom->open(strommodus);
		if(!QuelldatenstromBereit)
		{
			setErrorString(trUtf8("Der Quelldatenstrom konnte nicht geöffnet werden."));
#ifndef QT_NO_DEBUG
			qWarning(qPrintable(trUtf8("QFrankDatenstromfilter öffnen: Quelldatenstrom konnte nicht geöffnet werden","debug")));
#endif
		}
	}
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
	//Ist der Strom offen?? wenn nein öffnen
	if(!isOpen())
	{
		//versuchen zu öffnen
		if(!open(K_Quelldatenstrom->openMode()))
		{
#ifndef QT_NO_DEBUG
			qWarning(qPrintable(trUtf8("QFrankDatenstromfilter lesen: der Filter konnte nicht geöffnet werden.","debug")));
#endif
			setErrorString(trUtf8("Stromfilter konnte nicht geöffnet werden."));
			return -1;
		}			
	}
#ifndef QT_NO_DEBUG
	qDebug("QFrankDatenstromfilter: es sollen %i Bytes gelesen werden",(int)maximaleLaenge);
#endif
	int Entschluesselt, EntschluesseltEntgueltig;
	QByteArray Puffer=K_Quelldatenstrom->read(maximaleLaenge);
#ifndef QT_NO_DEBUG
	qDebug("\tDie Quelle lieferte %i Bytes.",Puffer.size());
#endif
	if(EVP_DecryptUpdate(K_Entschluesseln,(uchar*)daten,&Entschluesselt,(uchar*)Puffer.constData(),Puffer.size())!=1)
	{
#ifndef QT_NO_DEBUG
		qWarning(qPrintable(trUtf8("QFrankDatenstromfilter: Entschlüsselung gescheitert","debug")));
#endif
		return -1;
	}
	else
	{
		if(EVP_DecryptFinal_ex(K_Entschluesseln,((uchar*)daten)+Entschluesselt,&EntschluesseltEntgueltig)!=1)
		{
#ifndef QT_NO_DEBUG
			qWarning(qPrintable(trUtf8("QFrankDatenstromfilter: Entgültige Entschlüsselung gescheitert","debug")));
#endif
			return -1;
		}
		else
		{
			Entschluesselt=Entschluesselt+EntschluesseltEntgueltig;
#ifndef QT_NO_DEBUG
			qDebug(qPrintable(trUtf8("QFrankDatenstromfilter: es wurden %1 Bytes entschlüsselt","debug").arg(Entschluesselt)));
			//qDebug(qPrintable(QString("\tDaten: %1").arg(QString(QByteArray(daten,Entschluesselt)))));
#endif
		}
	}
	return Entschluesselt;
}

qint64 QFrankDatenstromfilter::writeData(const char *daten, qint64 maximaleLaenge)
{
	//Ist der Strom offen?? wenn nein öffnen
	if(!isOpen())
	{
		//versuchen zu öffnen
		if(!open(K_Quelldatenstrom->openMode()))
		{
#ifndef QT_NO_DEBUG
			qWarning(qPrintable(trUtf8("QFrankDatenstromfilter schreiben: der Filter konnte nicht geöffnet werden.","debug")));
#endif
			setErrorString(trUtf8("Stromfilter konnte nicht geöffnet werden."));
			return -1;
		}			
	}
#ifndef QT_NO_DEBUG
	qDebug("QFrankDatenstromfilter: es sollen %i Bytes geschrieben werden",(int)maximaleLaenge);
#endif
	int Verschluesselt,VerschluesseltEntgueltig;
	QByteArray Puffer;
	Puffer.resize(maximaleLaenge);
	if(EVP_EncryptUpdate(K_Verschluesseln,(uchar*)Puffer.data(),&Verschluesselt,(uchar*)daten,maximaleLaenge)!=1)
	{
#ifndef QT_NO_DEBUG
		qWarning(qPrintable(trUtf8("QFrankDatenstromfilter: Verschlüsselung gescheitert","debug")));
#endif
		return -1;
	}
	else
	{
		if(EVP_EncryptFinal_ex(K_Verschluesseln,((uchar*)Puffer.data())+Verschluesselt,&VerschluesseltEntgueltig)!=1)
		{
#ifndef QT_NO_DEBUG
			qWarning(qPrintable(trUtf8("QFrankDatenstromfilter: Entgültige Verschlüsselung gescheitert","debug")));
#endif
			return -1;
		}
		else
		{
#ifndef QT_NO_DEBUG
			Verschluesselt=Verschluesselt+VerschluesseltEntgueltig;
			qDebug(qPrintable(trUtf8("QFrankDatenstromfilter: es wurden %1 Bytes verschlüsselt","debug").arg(Verschluesselt)));
			//qDebug(qPrintable(QString("\tDaten: %1").arg(K_FeldNachHex(Puffer))));
#endif
			//Datenauf den Datenträger schreiben.
			return K_Quelldatenstrom->write(Puffer);
		}
	}
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

#ifndef QT_NO_DEBUG
QString QFrankDatenstromfilter::K_FeldNachHex(const QByteArray &feld) const
{
	QString tmp="";
	uchar low,high;
	for(int x=0;x<feld.size();x++)
	{
		//Byte zerlegen
		high=((feld.at(x) & 0xf0) >>4)+0x30;
		low=(feld.at(x) & 0x0f)+0x30;
		if(high>0x39)
			high=high+0x07;
		if(low>0x39)
			low=low+0x07;
		tmp.append(high);
		tmp.append(low);
		tmp.append("-");
	}
	return tmp.left(tmp.size()-1);
}
#endif
