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

#ifndef QFRANKSSL_H
#define QFRANKSSL_H

#include <QtNetwork>

//Unter Windows  braucht man Hilfe beim Exportieren
#ifdef BIBLIOTHEK_BAUEN
	#ifdef Q_WS_WIN
		#define DLL_BAUEN
	#endif
//Open SSL Header
#include <openssl/ssl.h>
#include <openssl/err.h>
#else
	class SSL_CTX;
	class SSL;
	class BIO;
#endif

#ifdef Q_WS_WIN 
	#ifdef DLL_BAUEN
		#define DLL_EXPORT __declspec(dllexport)
	#else
			#define DLL_EXPORT __declspec(dllimport)
	#endif
#else
		#define DLL_EXPORT
#endif

class DLL_EXPORT QFrankSSL: public QTcpSocket
{
	Q_OBJECT
	public:
				QFrankSSL(QObject* eltern);
				~QFrankSSL();
				void				VerbindungHerstellen(const QString &rechnername,const quint16 &port,const OpenMode &betriebsart=QIODevice::ReadWrite);
				void				VerfuegbareAlgorithmenFestlegen(const QStringList &welche){K_VerfuegbareAlgorithmen=welche;}
				void				VerbindungTrennen();
				const QStringList&	VerfuegbareAlgorithmen()const{return K_VerfuegbareAlgorithmen;}

	public slots:
				void				DatenSenden(const QByteArray &daten);

	signals:
				void				SSLFehler(const QString &fehlertext)const;
				void				DatenBereitZumAbhohlen(const QByteArray &daten)const;
				void				TunnelBereit()const;
	
	private slots:

				void				K_SocketfehlerAufgetreten(const QAbstractSocket::SocketError &fehler);	

	private:
				enum				Fehlerquelle{SSL_Struktur=0x00,SSL_Bibliothek=0x01};
				enum				StatusDerVerbidnung{VERBINDEN=0x00,HANDSCHLAG=0x01,VERBUNDEN=0x02};
				Q_DECLARE_FLAGS(ArtDerFehlerquelle,Fehlerquelle)
				QString				K_KeineOpenSSLStrukturText;
				QString				K_KeineSSLStrukturText;
				QString				K_SSLServerNichtGefundenText;
				QString				K_SSLServerVerbindungAbgelehntText;
				QString				K_SSLServerVerbindungVomServerGetrenntText;
				QString				K_SSLStrukturKonnteNichtErzeugtWerdenText;
				QString				K_OpenSSLFehlerText;
				SSL_CTX*			K_OpenSSLStruktur;
				SSL*				K_SSLStruktur;
				BIO*				K_Empfangspuffer;
				BIO*				K_Sendepuffer;
				const QString		K_SSLFehlertext(const QFrankSSL::ArtDerFehlerquelle &fehlerquelle=QFrankSSL::SSL_Bibliothek)const;
				bool				K_SSL_Betriebsbereit;
				bool				K_SSL_VerbindungAufgebaut;
				bool				K_SSL_Handshake_durchgefuehrt;
				bool				K_TunnelBereit;
				const bool			K_MussWasGesendetWerden();
				const bool			K_SSLStrukturAufbauen();
				int					K_SSL_Fehlercode;
				void				K_DatenSenden();
				void				K_SSL_Handshake();
				void				K_VerfuegbareAlgorithmenHohlen();
				void				K_AllesZuruecksetzen();
				QByteArray			K_EmpfangenenDaten;
				QStringList			K_VerfuegbareAlgorithmen;

#ifndef QT_NO_DEBUG
			QString					K_FeldNachHex(const QByteArray &feld) const;
#endif
				
	private slots:
				void				K_FehlertextSenden();
				void				K_MitServerVerbunden();
				void				K_DatenKoennenGelesenWerden();
};
#endif
