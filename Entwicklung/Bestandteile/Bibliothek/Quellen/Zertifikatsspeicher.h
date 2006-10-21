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

#ifndef QFRANKSSLZERTIFIKATSPEICHER_H
#define QFRANKSSLZERTIFIKATSPEICHER_H

#include <QtCore>

#ifdef Q_WS_WIN 
	#include <windows.h>
	#include <Wincrypt.h>
	//Unter Windows  braucht man Hilfe beim Exportieren
	#ifdef DLL_BAUEN
		#define DLL_EXPORT __declspec(dllexport)
	#else
		#define DLL_EXPORT __declspec(dllimport)
	#endif
#else
	//Nicht Windows Systeme brauchen ja den XML Speicher;
	class QDomDocument;
	class QDomNode;
	#define DLL_EXPORT
#endif

typedef struct x509_st X509;
typedef struct X509_crl_st X509_CRL;

class DLL_EXPORT QFrankSSLZertifikatspeicher: public QObject
{
	Q_OBJECT
	public:
				QFrankSSLZertifikatspeicher(QObject* eltern);
				~QFrankSSLZertifikatspeicher();
				enum				ArtDesZertifikats{CRL=0x00,CA=0x01,Zert=0x02};
				Q_DECLARE_FLAGS(Zertifikatstype,ArtDesZertifikats)
				const QStringList	ListeAllerZertifikate(const QFrankSSLZertifikatspeicher::Zertifikatstype &type);
				enum				ArtDesSpeichers{System=0x01,Nutzer=0x02};
				Q_DECLARE_FLAGS(Speicherort,ArtDesSpeichers)
#ifndef Q_WS_WIN
				void				PasswortFuerDenSpeicher(QString &passwort);
				void				ZertifikatSpeichern(const QFrankSSLZertifikatspeicher::Speicherort &ort,
														const QFrankSSLZertifikatspeicher::Zertifikatstype &type,const QString &datei);
				void				loeschen(const QFrankSSLZertifikatspeicher::Speicherort &ort);
#endif

	public slots:				
				void				SpeicherLaden();

	signals:
				void				Fertig()const;
				void				Fehler(const QString &fehlertext)const;
				void				Warnung(const QString &warnungstext)const;
#ifndef Q_WS_WIN
				void				PasswortFuerDenSpeicherHohlen()const;
#endif

	private:
				QList<X509*>		*K_Zertifikatsliste;
				QList<X509_CRL*>	*K_Rueckrufliste;
				bool 				K_Speichergeladen;
				QString				K_SpeichertypeText;
				void				K_SpeichertypeTextSetzen(const QFrankSSLZertifikatspeicher::Speicherort &type);
				QString				K_ZertifikatNachText(const QFrankSSLZertifikatspeicher::Zertifikatstype &type,void *zertifikat);
#ifndef Q_WS_WIN
				// Unix/Linux/Mac Speicher
				enum				LadenOderSpeichern{Laden=0x01,Speichern=0x02,Nichts=0x03};
				Q_DECLARE_FLAGS(LadenSpeichern,LadenOderSpeichern)
				QFrankSSLZertifikatspeicher::LadenSpeichern		K_ZertspeicherAktion;
				QFrankSSLZertifikatspeicher::Speicherort		K_Speicherort;
				QFrankSSLZertifikatspeicher::Zertifikatstype	K_Zerttype;
				QString											K_Zertdatei;
				void				K_PasswortLoeschen();
				QString				K_DateinameBenutzer;
				QString				K_DateinameSystem;
				QString				K_Passwort;
				bool				K_PasswortGesetzt;
				bool				K_ZertifikatsdateiLaden(const QString &quellDatei,QByteArray &daten);
				bool				K_FeldNachZert(QByteArray &feld,const QFrankSSLZertifikatspeicher::Zertifikatstype &type,bool wandeln=false);
				bool				K_XMLLaden(QDomDocument *dokument,const QFrankSSLZertifikatspeicher::Speicherort &type);
				bool				K_XMLSpeichern(QDomDocument *dokument,const QFrankSSLZertifikatspeicher::Speicherort &ort);
				bool				K_XMLBearbeiten(QDomDocument *xml);
				bool				K_XMListZertspeicher(QDomDocument *xml);
				bool				K_XMLEintragLesen(const QFrankSSLZertifikatspeicher::Zertifikatstype &type,QDomNode *eintrag);
				bool				K_XMLEintragSchreiben(const QFrankSSLZertifikatspeicher::Zertifikatstype &type,const QString &quellDatei,QDomDocument *xml);
#else
				// Windows Speicher
				bool				K_SpeicherLaden(const QFrankSSLZertifikatspeicher::Speicherort &type);
				bool				K_UnterspeicherLesen(const QString &unterspeichername,const DWORD &speicherort);
				static BOOL	WINAPI 	CertEnumSystemStoreRueckruf(const void *speicherplatz,DWORD parameter,PCERT_SYSTEM_STORE_INFO speicherInfos,
																void *reserviert,void *hilfsparameter);
#endif

};
#endif
