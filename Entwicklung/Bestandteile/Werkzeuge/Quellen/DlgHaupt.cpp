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

#include "DlgHaupt.h"
#include "DlgGPL.h"
#include "DlgDateiauswahl.h"
#include <qssl.h>

QFrankZertkonfDlgHaupt::QFrankZertkonfDlgHaupt(QWidget *eltern):QMainWindow(eltern)
{
	setupUi(this);
	QDesktopWidget *Desktop = QApplication::desktop(); //neue X und Y Koordinate
	int x=(Desktop->width()-this->width())/2;
	int y=(Desktop->height()-this->height())/2;
	//jetzt das Fenster verschieben
	this->move(x,y);
	K_SpeicherortGruppe=new QButtonGroup(this);
	K_SpeicherortGruppe->addButton(speicherNutzer,1);
	K_SpeicherortGruppe->addButton(speicherSystem,0);
	SSLSystem=new QFrankSSL(this);
	connect(K_SpeicherortGruppe,SIGNAL(buttonClicked(int)),this,SLOT(K_SpeicherortGeaendert(const int&)));
	connect(SSLSystem->Zertifikatsspeicher(),SIGNAL(Fehler(const QString&)),this,SLOT(K_Fehler(const QString&)));	
	K_Speicherort=QFrankSSLZertifikatspeicher::Nutzer;
}

void QFrankZertkonfDlgHaupt::on_Menuepunkt_Zertifikat_triggered()
{
	QFrankZertkonfDlgDateiauswahl Dialog(this,QFrankZertkonfDlgDateiauswahl::ZERT);
	Dialog.TitelSetzen(tr("Zertifikat importieren"));
	if(Dialog.exec()==QDialog::Rejected)
		return;
	SSLSystem->Zertifikatsspeicher()->ZertifikatSpeichern((QFrankSSLZertifikatspeicher::Speicherort)K_Speicherort,QFrankSSLZertifikatspeicher::CA,
														  QFile(Dialog.Datei()));
}


void QFrankZertkonfDlgHaupt::on_Menuepunkt_CRL_triggered()
{
	QFrankZertkonfDlgDateiauswahl Dialog(this,QFrankZertkonfDlgDateiauswahl::CRL);
	Dialog.TitelSetzen(trUtf8("Rückrufliste importieren"));
	if(Dialog.exec()==QDialog::Rejected)
		return;
	SSLSystem->Zertifikatsspeicher()->ZertifikatSpeichern((QFrankSSLZertifikatspeicher::Speicherort)K_Speicherort,QFrankSSLZertifikatspeicher::CRL,
														  QFile(Dialog.Datei()));
}

void QFrankZertkonfDlgHaupt::on_Menuepunkt_GPL_Lizenz_triggered()
{
	QFile Lizenzdatei(":/Lizenz.txt");
	if(!Lizenzdatei.open(QIODevice::ReadOnly|QIODevice::Text))
							qFatal("Programmresourcen sind schrott.");
	QFrankDlgGPL Dialog(this);
	Dialog.LizenzTextSetzen(QString(Lizenzdatei.readAll()));
	Dialog.exec();
	Lizenzdatei.close();
}

void QFrankZertkonfDlgHaupt::on_Menuepunkt_ZertifikateAnzeigen_triggered()
{
	QMessageBox::information(this,"Testliste für Zerts",SSLSystem->Zertifikatsspeicher()->ListeAllerZertifikate(QFrankSSLZertifikatspeicher::CA).join(","));
							
}

void QFrankZertkonfDlgHaupt::on_Menuepunkt_RueckruflistenAnzeigen_triggered()
{
}

void QFrankZertkonfDlgHaupt::K_Fehler(const QString &fehler)
{
	QMessageBox::critical(this,tr("Fehler"),tr("Folgener Fehler ist aufgetreten:\r\n%1").arg(fehler));
}

void QFrankZertkonfDlgHaupt::K_SpeicherortGeaendert(const int &aktiv)
{
	switch(aktiv)
	{
		case 0:
				K_Speicherort=QFrankSSLZertifikatspeicher::System;
				break;
		case 1:
				K_Speicherort=QFrankSSLZertifikatspeicher::Nutzer;
				break;
		default:
				qFatal("FrankZertkonfDlgHaupt SpeicherortGeaendert nicht definierter Speicherort");
				break;
	}
}

void QFrankZertkonfDlgHaupt::on_Menuepunkt_ueber_triggered()
{
	QMessageBox::about(this,trUtf8("Über QSSL konfig"),trUtf8("QSSL Zertifikatsspeicher Konfiguration Version %1, Urheberrecht(©) 2006 Frank Büttner.\r\n"
															  "QSSL Zertifikatsspeicher Konfiguration wird OHNE JEGLICHE GARANTIE bereitgestellt.\r\n"
															  "Weitere Hinweise entnehmen Sie bitte der Lizenz\r\n"
															  "Dies ist freie Software, Sie können sie unter Beachtung der GPL Lizenz weitergeben.\r\n"
															  "Diese Anwendung benutzt die OpenSSL Bibliothek.").arg(QFrankSSL::VersionText()));
}

void QFrankZertkonfDlgHaupt::on_Menuepunkt_ueberQt_triggered()
{
	QMessageBox::aboutQt(this);
}
