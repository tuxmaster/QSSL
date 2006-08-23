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

#include "qssl.h"

QFrankSSL::QFrankSSL(QObject* eltern): QTcpSocket(eltern)
{
	//Warnung bei DEBUG
#ifndef QT_NO_DEBUG
	qWarning("WARNUNG Debugversion wird benutzt.\r\nEs koennen sicherheitsrelevante Daten ausgegeben werden!!");
#endif
	K_KeineOpenSSLStrukturText=trUtf8("OpenSSL Struktur nicht verfügbar\r\n");
	K_KeineSSLStrukturText=trUtf8("SSL Struktur nicht verfügbar\r\n");
	K_SSLServerNichtGefundenText=tr("Der SSL Server wurde nicht gefunden.");
	K_SSLServerVerbindungAbgelehntText=tr("Der SSL Server hat die Verbindung abgelehnt");
	K_SSLServerVerbindungVomServerGetrenntText=tr("Der SSL Server hat die Verbindung getrennt.");
	K_SSLStrukturKonnteNichtErzeugtWerdenText=tr("Die SSL Struktur konnte nicht erzeugt weden.\r\n");
	K_SSL_VerbindungAufgebaut=false;
	K_SSL_Handshake_durchgefuehrt=false;
	K_TunnelBereit=false;
	K_OpenSSLStruktur=NULL;
	K_SSLStruktur=NULL;
	//OpenSSL initialisieren
	SSL_load_error_strings();
	SSL_library_init();
	K_Empfangspuffer=BIO_new(BIO_s_mem());
	K_Sendepuffer=BIO_new(BIO_s_mem());
	//OpenSSL Verbindung aufbauen, hier lassen wir erst mal alle SSL Versionen zu.
	K_OpenSSLStruktur=SSL_CTX_new(SSLv23_client_method());
	if(K_OpenSSLStruktur==NULL)
	{
#ifndef QT_NO_DEBUG
		qDebug("QFrankSSL OpenSSL Struktur konnte nicht erstellt werden.");
		qDebug()<<K_SSLFehlertext();
#endif
		QTimer::singleShot(0,this,SLOT(K_FehlertextSenden()));
		return;
	}
	connect(this,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(K_SocketfehlerAufgetreten(const QAbstractSocket::SocketError)));
	connect(this,SIGNAL(readyRead()),this,SLOT(K_DatenKoennenGelesenWerden()));
	connect(this,SIGNAL(connected()),this,SLOT(K_MitServerVerbunden()));
	
}

QFrankSSL::~QFrankSSL()
{
	//Wenn verbunden dann trennen
	if(state()==QAbstractSocket::ConnectedState)
	{
		//Erst SSL Verbindung trennen wenn vorhanden, und dann vom Server selbst
		//SSL abbauen
		VerbindungTrennen();
		disconnectFromHost();
	}	
	//OpenSSL aufräumen
	if(K_SSLStruktur!=NULL)
		SSL_free(K_SSLStruktur);
	ERR_free_strings();
	if(K_OpenSSLStruktur!=NULL)
	{
		SSL_CTX_free(K_OpenSSLStruktur);
	}
	//BIO_free(K_Sendepuffer);
	//BIO_free(K_Empfangspuffer);
	ERR_free_strings();
	ERR_remove_state(0);
}

const bool QFrankSSL::K_SSLStrukturAufbauen()
{
	if(K_OpenSSLStruktur==NULL)
	{
		K_OpenSSLFehlerText=K_KeineOpenSSLStrukturText+K_SSLFehlertext();
#ifndef QT_NO_DEBUG
		qCritical("QFrankSSL SSL Struktur aufbauen: OpenSSL Struktur fehlt");
		qCritical(K_SSLFehlertext().toAscii().data());
#endif
		K_AllesZuruecksetzen();
		return false;
	}
	K_SSLStruktur=SSL_new(K_OpenSSLStruktur);
	if(K_SSLStruktur==NULL)
	{
		K_OpenSSLFehlerText=K_SSLFehlertext();
#ifndef QT_NO_DEBUG
		qWarning("QFrankSSL SSL Struktur aufbauen: fehlgeschlagen");
		qWarning()<<K_OpenSSLFehlerText;
#endif
		K_AllesZuruecksetzen();
		return false;
	}
#ifndef QT_NO_DEBUG
	qDebug()<<"QFrankSSL SSL Struktur aufbauen: erfolgreich";
#endif
	//Nur hohlen, wenn die Liste leer ist, da sonst die Nutzervorgaben überschrieben werden.
	if(K_VerfuegbareAlgorithmen.isEmpty())
		K_VerfuegbareAlgorithmenHohlen();
	return true;
}

void QFrankSSL::K_VerfuegbareAlgorithmenHohlen()
{
	if(K_SSLStruktur==NULL)
	{
#ifndef QT_NO_DEBUG
		qWarning()<<"QFrankSSL verfuegbare Algorithmen: keine gueltige SSL Struktur";
#endif
		emit SSLFehler(K_KeineSSLStrukturText);
		return;
	}
	const char* Algorithmus=0;
	int Prioritaet=0;
	do
	{
		Algorithmus=SSL_get_cipher_list(K_SSLStruktur,Prioritaet);
		if(Algorithmus==NULL)
			break;
		K_VerfuegbareAlgorithmen<<Algorithmus;
		Prioritaet++;
	}
	while(true);
#ifndef QT_NO_DEBUG
	qDebug()<<"QFrankSSL verfuegbare Algorithmen:"<<K_VerfuegbareAlgorithmen.join(":");
#endif	
}



void QFrankSSL::DatenSenden(const QByteArray &daten)
{
	if(!K_TunnelBereit)
	{
#ifndef QT_NO_DEBUG
		qDebug()<<"QFrankSSL DatenSenden: geht nicht, da Tunnel nicht bereit.";
#endif
		return;
	}
	SSL_write(K_SSLStruktur,daten.data(),daten.size());
	if(K_MussWasGesendetWerden())
		K_DatenSenden();
}

void QFrankSSL::K_DatenKoennenGelesenWerden()
{
	int BytesDa=bytesAvailable();
#ifndef QT_NO_DEBUG
	qDebug()<<QString("QFrankSSL: Es koennen %1 Bytes gelesen werden.").arg(BytesDa);
#endif
	if(!K_SSL_VerbindungAufgebaut)
	{
#ifndef QT_NO_DEBUG
		qDebug()<<"\tAber es besteht keine Verbindung zum SSL Server:(";
#endif
		return;
	}
	if(BytesDa==0)
	{
#ifndef QT_NO_DEBUG
		qDebug()<<"\tEs sollen 0 Byte gelesen werde. Sinnlos.";
#endif
		return;
	}
	 
	BIO_write(K_Empfangspuffer,read(BytesDa).data(),BytesDa);
	//müssen wir senden??
	if(K_MussWasGesendetWerden())
		K_DatenSenden();
	//schauen wir mal ob wir daten haben
	K_EmpfangenenDaten.resize(BytesDa);
	K_SSL_Fehlercode=SSL_read(K_SSLStruktur,K_EmpfangenenDaten.data(),BytesDa);
	if(K_SSL_Fehlercode>0)
	{
#ifndef QT_NO_DEBUG
		qDebug()<<"QFrankSSL: Daten empfangen";
#endif
		if(K_SSL_Fehlercode<BytesDa)
			K_EmpfangenenDaten.resize(K_SSL_Fehlercode);
		emit DatenBereitZumAbhohlen(K_EmpfangenenDaten);
#ifndef QT_NO_DEBUG
		qDebug()<<"QFrankSSL: Daten:"<<K_FeldNachHex(K_EmpfangenenDaten);
		SSL_write(K_SSLStruktur,K_EmpfangenenDaten.data(),K_EmpfangenenDaten.size());
		if(K_MussWasGesendetWerden())
			K_DatenSenden();
#endif
		
	}
	if(K_SSL_Fehlercode<0)
	{
			K_SSL_Fehlercode=SSL_get_error(K_SSLStruktur,K_SSL_Fehlercode);
#ifndef QT_NO_DEBUG
			qDebug()<<"\tSSL_Error ergab:"<<K_SSL_Fehlercode;
#endif
	}
	if(!K_SSL_Handshake_durchgefuehrt)
		K_SSL_Handshake();
	
}

void QFrankSSL::VerbindungHerstellen(const QString &rechnername,const quint16 &port,const OpenMode &betriebsart)
{
	if(!K_SSLStrukturAufbauen())
		return;
	/*if(K_SSLStruktur==NULL)
	{
#ifndef QT_NO_DEBUG
		qWarning("QFrankSSL VerbindungHerstellen: SSL Struktur nicht bereit");
#endif
		emit SSLFehler(K_KeineSSLStrukturText);
		return;
	}*/
		
	/*	setzen der zu benutzenden Verschlüsselungsalgorithmen
		Wichig ist, das die in ansteigener Reihenfolge übergeben werden!!!.
	*/
	if(SSL_set_cipher_list(K_SSLStruktur,K_VerfuegbareAlgorithmen.join(":").toAscii().constData())==0)
	{
#ifndef QT_NO_DEBUG
		qCritical("QFrankSSL kein gueltiger Verschlüsselungsalgorithmus angegeben");
#endif
		K_AllesZuruecksetzen();
		//Liste löschen, da eh ungültig
		K_VerfuegbareAlgorithmen.clear();
		emit SSLFehler(trUtf8("Gewünschter Verschlüsselungsalgorithmus wird von der aktuellen OpenSSL Bibliothek nicht unerstützt!"));
		return;
	}
	connectToHost(rechnername,port,betriebsart);
}

const bool QFrankSSL::K_MussWasGesendetWerden()
{
#ifndef QT_NO_DEBUG
	qDebug()<<"QFrankSSL muessen wir Daten senden?";
#endif
	//Wieviel daten warten auf  Bearbeitung?? >0 sind welche da
	int warteneDaten=BIO_ctrl(K_Sendepuffer,BIO_CTRL_PENDING,0,NULL);
	if (warteneDaten>0)
	{
#ifndef QT_NO_DEBUG
		qDebug()<<"\tja"<< warteneDaten <<"Bytes";
#endif
		return true;
	}
#ifndef QT_NO_DEBUG
	qDebug()<<"\tnein";
#endif
	return false;
}

void QFrankSSL::K_DatenSenden()
{
#ifndef QT_NO_DEBUG
	qDebug("QFrankSSL Daten in den Tunnel schicken");
#endif
	QByteArray Daten;
	int wievielDaten=BIO_ctrl(K_Sendepuffer,BIO_CTRL_PENDING,0,NULL);
	if(wievielDaten<=0)
		return;
	Daten.resize(wievielDaten);
	int zuLesendeDaten=BIO_read(K_Sendepuffer, Daten.data(), wievielDaten);
	if(zuLesendeDaten<0)
		return;
	if(zuLesendeDaten<wievielDaten)
		Daten.resize(zuLesendeDaten);
	write(Daten);

}

void QFrankSSL::K_SSL_Handshake()
{
#ifndef QT_NO_DEBUG
	qDebug("QFrankSSL Handshake");
#endif
	if(K_SSLStruktur>0)
	{
		K_SSL_Handshake_durchgefuehrt=true;
		SSL_do_handshake(K_SSLStruktur);
		if(K_MussWasGesendetWerden())
			K_DatenSenden();
		K_TunnelBereit=true;
		emit TunnelBereit();
	}
}

void QFrankSSL::K_MitServerVerbunden()
{
	if(K_SSLStruktur==NULL)
	{
#ifndef QT_NO_DEBUG
		qWarning("QFrankSSL Verbindung mit dem Server hergestellt: Keine SSL Strukur");
#endif
		K_AllesZuruecksetzen();
		emit SSLFehler(K_KeineSSLStrukturText);
		return;
	}	
#ifndef QT_NO_DEBUG
	qDebug("QFrankSSL Verbindung mit dem Server hergestellt. Baue OpenSSL auf");
#endif
	K_SSL_VerbindungAufgebaut=true;
	
	SSL_set_bio(K_SSLStruktur, K_Empfangspuffer, K_Sendepuffer);
	SSL_set_connect_state(K_SSLStruktur);

	// SSL Verbindung erstellen
	SSL_connect(K_SSLStruktur);
	if(K_MussWasGesendetWerden())
			K_DatenSenden();
}


const QString QFrankSSL::K_SSLFehlertext(const QFrankSSL::ArtDerFehlerquelle &fehlerquelle)const
{
	static QByteArray Fehlerpuffer("x",256);
	switch(fehlerquelle)
	{
		case QFrankSSL::SSL_Bibliothek:
										ERR_error_string_n(ERR_peek_last_error(),Fehlerpuffer.data(),Fehlerpuffer.size());
										return QString(Fehlerpuffer);
										break;
		case QFrankSSL::SSL_Struktur:
										return QString("%1").arg(K_SSL_Fehlercode);
										break;
	}
	return "";
}

void QFrankSSL::K_FehlertextSenden()
{
	emit SSLFehler(K_SSLFehlertext());
}

void QFrankSSL::VerbindungTrennen()
{
	if(K_SSLStruktur==NULL)
		return;
	SSL_shutdown(K_SSLStruktur);
	if(K_MussWasGesendetWerden())
			K_DatenSenden();
}

void QFrankSSL::K_AllesZuruecksetzen()
{	
	if(state()==QAbstractSocket::ConnectedState)
		disconnectFromHost();
	//SSL Struktur löschen
	if(K_SSLStruktur!=NULL)
		SSL_free(K_SSLStruktur);
	K_SSLStruktur=NULL;

}

void QFrankSSL::K_SocketfehlerAufgetreten(const QAbstractSocket::SocketError &fehler)
{
	K_AllesZuruecksetzen();
	switch(fehler)
	{
		case QAbstractSocket::HostNotFoundError:
#ifndef QT_NO_DEBUG
													qWarning("QFrank SSL Verbindung: SSL Server nicht gefunden");
#endif
													emit SSLFehler(K_SSLServerNichtGefundenText);
													break;
		case QAbstractSocket::ConnectionRefusedError:
#ifndef QT_NO_DEBUG
													qWarning("QFrank SSL Verbindung: SSL Server Verbindung abgelehnt");
#endif
													emit SSLFehler(K_SSLServerVerbindungAbgelehntText);
													break;
		case QAbstractSocket::RemoteHostClosedError:
#ifndef QT_NO_DEBUG
													qDebug("QFrank SSL Verbindung: SSL Server Verbindung getrennt");
#endif
													emit SSLFehler(K_SSLServerVerbindungVomServerGetrenntText);
													break;
		default:
													qFatal("QFrank SSL Verbindung: Zustand nicht bearbeitet Code: %i",fehler);
													break;
	}
}

#ifndef QT_NO_DEBUG
QString QFrankSSL::K_FeldNachHex(const QByteArray &feld) const
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
