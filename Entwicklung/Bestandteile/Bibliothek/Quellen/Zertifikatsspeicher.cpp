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
#ifndef Q_WS_WIN
#include "Datenstromfilter.h"
#include <QtXml>
#else
#include <windows.h>
#include <Wincrypt.h>
#endif

QFrankSSLZertifikatspeicher::QFrankSSLZertifikatspeicher(QObject* eltern):QObject(eltern)
{
	//Warnung bei Debug
#ifndef QT_NO_DEBUG
	qWarning(qPrintable(trUtf8("WARNUNG Debugversion wird benutzt.\r\nEs können sicherheitsrelevante Daten ausgegeben werden!!","debug")));
#endif
#ifndef Q_WS_WIN
	QSettings EinstellungenSystem(QSettings::IniFormat,QSettings::SystemScope,"QSSL","tmp");
	QSettings EinstellungenNutzer(QSettings::IniFormat,QSettings::UserScope,"QSSL","tmp");
	K_DateinameSystem=EinstellungenSystem.fileName().left(EinstellungenSystem.fileName().lastIndexOf("/"))+"/Zertifikate.db";
	K_DateinameBenutzer=EinstellungenNutzer.fileName().left(EinstellungenNutzer.fileName().lastIndexOf("/"))+"/Zertifikate.db";
	K_PasswortGesetzt=false;
	K_ZertspeicherAktion=QFrankSSLZertifikatspeicher::Nichts;
#ifndef QT_NO_DEBUG
	qDebug(qPrintable(QString("Ablageort des Zertifikatsspeichers:\r\n\tSystemweit:%1\r\n\tBenutzer:%2").arg(K_DateinameSystem).arg(K_DateinameBenutzer)));
#endif
#endif
	K_Speichergeladen=false;	
}

void QFrankSSLZertifikatspeicher::SpeicherLaden()
{
	/*
		Strucktur des Speichers(wenn nicht der OS eigene genutzt wird):
		<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
		<Zertifikatsspeicher>
			<CRL>
					<Daten>Base64 Daten der 1. CRL</Daten>
					<Daten>Base64 Daten der n. CRL</Daten>
			</CRL>
			<CA>
					<Daten>Base64 Daten der 1. CA</Daten>
					<Daten>Babe64 Daten der n. CA</Daten>
			<CA>
		</Zertifikatsspeicher>
		Die Einträge CRL und CA können beliebig oft auftreten
	*/
	if(K_Speichergeladen)
	{
#ifndef QT_NO_DEBUG
		qWarning("QFrankSSLZertifikatspeicher Laden: speicher ist schon geladen");
#endif
		emit Fehler(tr("Der Zertifikatsspeicher wurde bereits geladen"));
		return;
	}
	// Unix/Linux/Mac Speicher
#ifndef Q_WS_WIN
	if(!K_PasswortGesetzt)
	{
		//Passwort für den Nutzerspeicher abfragen
		K_ZertspeicherAktion=QFrankSSLZertifikatspeicher::Laden;
		emit PasswortFuerDenSpeicherHohlen();
		return;
	}		
	QDomDocument* Speicher=new QDomDocument();
	//System
	if(K_XMLLaden(Speicher,QFrankSSLZertifikatspeicher::System))
	{
		//Bearbeiten
		if(!K_XMLBearbeiten(Speicher))
		{
#ifndef QT_NO_DEBUG
			qCritical(qPrintable(trUtf8("QFrankSSLZertifikatspeicher Laden: Speicher des Systems beschädigt","debug")));
#endif
			emit Fehler(trUtf8("Der Zertifikatspeicher des Systems ist beschädigt."));
			K_PasswortLoeschen();
			delete Speicher;
			return;
		}
#ifndef QT_NO_DEBUG
				qDebug("QFrankSSLZertifikatspeicher Laden: Systemspeicher geladen.");
				//qDebug("Inhalt: %s",qPrintable(Speicher->toString()));
#endif
			
		K_Speichergeladen=true;
	}
	else
	{
		K_PasswortLoeschen();
		delete Speicher;
		return;
	}
	//Nutzer
	if(K_XMLLaden(Speicher,QFrankSSLZertifikatspeicher::Nutzer))
	{
		//Bearbeiten
		if(!K_XMLBearbeiten(Speicher))
		{
#ifndef QT_NO_DEBUG
			qCritical(qPrintable(trUtf8("QFrankSSLZertifikatspeicher Laden: Speicher des Nutzers beschädigt","debug")));
#endif
			emit Fehler(trUtf8("Der Zertifikatspeicher des Nutzers ist beschädigt."));
			delete Speicher;
			K_PasswortLoeschen();
			return;
		}
#ifndef QT_NO_DEBUG
				qDebug("QFrankSSLZertifikatspeicher Laden: Nutzerpeicher geladen.");
				//qDebug("Inhalt: %s",qPrintable(Speicher->toString()));
#endif
		K_Speichergeladen=true;
	}
	else
	{
		K_Speichergeladen=false;
		delete Speicher;
		K_PasswortLoeschen();
		return;
	}
	K_PasswortLoeschen();
#else
	//Windows Speicher
	if(!K_SpeicherLaden(QFrankSSLZertifikatspeicher::System))
	{
#ifndef QT_NO_DEBUG
			qCritical(qPrintable(trUtf8("QFrankSSLZertifikatspeicher Laden: laden des Systemspeichers gescheitert","debug")));
#endif
			emit Fehler(trUtf8("Der Zertifikatspeicher des Systems konnte nicht geladen werden."));
	}
	else
	{
#ifndef QT_NO_DEBUG
		qDebug("QFrankSSLZertifikatspeicher Laden: Nutzerpeicher geladen.");
#endif
		if(!K_SpeicherLaden(QFrankSSLZertifikatspeicher::Nutzer))
		{
#ifndef QT_NO_DEBUG
			qCritical(qPrintable(trUtf8("QFrankSSLZertifikatspeicher Laden: laden des Nutzerpeichers gescheitert","debug")));
#endif
			emit Fehler(tr("Der Zertifikatspeicher des Nutzers konnte nicht geladen werden."));
		}
#ifndef QT_NO_DEBUG
		else
			qDebug("QFrankSSLZertifikatspeicher Laden: Systemspeicher geladen.");
#endif
	}
#endif
}

const QStringList QFrankSSLZertifikatspeicher::ListeAllerZertifikate(const QFrankSSLZertifikatspeicher::Zertifikatstype &type)const
{
	return QStringList()<<"Zert1"<<"Zert2";
}

void QFrankSSLZertifikatspeicher::K_SpeichertypeTextSetzen(const QFrankSSLZertifikatspeicher::Speicherort &type)
{
	K_SpeichertypeText=tr("Nutzer");
	if(type==QFrankSSLZertifikatspeicher::System)
		K_SpeichertypeText=tr("System");
}

#ifndef Q_WS_WIN

bool QFrankSSLZertifikatspeicher::K_XMLLaden(QDomDocument *dokument,const QFrankSSLZertifikatspeicher::Speicherort &type)
{
	K_SpeichertypeTextSetzen(type);
	//existiert die Datei??
	QFile Datei(K_DateinameBenutzer);
	if(type==QFrankSSLZertifikatspeicher::System)
		Datei.setFileName(K_DateinameSystem);
	if(Datei.exists())
	{
		//Öffnen der Datei
		if(!Datei.open(QIODevice::ReadOnly))
		{
			//ging nicht also Fehler ausgeben
#ifndef QT_NO_DEBUG
			qWarning(qPrintable(trUtf8("QFrankSSLZertifikatspeicher XMLLaden: %1speicher konnte nicht geöffnet werden.","debug").arg(K_SpeichertypeText)));
#endif
			emit Fehler(tr("Der Zertifikatsspeicher des %1s konnte nicht gelesen werden.").arg(K_SpeichertypeText));
			return false;
		}
		else
		{
			//Inhalt setzen entweder direkt oder vorher entschlüsselm
			if(type==QFrankSSLZertifikatspeicher::Nutzer)
			{
				QFrankDatenstromfilter Entschluesselung(&Datei,&K_Passwort);
				if(!dokument->setContent(&Entschluesselung))
				{
					//Entschlüsseln gescheitert.
#ifndef QT_NO_DEBUG
					qWarning(qPrintable(trUtf8("QFrankSSLZertifikatspeicher XMLLaden: Speicher des Nutzers beschädigt","debug")));
#endif
					Entschluesselung.close();	
					emit Fehler(trUtf8("Der Zertifikatspeicher des Benutzers ist beschädigt, oder das Passwort war falsch."));
					return false;
				}
				Entschluesselung.close();	
			}
			else
			{
				if(!dokument->setContent(&Datei))
				{
#ifndef QT_NO_DEBUG
					qWarning(qPrintable(trUtf8("QFrankSSLZertifikatspeicher XMLLaden: Speicher des Systems beschädigt","debug")));
#endif
					Datei.close();
					emit Fehler(trUtf8("Der Zertifikatspeicher des %1 ist beschädigt.").arg(K_SpeichertypeText));
					return false;
				}
				Datei.close();
			}
			//Ist die XML Datei ein Zertspeicher??
			if(!K_XMListZertspeicher(dokument))
			{
				emit Fehler(trUtf8("Der Zertifikatspeicher des %1 ist beschädigt.").arg(K_SpeichertypeText));
				return false;
			}
		}
	}
	else
	{
		//Speicher existiert nicht also warnen und ein leeres Modell zurückgeben.
#ifndef QT_NO_DEBUG
		qWarning(qPrintable(tr("QFrankSSLZertifikatspeicher XMLLaden: Der %1speicher existiert nicht.","debug").arg(K_SpeichertypeText)));
#endif		
		QDomProcessingInstruction Anweisung=dokument->createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"");
		dokument->appendChild(Anweisung);
		QDomElement Wurzel = dokument->createElement("Zertifikatsspeicher");
		dokument->appendChild(Wurzel);
		emit Warnung(tr("Der Zertifikatsspeicher des %1s existiert nicht.").arg(K_SpeichertypeText));
	}
	return true;
}

bool QFrankSSLZertifikatspeicher::K_XMLSpeichern(QDomDocument *dokument, const QFrankSSLZertifikatspeicher::Speicherort &ort)
{
	K_SpeichertypeTextSetzen(ort);
	QFile Datei(K_DateinameSystem);
	if(ort==QFrankSSLZertifikatspeicher::Nutzer)
		Datei.setFileName(K_DateinameBenutzer);
	//Datei zum Schreiben öffnen
	if(!Datei.open(QIODevice::Truncate|QIODevice::WriteOnly))
	{
#ifndef QT_NO_DEBUG
		qDebug(qPrintable(trUtf8("QFrankSSLZertifikatspeicher XML speichern: %1speicher konnte nicht geöffnet werden.","debug").arg(K_SpeichertypeText)));
#endif
		emit Fehler(trUtf8("Der %1speicher konnte nicht überschrieben werden.").arg(K_SpeichertypeText));
		return false;
	}
	//Mit oder ohne Verschlüsselung speichern???
	if(ort==QFrankSSLZertifikatspeicher::Nutzer)
	{
		//mit
		QFrankDatenstromfilter Verschluesselung(&Datei,&K_Passwort);
		if(Verschluesselung.write(dokument->toByteArray())==-1)
		{
			Verschluesselung.close();
#ifndef QT_NO_DEBUG
			qDebug(qPrintable(QString("QFrankSSLZertifikatspeicher XML speichern: %1speicher konnte nicht geschrieben werden. Ursache:\r\n%2")
										.arg(K_SpeichertypeText).arg(Verschluesselung.errorString())));
#endif
			emit Fehler(trUtf8("Fehler beim schreiben auf den Datenträger währen des Schreibes des %1speichers. Ursache:\r\n%2").arg(K_SpeichertypeText)
																																.arg(Verschluesselung.
																																	 errorString()));
			return false;
		}
		Verschluesselung.close();
	}
	else
	{
		//ohne
		if(Datei.write(dokument->toByteArray())==-1)
		{
			Datei.close();
#ifndef QT_NO_DEBUG
			qDebug(qPrintable(QString("QFrankSSLZertifikatspeicher XML speichern: %1speicher konnte nicht geschrieben werden. Ursache:\r\n%2")
										.arg(K_SpeichertypeText).arg(Datei.errorString())));
#endif
			emit Fehler(trUtf8("Fehler beim schreiben auf den Datenträger währen des Schreibes des %1speichers. Ursache:\r\n%2").arg(K_SpeichertypeText)
																																.arg(Datei.errorString()));
			return false;
		}
		Datei.close();
	}
	return true;
}

bool QFrankSSLZertifikatspeicher::K_XMListZertspeicher(QDomDocument *xml)
{
	QDomElement Root=xml->documentElement();
	if(Root.tagName()!="Zertifikatsspeicher")
	{
#ifndef QT_NO_DEBUG
		qWarning("QFrankSSLZertifikatspeicher ist es ein Zertspeicher: XML Objekt ist kein Zertifikatsspeicher");
		qDebug("\t%s",qPrintable(Root.tagName()));
#endif
		return false;
	}
	return true;
}

bool QFrankSSLZertifikatspeicher::K_XMLBearbeiten(QDomDocument *xml)
{	
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
			if(!K_XMLEintragLesen(QFrankSSLZertifikatspeicher::CRL,&Eintrag.firstChild()))
				return false;			
		}
		else if(Element.tagName()=="CA")
		{
#ifndef QT_NO_DEBUG
			qDebug("Element CA gefunden");
#endif
			//durchsuchen aller CA's
			if(!K_XMLEintragLesen(QFrankSSLZertifikatspeicher::CA,&Eintrag.firstChild()))
				return false;		
		}
		Eintrag=Eintrag.nextSibling();
	}
	return true;
}

bool QFrankSSLZertifikatspeicher::K_XMLEintragLesen(const QFrankSSLZertifikatspeicher::Zertifikatstype &type,QDomNode *eintrag)
{
	//Alle Einträge bearbeiten
	BIO *Puffer = BIO_new(BIO_s_mem());
	QByteArray DatenDesEintrags;
	QDomElement Element;
	X509 *Zertifikat;
	X509_CRL *Rueckrufliste;
	bool Fehler=false;
	while(!eintrag->isNull())
	{
		Element=eintrag->toElement();
#ifndef QT_NO_DEBUG
		qDebug(qPrintable(QString("Inhalt des Elements: %1").arg(Element.text())));		
#endif
		DatenDesEintrags=QByteArray::fromBase64(Element.text().toAscii());
		BIO_reset(Puffer);
		Rueckrufliste=NULL;
		Zertifikat=NULL;
		if(BIO_write(Puffer,DatenDesEintrags.data(),DatenDesEintrags.size())!=DatenDesEintrags.size())
			qFatal("K_EintragBearbeiten es wurde nicht alle Bytes in den OpenSSL Puffer geschrieben");
		switch(type)
		{
			case QFrankSSLZertifikatspeicher::CRL:
													//if(!PEM_read_bio_X509_CRL(Puffer,&Rueckrufliste,0,NULL))
													if(d2i_X509_CRL_bio(Puffer,&Rueckrufliste)==NULL)
													{
#ifndef QT_NO_DEBUG
														qDebug("CRL konnte nicht gelesen werden");
#endif	
														Fehler=true;
														break;
													}
#ifndef QT_NO_DEBUG
													BIO_reset(Puffer);
													if(X509_CRL_print(Puffer,Rueckrufliste)==1)
													{
														QByteArray Zerttext;
														int Groesse=BIO_ctrl(Puffer,BIO_CTRL_PENDING,0,NULL);
														Zerttext.resize(Groesse);
														BIO_read(Puffer,Zerttext.data(),Groesse);
														qDebug(qPrintable(QString("CRL Element dekodiert: %1").arg(QString(Zerttext))));
													}													
#endif
													break;
			case QFrankSSLZertifikatspeicher::CA:
													if(d2i_X509_bio(Puffer,&Zertifikat)==NULL)
													//if(!PEM_read_bio_X509(Puffer,&Zertifikat,0,NULL))
													{
#ifndef QT_NO_DEBUG
														qDebug("CA konnte nicht gelesen werden");
#endif													
														Fehler=true;
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
		if(Rueckrufliste!=NULL)
			X509_CRL_free(Rueckrufliste);
	}
	BIO_free(Puffer);
	return !Fehler;
}


void QFrankSSLZertifikatspeicher::ZertifikatSpeichern(const QFrankSSLZertifikatspeicher::Speicherort &ort,
													  const QFrankSSLZertifikatspeicher::Zertifikatstype &type,const QString &datei)
{
	//Wenn wir ein Zert in den Nutzerspeicher schreiben sollen, dann braucht wir ein Passwort.
	if(ort==QFrankSSLZertifikatspeicher::Nutzer)
	{
		if(!K_PasswortGesetzt)
		{
			//Passwort für den Nutzerspeicher abfragen
			K_ZertspeicherAktion=QFrankSSLZertifikatspeicher::Speichern;
			K_Speicherort=ort;
			K_Zerttype=type;
			K_Zertdatei=datei;
			emit PasswortFuerDenSpeicherHohlen();
			return;
		}
	}
	QDomDocument* Speicher=new QDomDocument();
	if(!K_XMLLaden(Speicher,ort))
	{
#ifndef QT_NO_DEBUG
		qDebug("QFrankSSLZertifikatspeicher ZertifikatSpeichern: Speicher wurde nicht geladen.");
#endif
		delete Speicher;
		K_PasswortLoeschen();
		return;
	}
	//Eintrag in das XML Modell einfügen
	if(!K_XMLEintragSchreiben(type,datei,Speicher))
	{
		delete Speicher;
		K_PasswortLoeschen();
		return;
	}
	//XML Datei speichern
	if(!K_XMLSpeichern(Speicher,ort))
	{
		delete Speicher;
		K_PasswortLoeschen();
		return;
	}
	delete Speicher;
	K_PasswortLoeschen();
	return;
}

bool QFrankSSLZertifikatspeicher::K_XMLEintragSchreiben(const QFrankSSLZertifikatspeicher::Zertifikatstype &type,const QString &quellDatei,QDomDocument *xml)
{
	QByteArray EintragWert;
	if(!K_ZertifikatsdateiLaden(quellDatei,EintragWert))
	{
		emit Fehler(tr("Die Datei %1 konnte nicht ausgelesen werden.").arg(quellDatei));
		return false;
	}
	//Datei in ein DER Zert/CRL wandeln
	if(!K_FeldNachZert(EintragWert,type,true))
		return false;
	QDomElement Wurzel=xml->documentElement();
	QString EintragtypeBezeichnung;
	switch(type)
	{
		case QFrankSSLZertifikatspeicher::CRL:
												EintragtypeBezeichnung="CRL";
												break;
		case QFrankSSLZertifikatspeicher::CA:
												EintragtypeBezeichnung="CA";
												break;
		default:
												qFatal(qPrintable(trUtf8("QFrankSSLZertifikatspeicher Eintrag schreiben: unzulässiger Eintragstype","debug")));
												break;
	}
	QDomElement Type=xml->createElement(EintragtypeBezeichnung);
	Wurzel.appendChild(Type);
	QDomElement Eintrag=xml->createElement("Daten");
	Type.appendChild(Eintrag);
	QDomText Inhalt = xml->createTextNode(EintragWert.toBase64());
	Eintrag.appendChild(Inhalt);
	return true;
}

bool QFrankSSLZertifikatspeicher::K_FeldNachZert(QByteArray &feld,const QFrankSSLZertifikatspeicher::Zertifikatstype &type,bool wandeln)
{
	//Zuerst versuchen wir es mit einer DER Kodierung, dann mit einer PEM
	X509 *Zertifikat;
	X509_CRL *Rueckrufliste;
	BIO *Puffer = BIO_new(BIO_s_mem());
	Rueckrufliste=NULL;
	Zertifikat=NULL;
	if(BIO_write(Puffer,feld.data(),feld.size())!=feld.size())
			qFatal("K_FeldNachZert es wurde nicht alle Bytes in den OpenSSL Puffer geschrieben");
	bool Fehler=false;
	switch(type)
	{
		case QFrankSSLZertifikatspeicher::CRL:
												if(!d2i_X509_CRL_bio(Puffer,&Rueckrufliste))
												{
#ifndef QT_NO_DEBUG
													qDebug("QFrankSSLZertifikatspeicher FeldNachZert: CRL war nicht DER kodiert, versuche PEM");
#endif
													BIO_reset(Puffer);
													if(BIO_write(Puffer,feld.data(),feld.size())!=feld.size())
														qFatal("K_FeldNachZert es wurde nicht alle Bytes in den OpenSSL Puffer geschrieben");
													if(!PEM_read_bio_X509_CRL(Puffer,&Rueckrufliste,0,NULL))
													{
#ifndef QT_NO_DEBUG
														qDebug("QFrankSSLZertifikatspeicher FeldNachZert: CRL war nicht PEM kodiert, das wars");
#endif
														Fehler=true;
														emit QFrankSSLZertifikatspeicher::Fehler(trUtf8("Die Rückrufliste, war weder im DER noch im PEM Format."));
														break;
													}
												}
												else
													break;
												//wandeln nach DER wenn gewünscht, da CRL im PEM Format
												if(wandeln)
												{
													BIO_reset(Puffer);
													if(i2d_X509_CRL_bio(Puffer,Rueckrufliste)!=1)
													{
#ifndef QT_NO_DEBUG
														qDebug("QFrankSSLZertifikatspeicher FeldNachZert: wandeln nicht erfolgreich");
#endif
														Fehler=true;
														emit QFrankSSLZertifikatspeicher::Fehler(trUtf8("Die Rückrufliste konnte nicht in das DER Format "
																										"konvertiert werden."));
													}
													else
													{
														//Kovertierung ok
														int warteneBytes=BIO_ctrl(Puffer,BIO_CTRL_PENDING,0,NULL);
														feld.resize(warteneBytes);
														if(BIO_read(Puffer,feld.data(),warteneBytes)!=warteneBytes)
															qFatal("K_FeldNachZert es wurden nicht alle Bytes aus dem OpenSSL Puffer gelesen");
													}
												}
												break;
		case QFrankSSLZertifikatspeicher::CA:
												if(!d2i_X509_bio(Puffer,&Zertifikat))
												{
#ifndef QT_NO_DEBUG
													qDebug("QFrankSSLZertifikatspeicher FeldNachZert: CA war nicht DER kodiert, versuche PEM");
#endif
													BIO_reset(Puffer);
													if(BIO_write(Puffer,feld.data(),feld.size())!=feld.size())
														qFatal("K_FeldNachZert es wurde nicht alle Bytes in den OpenSSL Puffer geschrieben");
													if(!PEM_read_bio_X509(Puffer,&Zertifikat,0,NULL))
													{
#ifndef QT_NO_DEBUG
														qDebug("QFrankSSLZertifikatspeicher FeldNachZert: CA war nicht PEM kodiert, das wars");
#endif
														Fehler=true;
														emit QFrankSSLZertifikatspeicher::Fehler(tr("Das Zertifikat, war weder im DER noch im PEM Format."));
														break;
													}
												}
												else
													break;
												//wandeln nach DER wenn gewünscht, da Zert im PEM Format
												if(wandeln)
												{
													BIO_reset(Puffer);
													if(i2d_X509_bio(Puffer,Zertifikat)!=1)
													{
#ifndef QT_NO_DEBUG
														qDebug("QFrankSSLZertifikatspeicher FeldNachZert: wandeln nicht erfolgreich");
#endif
														Fehler=true;
														emit QFrankSSLZertifikatspeicher::Fehler(tr("Das Zertifikat konnte nicht in das DER Format "
																									"konvertiert werden."));
													}
													else
													{
														//Kovertierung ok
														int warteneBytes=BIO_ctrl(Puffer,BIO_CTRL_PENDING,0,NULL);
														feld.resize(warteneBytes);
														if(BIO_read(Puffer,feld.data(),warteneBytes)!=warteneBytes)
															qFatal("K_FeldNachZert es wurden nicht alle Bytes aus dem OpenSSL Puffer gelesen");
													}
												}
												break;
		default:
												qFatal(qPrintable(trUtf8("QFrankSSLZertifikatspeicher FeldNachZert: unzulässiger Eintragstype","debug")));
												break;
	}
	if(Zertifikat!=NULL)
		X509_free(Zertifikat);
	if(Rueckrufliste!=NULL)
		X509_CRL_free(Rueckrufliste);
	BIO_free(Puffer);
	return !Fehler;
}

bool QFrankSSLZertifikatspeicher::K_ZertifikatsdateiLaden(const QString &quellDatei,QByteArray &daten)
{
	QFile Datei(quellDatei);
	if(Datei.open(QIODevice::ReadOnly))
	{
		daten=Datei.readAll();
		if(!daten.isNull())
			return true;
	}
	return false;
}

void QFrankSSLZertifikatspeicher::loeschen(const QFrankSSLZertifikatspeicher::Speicherort &ort)
{
	K_SpeichertypeTextSetzen(ort);
	QFile Datei(K_DateinameBenutzer);
	if(ort==QFrankSSLZertifikatspeicher::System)
		Datei.setFileName(K_DateinameSystem);
	if(!Datei.remove())
	{
#ifndef QT_NO_DEBUG
		qCritical(qPrintable(trUtf8("QFrankSSLZertifikatspeicher Speicher löschen: %1 gescheitert.","debug").arg(Datei.fileName())));
#endif
		emit Fehler(trUtf8("Der %1speicher konnte nicht gelöscht werden.").arg(K_SpeichertypeText));
		return;
	}
#ifndef QT_NO_DEBUG
	qDebug(qPrintable(trUtf8("QFrankSSLZertifikatspeicher Speicher löschen: %1 gelöscht","debug").arg(Datei.fileName())));
#endif
}

void QFrankSSLZertifikatspeicher::PasswortFuerDenSpeicher(QString &passwort)
{
	K_Passwort=passwort;
	K_PasswortGesetzt=true;
	passwort.clear();
	switch(K_ZertspeicherAktion)
	{
		case QFrankSSLZertifikatspeicher::Laden:
													SpeicherLaden();
													break;
		case QFrankSSLZertifikatspeicher::Speichern:
													ZertifikatSpeichern(K_Speicherort,K_Zerttype,K_Zertdatei);
													break;
	}
}

void QFrankSSLZertifikatspeicher::K_PasswortLoeschen()
{
	K_Passwort.clear();
	K_PasswortGesetzt=false;
	K_ZertspeicherAktion=QFrankSSLZertifikatspeicher::Nichts;
}
#else
bool QFrankSSLZertifikatspeicher::K_SpeicherLaden(const QFrankSSLZertifikatspeicher::Speicherort &type)
{
	K_SpeichertypeTextSetzen(type);
	//CERT_STORE_PROV_SYSTEM_A = Alle vorhandene Zerts
	//Sytstem CERT_SYSTEM_STORE_LOCAL_MACHINE
	//Nutzer CERT_SYSTEM_STORE_CURRENT_USER
	HCERTSTORE  Zertifikatsspeicher;
	QString Speicher;
	if(type==QFrankSSLZertifikatspeicher::Nutzer)
		Speicher="My";
	else
		Speicher="Root";
	Zertifikatsspeicher=CertOpenStore(CERT_STORE_PROV_SYSTEM,X509_ASN_ENCODING,NULL,CERT_STORE_READONLY_FLAG,Speicher.utf16());
	if(Zertifikatsspeicher==NULL)
	{
#ifndef QT_NO_DEBUG
		DWORD Fehlerkode=GetLastError();
		qDebug("Fehler: %d",Fehlerkode);
		qCritical(qPrintable(QString("QFrankSSLZertifikatspeicher K_SpeicherLaden: %1speicher laden fehlgeschalgen").arg(K_SpeichertypeText)));
#endif
		return false;
	}
	if(!CertCloseStore(Zertifikatsspeicher,CERT_CLOSE_STORE_FORCE_FLAG))
		qFatal(qPrintable(trUtf8("QFrankSSLZertifikatspeicher K_SpeicherLaden: konnte den %1speicher nicht schließen.").arg(K_SpeichertypeText)));
#ifndef QT_NO_DEBUG
	qDebug(qPrintable(QString("QFrankSSLZertifikatspeicher K_SpeicherLaden: %1speicher geladen").arg(K_SpeichertypeText)));
#endif
	return true;
}
#endif
