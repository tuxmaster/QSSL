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

#ifndef QFRANKSSL_H
#define QFRANKSSL_H

#include <QtNetwork>

//Unter Windows  braucht man Hilfe beim Exportieren
#ifdef BIBLIOTHEK_BAUEN
	#ifdef Q_WS_WIN
		#define DLL_BAUEN
	#endif
//Open SSL Header
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif
#ifdef Q_WS_WIN 
	#ifdef DLL_BAUEN
		#define DLL_EXPORT __declspec(dllexport)
	#else
			#define DLL_EXPORT __declspec(dllimport)
	#endif
#else
		#define DLL_EXPORT
#endif

class DLL_EXPORT QFrankSSL: public QTcpSocket
{
	Q_OBJECT
	public:
				QFrankSSL(QObject* eltern);
				~QFrankSSL();
				
	private:
				SSL_CTX*	K_OpenSSLVerbindung;
							
};
#endif