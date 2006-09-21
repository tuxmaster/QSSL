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

#include "Zertifikatsspeicher.h"

//Den Filter brauchen wir nur unter nicht Windows Sytemen, da wir unter Windows den Systemeigenen nutzen.
//#ifndef Q_WS_WIN
#include "Datenstromfilter.h"
#include <QtXml>
//#endif

QFrankSSLZertifikatspeicher::QFrankSSLZertifikatspeicher(QObject* eltern):QObject(eltern)
{
	//Warnung bei Debug
#ifndef QT_NO_DEBUG
	qWarning(trUtf8("WARNUNG Debugversion wird benutzt.\r\nEs können sicherheitsrelevante Daten ausgegeben werden!!","debug").toLatin1().constData());
#endif
//#ifndef Q_WS_WIN
	QSettings EinstellungenSystem(QSettings::IniFormat,QSettings::SystemScope,"QSSL","tmp");
	QSettings EinstellungenNutzer(QSettings::IniFormat,QSettings::UserScope,"QSSL","tmp");
	K_SpeicherortSystemweit=EinstellungenSystem.fileName().left(EinstellungenSystem.fileName().lastIndexOf("/"))+"/Zertifikate.db";
	K_SpeicherortBenutzer=EinstellungenNutzer.fileName().left(EinstellungenNutzer.fileName().lastIndexOf("/"))+"/Zertifikate.db";
#ifndef QT_NO_DEBUG
	qDebug("Ablageort des Zertifikatsspeichers:\r\n\tSystemweit:%s\r\n\tBenutzer:%s",K_SpeicherortSystemweit.toAscii().constData(),
																					K_SpeicherortBenutzer.toAscii().constData());
#endif
//#endif
	K_Speichergeladen=false;
}

void QFrankSSLZertifikatspeicher::SpeicherLaden(bool passwort)
{
	/*
		Strucktur des Systemspeichers:
		<Zertifikatsspeicher>
			<CRL>
					<Daten>Base64 Daten der 1. CRL</Daten>
					.
					.
					<Daten>Base64 Daten der n. CRL</Daten>
			</CRL>
			<CA>
					<Daten>Base64 Daten der 1. CA</Daten>
					<Daten>Babe64 Daten der n. CA</Daten>
			<CA>
		</Zertifikatsspeicher>
	*/
	if(K_Speichergeladen)
	{
#ifndef QT_NO_DEBUG
		qWarning("QFrankSSLZertifikatspeicher Laden: speicher ist schon geladen");
#endif
		emit Fehler(tr("Der Zertifikatsspeicher wurde bereits geladen"));
		return;
	}
//#ifndef Q_WS_WIN
	if(!passwort)
	{
		//Passwort für den Nutzerspeicher abfragen
		emit PasswortFuerDenSpeicherHohlen();
		return;
	}
	QFile DateiBenutzer(K_SpeicherortBenutzer);
	QFile DateiSystem(K_SpeicherortSystemweit);
	QDomDocument* Speicher=new QDomDocument();
	if(DateiSystem.exists())
	{
		if(!DateiSystem.open(QIODevice::ReadOnly))
		{
#ifndef QT_NO_DEBUG
			qWarning(qPrintable(trUtf8("QFrankSSLZertifikatspeicher Laden: Systemspeicher konnte nicht geöffnet werden.","debug")));
#endif
			emit Warnung(tr("Der Zertifikatsspeicher des Systems konnte nicht gelesen werden."));
		}
		else
		{
			if(!Speicher->setContent(&DateiSystem))
			{
#ifndef QT_NO_DEBUG
				qCritical(qPrintable(trUtf8("QFrankSSLZertifikatspeicher Laden: Speicher des Systems beschädigt","debug")));
#endif
				emit Fehler(trUtf8("Der Zertifikatspeicher des Systems ist beschädigt."));
				delete Speicher;
				return;
			}
			else
			{
#ifndef QT_NO_DEBUG
				qDebug("QFrankSSLZertifikatspeicher Laden: Systemspeicher geladen.");
				qDebug("Inhalt: %s",qPrintable(Speicher->toString()));
#endif
			}
		}
	}
	else
	{
#ifndef QT_NO_DEBUG
		qDebug("QFrankSSLZertifikatspeicher Laden: kein Systemspeicher vorhanden.");
#endif
	}
	if(DateiBenutzer.exists())
	{
		QFrankDatenstromfilter Entschluesselung(&DateiBenutzer,K_Passwort);
		if(!Entschluesselung.open(QIODevice::ReadOnly))
		{
#ifndef QT_NO_DEBUG
			qCritical(qPrintable(trUtf8("QFrankSSLZertifikatspeicher Laden: konnte den Speicher des Nutzers nicht öffnen\r\nUrsache:%1","debug").arg(Entschluesselung.
																																					errorString())));
#endif
			emit Fehler(tr("Der Zertifikatspeicher des Benutzers konnte nicht geladen werden."));
			delete Speicher;
			return;
		}
		if(!Speicher->setContent(&Entschluesselung))
		{
#ifndef QT_NO_DEBUG
			qCritical(qPrintable(trUtf8("QFrankSSLZertifikatspeicher Laden: Speicher des Nutzers beschädigt","debug")));
#endif
			emit Fehler(trUtf8("Der Zertifikatspeicher des Benutzers ist beschädigt."));
			delete Speicher;
			return;
		}

#ifndef QT_NO_DEBUG
		qDebug("QFrankSSLZertifikatspeicher Laden: Nutzerspeicher geladen.");
		qDebug("Inhalt: %s",qPrintable(Speicher->toString()));
#endif
	}
	delete Speicher;
//#endif
	K_Speichergeladen=true;
}

void QFrankSSLZertifikatspeicher::PasswortFuerDenSpeicher(QString* passwort)
{
	K_Passwort=passwort;
	SpeicherLaden(true);
}
