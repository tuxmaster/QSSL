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

#include "Ereignisfilter.h"

QFrankZertkonfEreignisfilter::QFrankZertkonfEreignisfilter(QObject* eltern):QObject(eltern)
{
}

bool QFrankZertkonfEreignisfilter::eventFilter(QObject *wer, QEvent *was)
{
	//Soll Drop vorbereitet werden??
	if(wer->objectName()=="sZiehflaeche" && was->type()==QEvent::DragEnter)
	{
		//Erstellen wir daraus mal ein QDragEnterEvent
		QDragEnterEvent *start=(QDragEnterEvent*)was;
		if(start->mimeData()->hasUrls())
		{
			if(QFile::exists(start->mimeData()->urls().at(0).toLocalFile()))
				start->acceptProposedAction();			
		}
		return true;
	}
	//Drop durchführen
	if(wer->objectName()=="sZiehflaeche" && was->type()==QEvent::Drop)
	{
		QDropEvent *ende=(QDropEvent*)was;
		QMetaObject::invokeMethod(wer->parent(),"DateiAngekommen",Qt::DirectConnection,Q_ARG(QString,ende->mimeData()->urls().at(0).toLocalFile()));
		ende->acceptProposedAction();
		return true;
	}
	// ist nix für unser Filter
	return QObject::eventFilter(wer, was);
}
