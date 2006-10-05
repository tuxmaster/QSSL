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

#ifndef QFRANKZERTKONFDLGHAUPT
#define QFRANKZERTKONFDLGHAUPT

#include <QtGui>
#include "ui_KonfigBasis.h"

class QFrankSSL;

class QFrankZertkonfDlgHaupt :public QMainWindow,private Ui::DlgKonfigBasis 
{
	Q_OBJECT
	public:
			QFrankZertkonfDlgHaupt(QWidget *eltern=0);

	private slots:

			void			on_Menuepunkt_CRL_triggered();
			void			on_Menuepunkt_Zertifikat_triggered();		
			void			on_Menuepunkt_ZertifikateAnzeigen_triggered();
			void			on_Menuepunkt_RueckruflistenAnzeigen_triggered();
			void			on_Menuepunkt_ueberQt_triggered();
			void			on_Menuepunkt_ueber_triggered();
			void			on_Menuepunkt_GPL_Lizenz_triggered();
			void			K_SpeicherortGeaendert(const int &aktiv);
			void			K_Fehler(const QString &fehler);

	private:
			QFrankSSL		*SSLSystem;
			int				K_Speicherort;
			QButtonGroup	*K_SpeicherortGruppe;
};
#endif
