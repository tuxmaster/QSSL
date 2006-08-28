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
#include "Zertifikatsspeicher.h"

//Open SSL Header
#include <openssl/ssl.h>
#include <openssl/err.h>

QFrankSSL::QFrankSSL(QObject* eltern): QTcpSocket(eltern)
{
	//Warnung bei DEBUG
#ifndef QT_NO_DEBUG
	qWarning(trUtf8("WARNUNG Debugversion wird benutzt.\r\nEs können sicherheitsrelevante Daten ausgegeben werden!!").toLatin1().constData());
#endif
	//Der 1. erstellt den Zertifikatsspeicher
	if(K_Zertifikatspeicher==0)
		K_Zertifikatspeicher=new QFrankSSLZertifikatspeicher(QCoreApplication::instance());
	K_KeineOpenSSLStrukturText=trUtf8("OpenSSL Struktur nicht verfügbar\r\n");
	K_KeineSSLStrukturText=trUtf8("SSL Struktur nicht verfügbar\r\n");
	K_SSLServerNichtGefundenText=tr("Der SSL Server wurde nicht gefunden.");
	K_SSLServerVerbindungAbgelehntText=tr("Der SSL Server hat die Verbindung abgelehnt");
	K_SSLServerVerbindungVomServerGetrenntText=tr("Der SSL Server hat die Verbindung getrennt.");
	K_SSLStrukturKonnteNichtErzeugtWerdenText=tr("Die SSL Struktur konnte nicht erzeugt weden.\r\n");
	K_Verbindungsstatus=QFrankSSL::GETRENNT;
	K_OpenSSLStruktur=NULL;
	K_SSLStruktur=NULL;
	K_ZuBenutzendeSSLVersionen=QFrankSSL::SSLv2|QFrankSSL::SSLv3|QFrankSSL::TLSv1;
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
	long Parameter=0;
	if(!(K_ZuBenutzendeSSLVersionen&QFrankSSL::SSLv2))
		Parameter=Parameter|SSL_OP_NO_SSLv2;
	if(!(K_ZuBenutzendeSSLVersionen&QFrankSSL::SSLv3))
		Parameter=Parameter|SSL_OP_NO_SSLv3;
	if(!(K_ZuBenutzendeSSLVersionen&QFrankSSL::TLSv1))
		Parameter=Parameter|SSL_OP_NO_TLSv1;
#ifndef QT_NO_DEBUG
	qDebug("QFrankSSL SSL Struktur aufbauen: erfolgreich");
	QString SSLOptionen="SSL Optionen:";
	long Optionen=SSL_set_options(K_SSLStruktur,Parameter);
	if(Optionen&SSL_OP_NO_SSLv2)
		SSLOptionen.append("\r\n\tKein SSLv2");
	else
		SSLOptionen.append("\r\n\tSSLv2");
	if(Optionen&SSL_OP_NO_SSLv3)
		SSLOptionen.append("\r\n\tKein SSLv3");
	else
		SSLOptionen.append("\r\n\tSSLv3");
	if(Optionen&SSL_OP_NO_TLSv1)
		SSLOptionen.append("\r\n\tKein TLSv1");
	else
		SSLOptionen.append("\r\n\tTLSv1");
	qDebug(SSLOptionen.toUtf8().constData());
#else
	SSL_set_options(K_SSLStruktur,Parameter);
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
		qWarning(trUtf8("QFrankSSL verfügbare Algorithmen: keine gültige SSL Struktur").toLatin1().constData());
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
	qDebug(trUtf8("QFrankSSL verfügbare Algorithmen:\r\n%1").arg(K_VerfuegbareAlgorithmen.join(":")).toLatin1().constData());
#endif	
}





void QFrankSSL::K_DatenKoennenGelesenWerden()
{
	int BytesDa=bytesAvailable();
#ifndef QT_NO_DEBUG
	qDebug(trUtf8("QFrankSSL: Es können %1 Bytes gelesen werden.").arg(BytesDa).toLatin1().constData());
#endif
	if(K_Verbindungsstatus==QFrankSSL::GETRENNT)
	{
#ifndef QT_NO_DEBUG
		qDebug(QString("\tAber es besteht keine Verbindung zum SSL Server:(").toLatin1().constData());
#endif
		return;
	}
	if(BytesDa==0)
	{
#ifndef QT_NO_DEBUG
		qDebug(QString("\tEs sollen 0 Byte gelesen werde. Sinnlos.").toLatin1().constData());
#endif
		return;
	}
	 
	BIO_write(K_Empfangspuffer,read(BytesDa).data(),BytesDa);
	//müssen wir senden??
	if(K_MussWasGesendetWerden())
		K_DatenSenden();
	QString ServerAntwort;
	switch(K_Verbindungsstatus)
	{
		case QFrankSSL::VERBUNDEN:
									K_EmpfangenenDaten.resize(BytesDa);
									K_SSL_Fehlercode=SSL_read(K_SSLStruktur,K_EmpfangenenDaten.data(),BytesDa);
									if(K_SSL_Fehlercode>0)
									{
#ifndef QT_NO_DEBUG
										qDebug("QFrankSSL: Daten empfangen");
#endif
										if(K_SSL_Fehlercode<BytesDa)
											K_EmpfangenenDaten.resize(K_SSL_Fehlercode);
										emit DatenBereitZumAbhohlen(K_EmpfangenenDaten);
#ifndef QT_NO_DEBUG
										qDebug(QString("QFrankSSL: Daten:\r\n%1").arg(K_FeldNachHex(K_EmpfangenenDaten)).toLatin1().constData());
#endif
									}
									if(K_SSL_Fehlercode<0)
									{
										K_SSL_Fehlercode=SSL_get_error(K_SSLStruktur,K_SSL_Fehlercode);
#ifndef QT_NO_DEBUG
										qDebug(QString("\tSSL_Error ergab:%1").arg(K_SSL_Fehlercode).toLatin1().constData());
#endif									
									}
									break;
		case QFrankSSL::VERBINDEN:
									/*	Wenn die Verbindung mit dem SSL Server steht, Handschlag durchführen
										Es sollte das Wort server hello drin vorkommen.

									*/
									ServerAntwort=SSL_state_string_long(K_SSLStruktur);
									if(ServerAntwort.contains("server hello"))
									{
#ifndef QT_NO_DEBUG
										qDebug(trUtf8("QFrankSSL Daten empfangen: Bereit für den Handschake.\r\nAntwort vom Server: %1").arg(ServerAntwort).toLatin1().constData());
#endif
										K_SSL_Handshake();
									}
									else
									{
#ifndef QT_NO_DEBUG
										qDebug(trUtf8("QFrankSSL Daten empfangen: Nicht bereit für den Handschake.\r\nAntwort vom Server: %1").arg(ServerAntwort).toLatin1().constData());
#endif
									}
									break;
		case QFrankSSL::HANDSCHLAG:
									/*	schauen wir mal ob der Handschlag geklappt hat.
										es sollte der Text read finished oder read server verfify stehen drin stehen.
									*/
									ServerAntwort=SSL_state_string_long(K_SSLStruktur);
									if(ServerAntwort.contains("read finished") || ServerAntwort.contains("read server verfify"))
									{
#ifndef QT_NO_DEBUG
										qDebug(QString("QFrankSSL Daten empfangen: Handshake ok.\r\nAntwort vom Server: %1").arg(ServerAntwort).toLatin1().constData());
#endif
										K_Verbindungsstatus=QFrankSSL::VERBUNDEN;
										emit TunnelBereit();
									}
									else
									{
#ifndef QT_NO_DEBUG
										qDebug(QString("QFrankSSL Daten empfangen: Handshake gescheitert.\r\nAntwort vom Server: %1").arg(ServerAntwort).toLatin1().constData());
#endif
									}
									break;
	}
	
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
	if(K_OpenSSLMitBugs())
	{
		K_AllesZuruecksetzen();
		return;
	}
	if(SSL_set_cipher_list(K_SSLStruktur,K_VerfuegbareAlgorithmen.join(":").toAscii().constData())==0)
	{
#ifndef QT_NO_DEBUG
		qCritical(trUtf8("QFrankSSL kein gültiger Verschlüsselungsalgorithmus angegeben").toLatin1().constData());
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
	qDebug(trUtf8("QFrankSSL müssen wir Daten senden?").toLatin1().constData());
#endif
	//Wieviel daten warten auf  Bearbeitung?? >0 sind welche da
	int warteneDaten=BIO_ctrl(K_Sendepuffer,BIO_CTRL_PENDING,0,NULL);
	if (warteneDaten>0)
	{
#ifndef QT_NO_DEBUG
		qDebug(QString("\tja %1 Bytes").arg(warteneDaten).toLatin1().constData());
#endif
		return true;
	}
#ifndef QT_NO_DEBUG
	qDebug("\tnein");
#endif
	return false;
}

void QFrankSSL::DatenSenden(const QByteArray &daten)
{
	if(K_Verbindungsstatus!=QFrankSSL::VERBUNDEN)
	{
#ifndef QT_NO_DEBUG
		qWarning("QFrankSSL Daten Senden: geht nicht, da Tunnel nicht bereit.");
#endif
		K_AllesZuruecksetzen();
		emit SSLFehler(tr("Der SSL Tunnel ist nicht aufgebaut."));
		return;
	}
#ifndef QT_NO_DEBUG
	qDebug(QString("QFrankSSL Daten Senden Daten: \r\n%1").arg(K_FeldNachHex(daten)).toLatin1().constData());
#endif
	SSL_write(K_SSLStruktur,daten.data(),daten.size());
	if(K_MussWasGesendetWerden())
		K_DatenSenden();
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
	qDebug("QFrankSSL Handshake versuchen");
#endif
	if(K_SSLStruktur>0)
	{
		K_Verbindungsstatus=QFrankSSL::HANDSCHLAG;
		SSL_do_handshake(K_SSLStruktur);
		if(K_MussWasGesendetWerden())
			K_DatenSenden();		
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
	SSL_set_bio(K_SSLStruktur, K_Empfangspuffer, K_Sendepuffer);
	SSL_set_connect_state(K_SSLStruktur);
	K_Verbindungsstatus=QFrankSSL::VERBINDEN;
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

const bool QFrankSSL::K_OpenSSLMitBugs()const
{
	/*	Version 0.9.7c hat ein Bug in der Funktion
		SSL_set_cipher_list() es wird immer 1 geliefert.
		Egal ob der Algorithmus gültig ist oder nicht.
	*/
	//qDebug("0x%X",SSLeay());
	if(SSLeay()==0x00090703f)
	{
#ifndef QT_NO_DEBUG
		qCritical(trUtf8("QFrankSSL Bugprüfung: 0.9.7c gefunden!!!").toLatin1().constData());
#endif
		emit SSLFehler(trUtf8("Die installierte OpenSSL Version: %1 enthält Bugs.\r\nWeitere Hinweise entnehmen Sie bitte der Datei Hinweise.txt")
								.arg(SSLeay_version(SSLEAY_VERSION)));
		return true;
	}
	return false;
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

QFrankSSLZertifikatspeicher* QFrankSSL::K_Zertifikatspeicher=0;

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
