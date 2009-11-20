/**************************************************************************
*   Copyright (C) 2009-2010 by Dimitris Palyvos-Giannas                   *
*   jimaras@gmail.com                                                     *
*                                                                         *
*   This is a part of pgl-gui.                                            *
*   pgl-gui is free software; you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#include "loghandler.h"

LogHandler::LogHandler( QObject *parent ) :
    QObject( parent ) {

}

LogHandler::LogHandler( const QString &daemonPath, const QString &cmdPath, QObject *parent ) :
    QObject( parent ) {

    setFilePaths( daemonPath, cmdPath );

}

LogHandler::~LogHandler() {

}

void LogHandler::setFilePaths( const QString &daemonPath, const QString &cmdPath ) {

    AbstractHandler::setFilePath( daemonPath, DAEMON_FILE_ID );
    AbstractHandler::setFilePath( cmdPath, CMD_FILE_ID );

}

bool LogHandler::isWorking() const {

    if ( AbstractHandler::getFilePath( DAEMON_FILE_ID ).isEmpty() ) {
        return false;
    }
    if ( AbstractHandler::getFilePath( CMD_FILE_ID ).isEmpty() ) {
        return false;
    }

    return true;

}

LogItem LogHandler::parseDaemonEntry( const QString &entry ) const {

    LogItem newItem;
    newItem.m_Type = PGLG_INGORE;

    //Separate the time and date from the rest of the item
   // QString 

}

#include "loghandler.moc"
