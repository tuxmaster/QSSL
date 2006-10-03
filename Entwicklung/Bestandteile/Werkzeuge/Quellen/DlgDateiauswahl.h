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

#ifndef QFRANKZERTKONFDLGDATEIAUSWAHL
#define QFRANKZERTKONFDLGDATEIAUSWAHL

#include <QtGui>
#include "ui_DateiauswahlBasis.h"

class QFrankZertkonfDlgDateiauswahl:public QDialog,private Ui::DlgDateiauswahlBasis
{
	Q_OBJECT
	public:
				enum			TypeDerDatei{ZERTPEM=0x00,ZERTDER=0x01,CRLPEM=0x02,CRLDER=0x03};
				Q_DECLARE_FLAGS(Dateitype,TypeDerDatei)
				QFrankZertkonfDlgDateiauswahl(QWidget* eltern,const QFrankZertkonfDlgDateiauswahl::Dateitype &dateitype);
				void			TitelSetzen(const QString &titel);
				const QString	Datei()const{return txtDatei->text();}

	private:
				QFrankZertkonfDlgDateiauswahl::Dateitype	K_Dateitype;

	private slots:
				void			DateiAngekommen(const QString datei);
				void			on_sfDateiauswahl_clicked();
				void			on_sfOK_clicked();
};

#endif
