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

#ifndef QFRANKDLGHAUPT_H
#define QFRANKDLGHAUPT_H

#include "ui_DemoBasis.h"

static void						QtNachrichten(QtMsgType type, const char *msg);
static QTextEdit*				Debugausgabe;

class QFrankSSL;

class QFrankDlgHaupt: public QDialog, private Ui::DlgDemoBasis
{
	Q_OBJECT
	public:
					QFrankDlgHaupt(QWidget* eltern=0);				
					~QFrankDlgHaupt();
	private:
					QFrankSSL*	K_SSL;
					bool		K_VerbindenTrennen;
					void		SindAlleSSLVersionenDeaktiviert();
					void		K_SteuerschaltflaechenFreigeben(const bool& freigeben=false);

	private slots:
					void		on_cbVerschluesselungFestlegen_stateChanged(int zustand);
					void		on_cbSSL2_stateChanged(int zustand);
					void		on_cbSSL3_stateChanged(int zustand);
					void		on_cbTLS1_stateChanged(int zustand);
					void		on_sfVerbinden_released();
					void		on_sfSenden_released();
					void		on_txtDebug_textChanged();
					void		on_txtFehler_textChanged();
					void		on_txtEmpfangen_textChanged();
					void		K_EsGabEinFehler(const QString & fehlertext);
					void		K_DatenSindDa(const QByteArray &daten);
					void		K_TunnelAufgebaut();
					void		K_VerbindungGetrennt(const bool &mitFehler);
};

#endif
