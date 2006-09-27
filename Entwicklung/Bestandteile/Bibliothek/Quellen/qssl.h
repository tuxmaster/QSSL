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
#ifdef Q_WS_WIN 
	#ifdef DLL_BAUEN
		#define DLL_EXPORT __declspec(dllexport)
	#else
		#define DLL_EXPORT __declspec(dllimport)
	#endif
#else
		#define DLL_EXPORT
#endif

class QFrankSSLZertifikatspeicher;
typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;
typedef struct bio_st BIO;

//MMmmPP M=Major,m=minor,P=Patch
#define QFRANKSSLVERSION 0x000100;
#define QFRANKSSLVERSIONTEXT "0.1.0";

class DLL_EXPORT QFrankSSL: public QTcpSocket
{
	Q_OBJECT
	public:
				enum								ArtDerSSLVerbindung{SSLv2=0x01,SSLv3=0x02,TLSv1=0x04};
				Q_DECLARE_FLAGS(SSLVersion,ArtDerSSLVerbindung);

				QFrankSSL(QObject* eltern);
				~QFrankSSL();
				void								VerbindungHerstellen(const QString &rechnername,const quint16 &port,const OpenMode &betriebsart=QIODevice::ReadWrite);
				void								VerfuegbareAlgorithmenFestlegen(const QStringList &welche){K_VerfuegbareAlgorithmen=welche;}
				void								VerbindungTrennen();
				void								ZertifikateLaden();
				void								ZertifikatsspeicherPasswort(QString* passwort);
				void								SSLVersionenFestlegen(const QFrankSSL::SSLVersion &sslVersion){K_ZuBenutzendeSSLVersionen=sslVersion;}
				const QFrankSSL::SSLVersion&		SSLVersionen()const{return K_ZuBenutzendeSSLVersionen;}
				const QStringList&					VerfuegbareAlgorithmen()const{return K_VerfuegbareAlgorithmen;}
				static const QString				VersionText(){return QFRANKSSLVERSIONTEXT;}
				static const quint32				Version(){return QFRANKSSLVERSION}

	public slots:
				void								DatenSenden(const QByteArray &daten);

	signals:
				void								SSLFehler(const QString &fehlertext)const;
				void								DatenBereitZumAbhohlen(const QByteArray &daten)const;
				void								VerbindungGetrennt(const bool &fehler)const;
				void								TunnelBereit()const;
				void								PasswortFuerDenZertifikatsspeicher()const;

	private slots:

				void								K_SocketfehlerAufgetreten(const QAbstractSocket::SocketError &fehler);
				void								K_FehlertextSenden();
				void								K_MitServerVerbunden();
				void								K_VerbindungZumServerGetrennt();
				void								K_DatenKoennenGelesenWerden();

	private:
				enum								Fehlerquelle{SSL_Struktur=0x00,SSL_Bibliothek=0x01};
				enum								StatusDerVerbidnung{HANDSCHLAG=0x02,VERBUNDEN=0x03,VERBINDEN=0x04,GETRENNT=0x05,TRENNEN=0x06};
				Q_DECLARE_FLAGS(ArtDerFehlerquelle,Fehlerquelle)
				Q_DECLARE_FLAGS(Verbindungsstatus,StatusDerVerbidnung)
				QString								K_KeineOpenSSLStrukturText;
				QString								K_KeineSSLStrukturText;
				QString								K_SSLServerNichtGefundenText;
				QString								K_SSLServerVerbindungAbgelehntText;
				QString								K_SSLServerVerbindungVomServerGetrenntText;
				QString								K_SSLStrukturKonnteNichtErzeugtWerdenText;
				QString								K_OpenSSLFehlerText;
				static SSL_CTX*						K_OpenSSLStruktur;
				SSL*								K_SSLStruktur;
				BIO*								K_Empfangspuffer;
				BIO*								K_Sendepuffer;
				const QString						K_SSLFehlertext(const QFrankSSL::ArtDerFehlerquelle &fehlerquelle=QFrankSSL::SSL_Bibliothek)const;
				const bool							K_MussWasGesendetWerden();
				const bool							K_SSLStrukturAufbauen();
				const bool							K_OpenSSLMitBugs()const;
				int									K_SSL_Fehlercode;
				void								K_DatenSenden();
				void								K_SSL_Handshake();
				void								K_VerfuegbareAlgorithmenHohlen();
				void								K_AllesZuruecksetzen();
				static void							K_SSL_Info_Callback(const SSL *ssl,int wo,int rueckgabe);
				QByteArray							K_EmpfangenenDaten;
				QStringList							K_VerfuegbareAlgorithmen;
				static QFrankSSLZertifikatspeicher*	K_Zertifikatspeicher;
				static uint							K_ZaehlerFuerKlasseninstanzen;
				static QHash<const SSL*,QFrankSSL*>	K_ListeDerSSLVerbindungen;
				QFrankSSL::Verbindungsstatus		K_Verbindungsstatus;
				QFrankSSL::SSLVersion				K_ZuBenutzendeSSLVersionen;
				static QTranslator*					K_Uebersetzung;

#ifndef QT_NO_DEBUG
				QString								K_FeldNachHex(const QByteArray &feld) const;
				void								K_ServerZertifikatUntersuchen()const;
#endif

};
Q_DECLARE_OPERATORS_FOR_FLAGS(QFrankSSL::SSLVersion)
#endif
