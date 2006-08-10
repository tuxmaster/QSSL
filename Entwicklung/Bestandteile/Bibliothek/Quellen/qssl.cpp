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

#include "qssl.h"

QFrankSSL::QFrankSSL(QObject* eltern): QTcpSocket(eltern)
{
	//OpenSSL initialisieren
	SSL_load_error_strings();
	SSL_library_init();
	//OpenSSL Verbindung aufbauen, hier lassen wir erst mal alle SSL Versionen zu.
	K_OpenSSLVerbindung=SSL_CTX_new(SSLv23_client_method());	
}

QFrankSSL::~QFrankSSL()
{
	//OpenSSL aufräumen
	ERR_free_strings();
	SSL_CTX_free(K_OpenSSLVerbindung);
}