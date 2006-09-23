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
#include <openssl/x509.h>
#include <openssl/pem.h>

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
		<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
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
				if(!K_XMLBearbeiten(Speicher))
				{
#ifndef QT_NO_DEBUG
					qCritical(qPrintable(trUtf8("QFrankSSLZertifikatspeicher Laden: Speicher des Systems beschädigt","debug")));
#endif
					emit Fehler(trUtf8("Der Zertifikatspeicher des Systems ist beschädigt."));
					delete Speicher;
					return;
				}
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

bool QFrankSSLZertifikatspeicher::K_XMLBearbeiten(QDomDocument *xml)
{
	QDomElement Root=xml->documentElement();
	if(Root.tagName()!="Zertifikatsspeicher")
	{
#ifndef QT_NO_DEBUG
		qCritical("QFrankSSLZertifikatspeicher XML bearbeiten: XML Objekt ist kein Zertifikatsspeicher");
		qDebug("\t%s",qPrintable(Root.tagName()));
#endif
		return false;
	}
	//alle Einträge durchsuchen
	QDomNode Eintrag=xml->namedItem("Zertifikatsspeicher").firstChild();
	while (!Eintrag.isNull())
	{
		QDomElement Element=Eintrag.toElement();
		//Was für ein Eintrag haben wir denn??
		if(Element.tagName()=="CRL")
		{
#ifndef QT_NO_DEBUG
			qDebug("Element CRL gefunden");
#endif
			//durchsuchen aller CRL's
			if(!K_EintragBearbeiten(QFrankSSLZertifikatspeicher::CRL,&Eintrag.firstChild()))
				return false;			
		}
		else if(Element.tagName()=="CA")
		{
#ifndef QT_NO_DEBUG
			qDebug("Element CA gefunden");
#endif
			//durchsuchen aller CA's
			if(!K_EintragBearbeiten(QFrankSSLZertifikatspeicher::CA,&Eintrag.firstChild()))
				return false;		
		}
		Eintrag=Eintrag.nextSibling();
	}
	return true;
}

bool QFrankSSLZertifikatspeicher::K_EintragBearbeiten(const QFrankSSLZertifikatspeicher::ArtDesEintrags &type,QDomNode *eintrag)
{
	//Alle Einträge bearbeiten
	BIO *Puffer = BIO_new(BIO_s_mem());
	QByteArray DatenDesEintrags;
	QDomElement Element;
	X509 *Zertifikat;
	while(!eintrag->isNull())
	{
		Element=eintrag->toElement();
#ifndef QT_NO_DEBUG
		qDebug(qPrintable(QString("Inhalt des Elements: %1").arg(Element.text())));
#endif
		DatenDesEintrags=QByteArray::fromBase64(Element.text().toAscii());
		BIO_reset(Puffer);
		Zertifikat=NULL;
		if(BIO_write(Puffer,DatenDesEintrags.data(),DatenDesEintrags.size())!=DatenDesEintrags.size())
			qFatal("K_EintragBearbeiten es wurde nicht alle Bytes in den OpenSSL Puffer geschrieben");
		switch(type)
		{
			case QFrankSSLZertifikatspeicher::CRL:			
													break;
			case QFrankSSLZertifikatspeicher::CA:
													if(!PEM_read_bio_X509(Puffer,&Zertifikat,0,NULL))
													{
#ifndef QT_NO_DEBUG
														qDebug("CA konnte nicht gelesen werden");
#endif													
														break;
													}
#ifndef QT_NO_DEBUG
													BIO_reset(Puffer);
													if(X509_print(Puffer,Zertifikat)==1)
													{
														QByteArray Zerttext;
														int Groesse=BIO_ctrl(Puffer,BIO_CTRL_PENDING,0,NULL);
														Zerttext.resize(Groesse);
														BIO_read(Puffer,Zerttext.data(),Groesse);
														qDebug(qPrintable(QString("CA Element dekodiert: %1").arg(QString(Zerttext))));
													}													
#endif
													break;
			case QFrankSSLZertifikatspeicher::Zert:
													break;
			default:
													qFatal(qPrintable(trUtf8("QFrankSSLZertifikatspeicher Eintrag bearbeiten: unzulässiger Eintragstype","debug")));
													break;
		};
		eintrag=&eintrag->nextSibling();
		if(Zertifikat!=NULL)
			X509_free(Zertifikat);
	}
	BIO_free(Puffer);
	return true;
}

void QFrankSSLZertifikatspeicher::PasswortFuerDenSpeicher(QString* passwort)
{
	K_Passwort=passwort;
	SpeicherLaden(true);
}
