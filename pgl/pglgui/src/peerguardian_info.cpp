/***************************************************************************
 *   Copyright (C) 2007-2008 by Dimitris Palyvos-Giannas   *
 *   jimaras@gmail.com   *
 *   Copyright (C) 2011 Carlos Pais
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#include "peerguardian_info.h"
#include "utils.h"

PeerguardianInfo::PeerguardianInfo( const QString &logPath, QObject *parent ) :
	QObject( parent )
{
	m_DaemonState = false;
    m_LogPath = logPath;
    getLoadedIps();
    updateDaemonState();
    mTimer = new QTimer(this);
    mTimer->setInterval(TIMER_INTERVAL);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(updateDaemonState()));
    mTimer->start();
}

void PeerguardianInfo::getLoadedIps()
{
    
    if ( QFile::exists(m_LogPath) )
    {
        QStringList fileData = getFileData( m_LogPath );
        QString blocklist_line("");
        
        for(int i=0; i < fileData.size(); i++)
        {
            if ( fileData[i].contains("INFO:") && fileData[i].contains("Blocking") )
                blocklist_line = fileData[i];
        }
        
        if ( ! blocklist_line.isEmpty() )
        {
            QStringList parts = blocklist_line.split("INFO:", QString::SkipEmptyParts);
            m_LoadedRanges = parts[1];
        } 
    }
}

QString PeerguardianInfo::checkProcessID() {

	QString command = "pidof";

	QProcess ps;
	ps.start( command, QStringList() << DAEMON );
	if ( ! ps.waitForStarted() ) {
		qWarning() << Q_FUNC_INFO << "Could not get process status for pgl.";
	}
	ps.closeWriteChannel();
	ps.waitForFinished();

    return ps.readAll().trimmed();
}

void PeerguardianInfo::updateDaemonState()
{
    QString pid = checkProcessID();
    bool daemonState = !pid.isEmpty();
	
    if ( pid != m_ProcessID ) {
        m_ProcessID = pid;
        emit processIDChanged( m_ProcessID );
	}

    if ( daemonState != m_DaemonState ) {
        m_DaemonState = daemonState;
        emit stateChanged( m_DaemonState );
	}
}

void PeerguardianInfo::processDate( QString &date ) {

	static QString prevDate;
	static QString prevResult;
	
	//Calculate the result only one time if the date is the same
	if ( prevDate == date ) {
		date = prevResult;
		return;
	}

	prevDate = date;

	QString year = date.section("-", 0, 0 );
	QString month = date.section( "-", 1, 1 );
	QString day = date.section( "-", 2, 2 );

	month = ( month.startsWith( "0" )  ?  QString( month[1] )  : month );
	day = ( day.startsWith( "0" )  ? QString( day[1] ) : day );

	date = tr( "%1/%2/%3" ).arg(day).arg(month).arg(year);
	prevResult = date;

}


