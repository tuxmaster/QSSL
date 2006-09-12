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
#ifndef QFRANKDATENSTROMFILTER_H
#define QFRANKDATENSTROMFILTER_H

#include <QtCore>

typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;

class QFrankDatenstromfilter: public  QIODevice
{
	Q_OBJECT
	public:
				QFrankDatenstromfilter(QIODevice *quelldatenstrom,QString *schluessel);
				~QFrankDatenstromfilter();
				bool			open(OpenMode strommodus);
				//Der Filter ist nur sequenziell
				bool			isSequential()const{return true;}
				bool			seek(qint64 position); 
				bool			reset();
				//Squenzieller Datenstrom hat keine Position
				qint64			pos()const{return 0;}
				void			close();
					
	private:
				qint64			readData(char *daten,qint64 maximaleLaenge);
				qint64			writeData(const char *daten, qint64 maximaleLaenge);
				QIODevice*		K_Quelldatenstrom;
				EVP_CIPHER_CTX*	K_Verschluesseln;
				EVP_CIPHER_CTX*	K_Entschluesseln;

#ifndef QT_NO_DEBUG
				QString			K_FeldNachHex(const QByteArray &feld) const;
#endif
};
#endif
