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

class QDomDocument;
class QDomNode;

class DLL_EXPORT QFrankSSLZertifikatspeicher: public QObject
{
	Q_OBJECT
	public:
				QFrankSSLZertifikatspeicher(QObject* eltern);
				enum				ArtDesZertifikats{CRL=0x00,CA=0x01,Zert=0x02};
				Q_DECLARE_FLAGS(Zertifikatstype,ArtDesZertifikats)
				void				PasswortFuerDenSpeicher(QString* passwort);
				const QStringList	ListeAllerZertifikate(const QFrankSSLZertifikatspeicher::Zertifikatstype &type)const;
				enum				ArtDesSpeichers{System=0x01,Nutzer=0x02};
				Q_DECLARE_FLAGS(Speicherort,ArtDesSpeichers)
//#ifndef Q_WS_WIN
				bool				ZertifikatSpeichern(const QFrankSSLZertifikatspeicher::Speicherort &ort,
														const QFrankSSLZertifikatspeicher::Zertifikatstype &type,const QString &datei);
				void				loeschen(const QFrankSSLZertifikatspeicher::Speicherort &ort);
//#endif

	public slots:				
				void				SpeicherLaden(bool passwort=false);

	signals:
				void				Fehler(const QString &fehlertext)const;
				void				Warnung(const QString &warnungstext)const;
				void				PasswortFuerDenSpeicherHohlen()const;

	private:
				QString				K_SpeicherortSystemweit;
				QString				K_SpeicherortBenutzer;
				bool				K_Speichergeladen;
				bool				K_XMLBearbeiten(QDomDocument *xml);
				bool				K_EintragBearbeiten(const QFrankSSLZertifikatspeicher::Zertifikatstype &type,QDomNode *eintrag);
				QString*			K_Passwort;
};
#endif
