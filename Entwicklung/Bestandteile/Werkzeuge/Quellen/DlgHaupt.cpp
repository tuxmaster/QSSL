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
}

void QFrankZertkonfDlgHaupt::on_Menuepunkt_ZertifikatPEMkodiert_triggered()
{
	QFrankZertkonfDlgDateiauswahl Dialog(this,QFrankZertkonfDlgDateiauswahl::ZERTPEM);
	Dialog.TitelSetzen(tr("Zertifikat PEM kodiert"));
	if(Dialog.exec()==QDialog::Rejected)
		return;
	QMessageBox::information(this,"",Dialog.Datei(),QMessageBox::Ok);
}

void QFrankZertkonfDlgHaupt::on_Menuepunkt_ZertifikatDERkodiert_triggered()
{
	QFrankZertkonfDlgDateiauswahl Dialog(this,QFrankZertkonfDlgDateiauswahl::ZERTDER);
	Dialog.TitelSetzen(tr("Zertifikat DER kodiert"));
	if(Dialog.exec()==QDialog::Rejected)
		return;
}

void QFrankZertkonfDlgHaupt::on_Menuepunkt_CRL_PEMkodiert_triggered()
{
	QFrankZertkonfDlgDateiauswahl Dialog(this,QFrankZertkonfDlgDateiauswahl::CRLPEM);
	Dialog.TitelSetzen(trUtf8("Rückrufliste PEM kodiert"));
	if(Dialog.exec()==QDialog::Rejected)
		return;
}

void QFrankZertkonfDlgHaupt::on_Menuepunkt_CRL_DERkodiert_triggered()
{
	QFrankZertkonfDlgDateiauswahl Dialog(this,QFrankZertkonfDlgDateiauswahl::CRLDER);
	Dialog.TitelSetzen(trUtf8("Rückrufliste DER kodiert"));
	if(Dialog.exec()==QDialog::Rejected)
		return;
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
