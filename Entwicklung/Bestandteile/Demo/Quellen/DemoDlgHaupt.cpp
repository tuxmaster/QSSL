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

#include <QtGui>

#include "DemoDlgHaupt.h"
#include <qssl.h>

QFrankDlgHaupt::QFrankDlgHaupt(QWidget* eltern):QDialog(eltern)
{
	K_VerbindenTrennen=true;
	setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);
	setupUi(this);
	//Qt Nachrichten abfangen
	Debugausgabe=txtDebug;
	qInstallMsgHandler(QtNachrichten);
	K_SSL=new QFrankSSL(this);
	connect(K_SSL,SIGNAL(SSLFehler(const QString)),this,SLOT(K_EsGabEinFehler(const QString&)));
	connect(K_SSL,SIGNAL(DatenBereitZumAbhohlen(const QByteArray&)),this,SLOT(K_DatenSindDa(const QByteArray&)));
	connect(K_SSL,SIGNAL(TunnelBereit()),this,SLOT(K_TunnelAufgebaut()));
	connect(K_SSL,SIGNAL(VerbindungGetrennt(const bool)),this,SLOT(K_VerbindungGetrennt(const bool&)));
	connect(K_SSL->Zertifikatsspeicher(),SIGNAL(PasswortFuerDenSpeicherHohlen()),this,SLOT(K_PasswortAbfragen()));
	QTimer::singleShot(0,this,SLOT(K_ZertifikateLaden()));	
}

QFrankDlgHaupt::~QFrankDlgHaupt()
{
	//Qt übernimmt wieder
	qInstallMsgHandler(0);
}

void QFrankDlgHaupt::on_cbVerschluesselungFestlegen_stateChanged(int zustand)
{
	if(zustand==Qt::Checked)
	{
		txtVerschluesselungen->setEnabled(true);
	}
	else
	{
		txtVerschluesselungen->setEnabled(false);
	}
}

void QFrankDlgHaupt::SindAlleSSLVersionenDeaktiviert()
{
	if(cbSSL2->checkState()==Qt::Unchecked && cbSSL3->checkState()==Qt::Unchecked && cbTLS1->checkState()==Qt::Unchecked)
	{
		QMessageBox::critical(this,"Sinnlose Auswahl",trUtf8("Es können nicht alle SSL Versionen deaktivert werden!!"),QMessageBox::Ok,QMessageBox::NoButton);
		sfVerbinden->setEnabled(false);
	}
	else
	{
		sfVerbinden->setEnabled(true);
	}
}

void QFrankDlgHaupt::on_cbSSL2_stateChanged(int zustand)
{
	SindAlleSSLVersionenDeaktiviert();
}

void QFrankDlgHaupt::on_cbSSL3_stateChanged(int zustand)
{
	SindAlleSSLVersionenDeaktiviert();
}

void QFrankDlgHaupt::on_cbTLS1_stateChanged(int zustand)
{
	SindAlleSSLVersionenDeaktiviert();
}

void QFrankDlgHaupt::K_SteuerschaltflaechenFreigeben(const bool& freigeben)
{
	txtServer->setEnabled(freigeben);

	intPort->setEnabled(freigeben);

	sfVerbinden->setEnabled(freigeben);

	cbVerschluesselungFestlegen->setEnabled(freigeben);
	cbSSL2->setEnabled(freigeben);
	cbSSL3->setEnabled(freigeben);
	cbTLS1->setEnabled(freigeben);
}

void QFrankDlgHaupt::on_sfVerbinden_released()
{
	if(K_VerbindenTrennen)
	{
		//Verbinden mit SSL Server
		sfVerbinden->setText("Trennen");
		
		K_SteuerschaltflaechenFreigeben();

		K_VerbindenTrennen=false;
		if(cbVerschluesselungFestlegen->checkState()==Qt::Checked)
			K_SSL->VerfuegbareAlgorithmenFestlegen(txtVerschluesselungen->text().split(":"));
		char SSLVersionen=0;
		if (cbSSL2->checkState()==Qt::Checked)
			SSLVersionen=SSLVersionen|QFrankSSL::SSLv2;
		if (cbSSL3->checkState()==Qt::Checked)
			SSLVersionen=SSLVersionen|QFrankSSL::SSLv3;
		if (cbTLS1->checkState()==Qt::Checked)
			SSLVersionen=SSLVersionen|QFrankSSL::TLSv1;
		if(SSLVersionen!=0)
			K_SSL->SSLVersionenFestlegen((QFrankSSL::SSLVersion)SSLVersionen);
		K_SSL->VerbindungHerstellen(txtServer->text(),intPort->value());
	}
	else
	{
		//Trennen vom SSL Server
		sfVerbinden->setText("Verbinden");
		K_SteuerschaltflaechenFreigeben(true);
		K_SSL->VerbindungTrennen();
		K_VerbindenTrennen=true;
	}
}

void QFrankDlgHaupt::K_EsGabEinFehler(const QString & fehlertext)
{
	if(fehlertext!="")
		txtFehler->append(fehlertext);
	on_sfVerbinden_released();
	sfVerbinden->setEnabled(true);
	sfSenden->setEnabled(false);
}

void QFrankDlgHaupt::K_TunnelAufgebaut()
{
	txtSenden->setEnabled(true);
	sfSenden->setEnabled(true);
	txtSenden->setFocus();
	sfVerbinden->setEnabled(true);

}

void QFrankDlgHaupt::K_PasswortAbfragen()
{
	K_SSL->Zertifikatsspeicher()->PasswortFuerDenSpeicher(&QInputDialog::getText(this,tr("Passwortabfrage"),trUtf8("Bitte geben Sie das Passwort für den"
																													"Zertifikatspeicher ein."),
															QLineEdit::NoEcho,QString(),0,(Qt::WFlags)Qt::Widget^Qt::WindowTitleHint));
}

void QFrankDlgHaupt::K_VerbindungGetrennt(const bool &mitFehler)
{
	if(!mitFehler)
	{
		qDebug("Demo: Verbindung ordenlich getrennt");
		K_EsGabEinFehler("");
	}
	else
	{
		qDebug("Demo: Verbindung nicht ordenlich getrennt");
		K_EsGabEinFehler("");
	}
	txtSenden->setEnabled(false);
}

void QFrankDlgHaupt::K_DatenSindDa(const QByteArray &daten)
{
	txtEmpfangen->append(daten);
}

void QFrankDlgHaupt::on_sfSenden_released()
{
	K_SSL->DatenSenden(txtSenden->text().toUtf8()+"\r\n");
	txtSenden->setFocus();
}

void QFrankDlgHaupt::on_txtDebug_textChanged()
{
	txtDebug->verticalScrollBar()->setSliderPosition(txtDebug->verticalScrollBar()->maximum());
}

void QFrankDlgHaupt::on_txtFehler_textChanged()
{
	txtFehler->verticalScrollBar()->setSliderPosition(txtFehler->verticalScrollBar()->maximum());
}

void QFrankDlgHaupt::on_txtEmpfangen_textChanged()
{
	txtEmpfangen->verticalScrollBar()->setSliderPosition(txtEmpfangen->verticalScrollBar()->maximum());
}

void QFrankDlgHaupt::K_ZertifikateLaden()
{
	K_SSL->Zertifikatsspeicher()->SpeicherLaden();
}

void QtNachrichten(QtMsgType type, const char *msg)
{
	switch(type)
	{
		case QtDebugMsg:
							Debugausgabe->append(QString("<font color=#00ff00>Debug: <font color=black>%1").arg(msg));
							break;
		case QtWarningMsg:
							if(getenv("QT_FATAL_WARNINGS")==NULL)
								Debugausgabe->append(QString("<font color=#ffe000>Warnung: <font color=black>%1").arg(msg));
							else
								fprintf(stderr,"Warung(mit Fatal Funktion): %s\n", msg);
							break;
		case QtCriticalMsg:	
							Debugausgabe->append(QString("<font color=red>Kritisch: <font color=black>%1").arg(msg));
							break;
#ifndef Q_CC_MSVC
		case QtFatalMsg:
							fprintf(stderr,"Fatal nix geht mehr: %s\n", msg);
							abort();
#endif
	}
}
