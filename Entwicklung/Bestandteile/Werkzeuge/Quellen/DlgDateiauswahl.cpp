/*
 *  Copyright (C) 2006 Frank B�ttner frank-buettner@gmx.net
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

QFrankZertkonfDlgDateiauswahl::QFrankZertkonfDlgDateiauswahl(QWidget* eltern):QDialog(eltern)
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
}

void QFrankZertkonfDlgDateiauswahl::DateiAngekommen(const QString datei)
{
	QMessageBox::information(this,"Drop angekommen",datei,QMessageBox::Ok,QMessageBox::NoButton);
}
