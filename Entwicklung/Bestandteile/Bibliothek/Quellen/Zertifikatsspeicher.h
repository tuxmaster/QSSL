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

class QDomDocument;
class QDomNode;

class QFrankSSLZertifikatspeicher: public QObject
{
	Q_OBJECT
	public:
				QFrankSSLZertifikatspeicher(QObject* eltern);
				void		SpeicherLaden(bool passwort=false);

	public slots:
				void		PasswortFuerDenSpeicher(QString* passwort);				

	signals:
				void		Fehler(const QString &fehlertext)const;
				void		Warnung(const QString &warnungstext)const;
				void		PasswortFuerDenSpeicherHohlen()const;

	private:
				enum		Eintraege{CRL=0x00,CA=0x01,Zert=0x02};
				Q_DECLARE_FLAGS(ArtDesEintrags,Eintraege)
				QString		K_SpeicherortSystemweit;
				QString		K_SpeicherortBenutzer;
				bool		K_Speichergeladen;
				bool		K_XMLBearbeiten(QDomDocument *xml);
				bool		K_EintragBearbeiten(const QFrankSSLZertifikatspeicher::ArtDesEintrags &type,QDomNode *eintrag);
				QString*	K_Passwort;
};
#endif
