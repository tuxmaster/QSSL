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

#include "DlgHaupt.h"
#include <qssl.h>

QFrankDlgHaupt::QFrankDlgHaupt(QWidget* eltern):QDialog(eltern)
{
	setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);
	setupUi(this);
	//Qt Nachrichten abfangen
	Debugausgabe=txtDebug;
	qInstallMsgHandler(QtNachrichten);
	K_SSL=new QFrankSSL(this);
}

QFrankDlgHaupt::~QFrankDlgHaupt()
{
	//Qt übernimmt wieder
	qInstallMsgHandler(0);
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
