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

#include "DlgDateiauswahl.h"
#include "Ereignisfilter.h"

QFrankZertkonfDlgDateiauswahl::QFrankZertkonfDlgDateiauswahl(QWidget* eltern,const QFrankZertkonfDlgDateiauswahl::Dateitype &dateitype):QDialog(eltern)
{
	setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);
	setupUi(this);
	QFrankZertkonfEreignisfilter *Filter=new QFrankZertkonfEreignisfilter(this);
	sZiehflaeche->installEventFilter(Filter);
	QDesktopWidget *Desktop = QApplication::desktop(); //neue X und Y Koordinate
	int x=(Desktop->width()-this->width())/2;
	int y=(Desktop->height()-this->height())/2;
	//jetzt das Fenster verschieben
	this->move(x,y);
	K_Dateitype=dateitype;
}

void QFrankZertkonfDlgDateiauswahl::on_sfDateiauswahl_clicked()
{
	QString Filter;
	switch(K_Dateitype)
	{
		case QFrankZertkonfDlgDateiauswahl::ZERTPEM:
														Filter=tr("Zertifikat (*.pem *.cer)");
														break;
		case QFrankZertkonfDlgDateiauswahl::ZERTDER:
														Filter=tr("Zertifikat (*.der *.cer)");
														break;
		case QFrankZertkonfDlgDateiauswahl::CRLPEM:
		case QFrankZertkonfDlgDateiauswahl::CRLDER:
														Filter=trUtf8("Rückrufliste (*.crl)");
														break;
		default:
														qFatal("Falscher Dateitype");
														break;
	}
	QString Datei=QFileDialog::getOpenFileName(this,trUtf8("Bitte die Datei auswählen"),QString(),Filter);
	if(Datei.isNull())
		return;
	DateiAngekommen(Datei);
}

void QFrankZertkonfDlgDateiauswahl::on_sfOK_clicked()
{
	if(txtDatei->text().isEmpty())
	{
		QMessageBox::warning(this,tr("Fehler"),tr("Das Feld Dateiname darf nicht leer sein."),QMessageBox::Ok,QMessageBox::NoButton);
		return;
	}
	else
		this->accept();
}

void QFrankZertkonfDlgDateiauswahl::TitelSetzen(const QString &titel)
{
	this->setWindowTitle(titel);
}

void QFrankZertkonfDlgDateiauswahl::DateiAngekommen(const QString datei)
{
	txtDatei->setText(datei);
}
