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

//Open SSL Header
#include <openssl/ssl.h>
#include <openssl/err.h>

QFrankSSL::QFrankSSL(QObject* eltern): QTcpSocket(eltern)
{
	//Der 1. läd die Übersetzungen
	if(K_Uebersetzung==0)
	{
		K_Uebersetzung=new QTranslator(this);
		K_Uebersetzung->load("qssl_"+QLocale::system().name().left(QLocale::system().name().indexOf("_")),QLibraryInfo::location(QLibraryInfo::TranslationsPath));
		//K_Uebersetzung->load("qssl_en",QLibraryInfo::location(QLibraryInfo::TranslationsPath));
		QCoreApplication::instance()->installTranslator(K_Uebersetzung);
	}
	//Warnung bei DEBUG
#ifndef QT_NO_DEBUG
	qWarning(qPrintable(trUtf8("WARNUNG Debugversion wird benutzt.\r\nEs können sicherheitsrelevante Daten ausgegeben werden!!","debug")));
#endif
	//Der 1. erstellt den Zertifikatsspeicher und initialisiert OpenSSL
	if(K_Zertifikatspeicher==0)
		K_Zertifikatspeicher=new QFrankSSLZertifikatspeicher(QCoreApplication::instance());
	if(K_ZaehlerFuerKlasseninstanzen==0)
	{
		//OpenSSL initialisieren, hier lassen wir erst mal alle SSL Versionen zu
		SSL_load_error_strings();
		SSL_library_init();
		K_OpenSSLStruktur=SSL_CTX_new(SSLv23_client_method());
	}
	K_ZaehlerFuerKlasseninstanzen++;
	K_KeineOpenSSLStrukturText=trUtf8("OpenSSL Struktur nicht verfügbar\r\n");
	K_KeineSSLStrukturText=trUtf8("SSL Struktur nicht verfügbar\r\n");
	K_SSLServerNichtGefundenText=tr("Der SSL Server wurde nicht gefunden.");
	K_SSLServerVerbindungAbgelehntText=tr("Der SSL Server hat die Verbindung abgelehnt");
	K_SSLServerVerbindungVomServerGetrenntText=tr("Der SSL Server hat die Verbindung getrennt.");
	K_SSLStrukturKonnteNichtErzeugtWerdenText=tr("Die SSL Struktur konnte nicht erzeugt weden.\r\n");
	K_Verbindungsstatus=QFrankSSL::GETRENNT;
	K_SSLStruktur=NULL;
	K_ZuBenutzendeSSLVersionen=QFrankSSL::SSLv2|QFrankSSL::SSLv3|QFrankSSL::TLSv1;
	if(K_OpenSSLStruktur==NULL)
	{
#ifndef QT_NO_DEBUG
		qCritical("QFrankSSL OpenSSL Struktur konnte nicht erstellt werden.");
		qCritical(K_SSLFehlertext().toAscii().constData());
#endif
		QTimer::singleShot(0,this,SLOT(K_FehlertextSenden()));
		return;
	}
	connect(this,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(K_SocketfehlerAufgetreten(const QAbstractSocket::SocketError)));
	connect(this,SIGNAL(readyRead()),this,SLOT(K_DatenKoennenGelesenWerden()));
	connect(this,SIGNAL(connected()),this,SLOT(K_MitServerVerbunden()));
	connect(this,SIGNAL(disconnected()),this,SLOT(K_VerbindungZumServerGetrennt()));	
}

QFrankSSL::~QFrankSSL()
{
	//Wenn verbunden dann trennen
	if(state()==QAbstractSocket::ConnectedState)
		//Erst SSL Verbindung trennen wenn vorhanden, und dann vom Server selbst trennen
		VerbindungTrennen();
	//OpenSSL aufräumen
	if(K_SSLStruktur!=NULL)
		SSL_free(K_SSLStruktur);
	K_ZaehlerFuerKlasseninstanzen--;
	//Der Letzte macht das Licht aus
	if(K_ZaehlerFuerKlasseninstanzen==0)
	{
		if(K_OpenSSLStruktur!=NULL)
			SSL_CTX_free(K_OpenSSLStruktur);
		ERR_free_strings();
		ERR_remove_state(0);
		QCoreApplication::instance()->removeTranslator(K_Uebersetzung);
	}	
}

const bool QFrankSSL::K_SSLStrukturAufbauen()
{
	if(K_OpenSSLStruktur==NULL)
	{
		K_OpenSSLFehlerText=K_KeineOpenSSLStrukturText+K_SSLFehlertext();
#ifndef QT_NO_DEBUG
		qCritical("QFrankSSL SSL Struktur aufbauen: OpenSSL Struktur fehlt");
		qCritical(qPrintable(K_SSLFehlertext()));
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
	//Damit die Callbackfunktion denn Zugriff auf die Klasse hat. 
	K_ListeDerSSLVerbindungen.insert(K_SSLStruktur,this);
	SSL_set_info_callback(K_SSLStruktur,K_SSL_Info_Callback);
	
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
	qDebug(SSLOptionen.toLatin1().constData());
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
		qWarning(qPrintable(trUtf8("QFrankSSL verfügbare Algorithmen: keine gültige SSL Struktur","debug")));
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
	qDebug(qPrintable(trUtf8("QFrankSSL verfügbare Algorithmen:\r\n%1","debug").arg(K_VerfuegbareAlgorithmen.join(":"))));
#endif	
}


void QFrankSSL::K_DatenKoennenGelesenWerden()
{
	int BytesDa=bytesAvailable();
#ifndef QT_NO_DEBUG
	qDebug(qPrintable(trUtf8("QFrankSSL: Es können %1 Bytes gelesen werden.","debug").arg(BytesDa)));
#endif
	if(K_Verbindungsstatus==QFrankSSL::GETRENNT)
	{
#ifndef QT_NO_DEBUG
		qDebug(qPrintable(QString("\tAber es besteht keine Verbindung zum SSL Server:(")));
#endif
		return;
	}
	if(BytesDa==0)
	{
#ifndef QT_NO_DEBUG
		qDebug("\tEs sollen 0 Byte gelesen werde. Sinnlos.");
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
										qDebug(qPrintable(QString("QFrankSSL: Daten:\r\n%1").arg(K_FeldNachHex(K_EmpfangenenDaten))));
#endif
									}
									if(K_SSL_Fehlercode<0)
									{
										K_SSL_Fehlercode=SSL_get_error(K_SSLStruktur,K_SSL_Fehlercode);
#ifndef QT_NO_DEBUG
										qDebug(qPrintable(QString("\tSSL_Error ergab:%1").arg(K_SSL_Fehlercode)));
#endif									
									}
									
									//ServerAntwort=SSL_state_string_long(K_SSLStruktur);
									//qDebug(ServerAntwort.toAscii().constData());

									break;
		case QFrankSSL::VERBINDEN:
									/*	Wenn die Verbindung mit dem SSL Server steht, Handschlag durchführen
										Es sollte das Wort server hello drin vorkommen.

									*/
									ServerAntwort=SSL_state_string_long(K_SSLStruktur);
									if(ServerAntwort.contains("server hello"))
									{
#ifndef QT_NO_DEBUG
										qDebug(qPrintable(trUtf8("QFrankSSL Daten empfangen: Bereit für den Handschake.\r\nAntwort vom Server: %1","debug")
																.arg(ServerAntwort)));
#endif
										K_SSL_Handshake();
									}
									else
									{
#ifndef QT_NO_DEBUG
										qDebug(qPrintable(trUtf8("QFrankSSL Daten empfangen: Nicht bereit für den Handschake.\r\nAntwort vom Server: %1","debug")
																.arg(ServerAntwort)));
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
										qDebug(qPrintable(QString("QFrankSSL Daten empfangen: Handshake ok.\r\nAntwort vom Server: %1").arg(ServerAntwort)));
										K_ServerZertifikatUntersuchen();
#endif
										K_Verbindungsstatus=QFrankSSL::VERBUNDEN;
										emit TunnelBereit();
									}
									else
									{
#ifndef QT_NO_DEBUG
										qDebug(qPrintable(QString("QFrankSSL Daten empfangen: Handshake gescheitert.\r\nAntwort vom Server: %1")
																  .arg(ServerAntwort)));
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
		qCritical(qPrintable(trUtf8("QFrankSSL kein gültiger Verschlüsselungsalgorithmus angegeben","debug")));
#endif
		K_AllesZuruecksetzen();
		//Liste löschen, da eh ungültig
		K_VerfuegbareAlgorithmen.clear();
		emit SSLFehler(trUtf8("Gewünschter Verschlüsselungsalgorithmus wird von der aktuellen OpenSSL Bibliothek nicht unerstützt!"));
		return;
	}
	
	connectToHost(rechnername,port,betriebsart);
}

void QFrankSSL::K_VerbindungZumServerGetrennt()
{
#ifndef QT_NO_DEBUG
	qDebug("QFrankSSL Verbindung zum Server verlohren.");
#endif
	K_AllesZuruecksetzen();
	emit VerbindungGetrennt(false);
}

const bool QFrankSSL::K_MussWasGesendetWerden()
{
#ifndef QT_NO_DEBUG
	qDebug(qPrintable(trUtf8("QFrankSSL müssen wir Daten senden?","debug")));
#endif
	//Wieviel daten warten auf  Bearbeitung?? >0 sind welche da
	int warteneDaten=BIO_ctrl(K_Sendepuffer,BIO_CTRL_PENDING,0,NULL);
	if (warteneDaten>0)
	{
#ifndef QT_NO_DEBUG
		qDebug(qPrintable(QString("\tja %1 Bytes").arg(warteneDaten)));
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
	qDebug(qPrintable(QString("QFrankSSL Daten Senden Daten: \r\n%1").arg(K_FeldNachHex(daten))));
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
	K_Empfangspuffer=BIO_new(BIO_s_mem());
	K_Sendepuffer=BIO_new(BIO_s_mem());
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
#ifndef QT_NO_DEBUG
	qDebug("QFrankSSL Verbindung trennen");
#endif
	if(K_SSLStruktur==NULL)
		return;
	K_Verbindungsstatus=QFrankSSL::TRENNEN;
	if(SSL_shutdown(K_SSLStruktur)==0)
		SSL_shutdown(K_SSLStruktur);
	if(K_MussWasGesendetWerden())
			K_DatenSenden();
	else
		disconnectFromHost();	
}

void QFrankSSL::K_AllesZuruecksetzen()
{	
#ifndef QT_NO_DEBUG
	qDebug(qPrintable(trUtf8("QFrankSSL alles zurücksetzen","debug")));
#endif
	if(state()==QAbstractSocket::ConnectedState)
		disconnectFromHost();
	//SSL Struktur löschen
	if(K_SSLStruktur!=NULL)
	{
		if(K_ListeDerSSLVerbindungen.contains(K_SSLStruktur))
			K_ListeDerSSLVerbindungen.remove(K_SSLStruktur);
		SSL_free(K_SSLStruktur);
		
	}
	K_SSLStruktur=NULL;
	K_Verbindungsstatus=QFrankSSL::GETRENNT;
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
		qCritical(qPrintable(trUtf8("QFrankSSL Bugprüfung: 0.9.7c gefunden!!!","debug")));
#endif
		emit SSLFehler(trUtf8("Die installierte OpenSSL Version: %1 enthält Bugs.\r\nWeitere Hinweise entnehmen Sie bitte der Datei Hinweise.txt")
								.arg(SSLeay_version(SSLEAY_VERSION)));
		return true;
	}
	return false;
}

void QFrankSSL::K_SocketfehlerAufgetreten(const QAbstractSocket::SocketError &fehler)
{
	switch(fehler)
	{
		case QAbstractSocket::HostNotFoundError:
#ifndef QT_NO_DEBUG
													qWarning("QFrank SSL Verbindung: SSL Server nicht gefunden");
#endif
													K_AllesZuruecksetzen();
													emit SSLFehler(K_SSLServerNichtGefundenText);
													break;
		case QAbstractSocket::ConnectionRefusedError:
#ifndef QT_NO_DEBUG
													qWarning("QFrank SSL Verbindung: SSL Server Verbindung abgelehnt");
#endif
													K_AllesZuruecksetzen();
													emit SSLFehler(K_SSLServerVerbindungAbgelehntText);
													break;
		case QAbstractSocket::RemoteHostClosedError:
#ifndef QT_NO_DEBUG
													qDebug("QFrank SSL Verbindung: SSL Server Verbindung getrennt");
#endif
													/*if(K_Verbindungsstatus==QFrankSSL::VERBUNDEN)
														emit VerbindungGetrennt(false);
													else
														emit SSLFehler(K_SSLServerVerbindungVomServerGetrenntText);
													break;
		default:
													qFatal("QFrank SSL Verbindung: Zustand nicht bearbeitet Code: %i",fehler);
													break;*/	
	}
	//K_AllesZuruecksetzen();
}

void QFrankSSL::K_SSL_Info_Callback(const SSL *ssl,int wo,int rueckgabe)
{
	/*	Haben wir das SSL Objekt in unserer Hashtabelle??
		Wenn nicht ganz großes Problem
	*/
	//int Wo=wo&~SSL_ST_MASK;
	if(!K_ListeDerSSLVerbindungen.contains(ssl))
		qFatal("QFrankSSL SSL_Info_Callback: Das SSL Objekt befindet sich nicht in der Tabelle!!");
	qDebug("wo=0x%X",wo);
	switch(wo)
	{
		case SSL_CB_CONNECT_EXIT:
#ifndef QT_NO_DEBUG
								qDebug("QFrankSSL SSL_Info_Callback: meldet ein Fehler");
#endif
								qDebug(qPrintable(QString("%1").arg(rueckgabe)));
								break;
		case SSL_CB_READ_ALERT:
		case SSL_CB_WRITE_ALERT:
									QString Alarmtype=SSL_alert_type_string_long(rueckgabe);
									QString Alarmtext=SSL_alert_desc_string_long(rueckgabe);
#ifndef QT_NO_DEBUG
									qCritical("QFrankSSL SSL_Info_Callback: meldet einen Alarm");
									qCritical("Art des Alams: %s\r\nBeschreibung: %s",Alarmtype.toAscii().constData(),Alarmtext.toAscii().constData());
#endif
									break;
	}

}

QFrankSSLZertifikatspeicher* QFrankSSL::K_Zertifikatspeicher=0;
uint QFrankSSL::K_ZaehlerFuerKlasseninstanzen=0;
SSL_CTX* QFrankSSL::K_OpenSSLStruktur=NULL;
QHash<const SSL*,QFrankSSL*> QFrankSSL::K_ListeDerSSLVerbindungen;
QTranslator* QFrankSSL::K_Uebersetzung=0;

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

void QFrankSSL::K_ServerZertifikatUntersuchen()const
{
	X509 *Zertifikat=SSL_get_peer_certificate(K_SSLStruktur);
	if(Zertifikat==NULL)
		qDebug("QFrankSSL Server Zertifikat untersuchen: Der Server zeigt kein Zertifikat vor.");
	else
	{
		BIO *Puffer = BIO_new(BIO_s_mem());
		if(X509_print(Puffer,Zertifikat)==1)
		{
			QByteArray Serverzert;
			int Groesse=BIO_ctrl(Puffer,BIO_CTRL_PENDING,0,NULL);
			Serverzert.resize(Groesse);
			BIO_read(Puffer,Serverzert.data(),Groesse);
			BIO_free(Puffer);
			qDebug(qPrintable(QString("QFrankSSL Server Zertifikat untersuchen: Das Serverzertifikat:\r\n%1").arg(QString(Serverzert))));
		}
		else
		{
			qDebug("QFrankSSL Server Zertifikat untersuchen: Zertifikat konnte nicht gelesen werden.");
		}
	}
}
#endif
