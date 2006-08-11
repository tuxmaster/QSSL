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
	qWarning("WARNUNG Debugversion wird benutzt.\r\nEs könnten sicherheitsrelevante Daten ausgegeben werden!!!!!");
#endif
	K_SSL_Betriebsbereit=false;
	//OpenSSL initialisieren
	SSL_library_init();
	SSL_load_error_strings();
	K_OpenSSLVerbindung=0;
	//OpenSSL Verbindung aufbauen, hier lassen wir erst mal alle SSL Versionen zu.
	K_OpenSSLVerbindung=SSL_CTX_new(SSLv23_client_method());
	if(K_OpenSSLVerbindung==NULL)
	{
#ifndef QT_NO_DEBUG
		qDebug("QFrankSSL OpenSSL Struktur konnte nicht erstellt werden.");
		qDebug()<<K_SSLFehlertext();
#endif
		QTimer::singleShot(0,this,SLOT(K_FehlertextSenden()));
		return;
	}
	K_SSLStruktur=SSL_new(K_OpenSSLVerbindung);
	if(K_SSLStruktur==NULL)
	{
#ifndef QT_NO_DEBUG
		qDebug("QFrankSSL SSL Struktur konnte nicht erstellt werden.");
		qDebug()<<K_SSLFehlertext();
#endif
		QTimer::singleShot(0,this,SLOT(K_FehlertextSenden()));
		return;
	}
	K_SSL_Betriebsbereit=true;
	connect(this,SIGNAL(connected()),this,SLOT(K_MitServerVerbunden()));
}

void QFrankSSL::VerbindungHerstellen(const QString &rechnername,const quint16 &port,const OpenMode &betriebsart)
{
	if(!K_SSL_Betriebsbereit)
		return;
	connectToHost(rechnername,port,betriebsart);
}

void QFrankSSL::K_MitServerVerbunden()
{
	if(!K_SSL_Betriebsbereit)
		return;
#ifndef QT_NO_DEBUG
	qDebug("QFrankSSL Verbindung mit dem Server hergestellt. Übergebe an OpenSSL");
#endif
	if(SSL_set_fd(K_SSLStruktur,socketDescriptor())!=1)
	{
		//Übergabe gescheitert.
#ifndef QT_NO_DEBUG
		qDebug("QFrankSSL Übergabe an OpenSSL gescheitert.");
		qDebug()<<K_SSLFehlertext();
#endif
		disconnectFromHost();
		emit SSLFehler(K_SSLFehlertext());
	}
	//SSL aushandeln 
	K_SSL_Fehlercode=SSL_connect(K_SSLStruktur);
	//<0 TLS/SSL handshake gescheitert, 1 alles ok
	if(K_SSL_Fehlercode==1)
	{
#ifndef QT_NO_DEBUG
		qDebug()<<"QFrankSSL SSL Aushandlung erfolgreich";
#endif		
	}
	else if(K_SSL_Fehlercode<0)
	{
#ifndef QT_NO_DEBUG
		qDebug()<<"QFrankSSL SSL Aushandlung gescheitert. Versuche Grund zu erfahren";
#endif
		K_SSL_Fehlercode=SSL_get_error(K_SSLStruktur,K_SSL_Fehlercode);
		switch(K_SSL_Fehlercode)
		{
			case SSL_ERROR_WANT_READ:
#ifndef QT_NO_DEBUG
										qDebug()<<"QFrankSSL SSL Aushandlung gescheitert Grund: Lesefehler";
#endif									break;
		}
			
	}
	else
	{
#ifndef QT_NO_DEBUG
		qDebug()<<"QFrankSSL SSL Aushandlung gescheitert Grund: nicht behebbar";
#endif
	}

	/*
	if (K_SSL_Fehlercode==0)
	{
	}
	if(K_SSL_Fehlercode!=SSL_ERROR_NONE)
	{
#ifndef QT_NO_DEBUG
		qDebug("QFrankSSL SSL Aushandlung der Parameter gescheitert.");
		qDebug()<<K_SSLFehlertext(QFrankSSL::SSL_Struktur);
#endif
		emit SSLFehler(K_SSLFehlertext(QFrankSSL::SSL_Struktur));
	}*/
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

QFrankSSL::~QFrankSSL()
{
	//OpenSSL aufräumen
	if(K_SSLStruktur!=NULL)
		SSL_free(K_SSLStruktur);
	ERR_free_strings();
	if(K_OpenSSLVerbindung!=NULL)
		SSL_CTX_free(K_OpenSSLVerbindung);
}