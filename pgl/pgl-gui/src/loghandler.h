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

#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <QObject>

#include <QString>
#include <QList>

#include "abstracthandler.h"

#define MAX_LOG_SIZE 100

/**
 * The message type a specific log entry
 */

enum LogItemType {
    PGLD_IN, //pgld blocked an incoming connection
    PGLD_OUT, //pgld blocked an outgoing connection
    PGLD_FWD, //pgld blocked a forwarded connection
    PGLD_SYSTEM, //pgld system message
    PGLD_ERROR, //pgld error message
    PGLC_SYSTEM, //pglcmd system message
    PGLC_ERROR, //pglcmd error message
    PGLG_INGORE //an invalid entry which should be ignored
};

/**
 * A struct representing the information contained in a log entry
 */

struct LogItem {
    QString m_RawMessage; //The raw data string from the log
    QString m_Date; //The date of the specific log entry
    QString m_Time; //The time of the specific log entry
    LogItemType m_Type; //The type of this entry
    //This either contains the description of the blocked host(in case of a hit entry) or the message without its timestamp
    QString m_Details;
    //The strings below are useful only for PGLD_IN, PGLD_OUT and PGLD_ERROR items.
    QString m_DestIP; //The IP of the destination of the connection
    QString m_DestPort; //The port of the destination of the connection
    QString m_SrcIP; //The IP of the source of the connection
    QString m_SrcPort; //The port of the source of the connection
    QString m_Protocol; //The protocol of the connection
};

/**
 * @brief LogHandler is responsible for parsing the logs created by both pgld and pglcmd.
 * 
 * After parsing the data, it stores them in a LogItem struct for easier access.
 * This class inherits from QObject in order to provide the necessary signals and slots.
 */

class LogHandler : public AbstractHandler, public QObject {

    Q_OBJECT

    public:
        /**
         * Default constructor. Does nothing
         * 
         * @param parent The parent of this object.
         */
        LogHandler( QObject *parent = 0 );
        /**
         * Alternative constructor.
         * 
         * This constructor not only creates a LogHandler object but it also tries to set the paths for the necessary log files.
         * @param daemonPath The path to the pgld log file.
         * @param cmdPath The path to the pglcmd log file.
         */
        LogHandler( const QString &daemonPath, const QString &cmdPath, QObject *parent = 0 );
        /**
         * Destructor.
         */
        ~LogHandler();
        /**
         * Searches for a stored LogItem with the specific destination IP.
         * 
         * 
         * @param destIP The destination IP of the specific log item.
         * @param type The type of the LogItem you want to find.
         * @return The LogItem with the specific properties or an empty LogItem with LogItemType = PGLG_IGNORE.
         */
        LogItem getItemByDestIP( const QString &ip, const LogItemType &type ) const;
        /**
        * Searches for a stored LogItem with the specific source IP.
        *
        *
        * @param srcIP The source IP of the specific log item.
        * @param type The type of the LogItem you want to find.
        * @return The LogItem with the specific properties or an empty LogItem with LogItemType = PGLG_IGNORE.
        */
        LogItem getItemBySrcIP( const QString &srcIP, const LogItemType &type ) const;
        /**
         * Searches for a stored LogItem with the specific details.
         * 
         * @param details The details string of the LogItem you want to find.
         * @return The LogItem with the specific properties or an empty LogItem with LogItemType = PGLG_IGNORE.
         */
        LogItem getItemByDetails( const QString &details ) const;
        /**
        * Sets the path to the log files.
        *
        * This function checks if the path is empty and if the file exists( using AbstractHandler::setFilePath() ).
        * If any of the above is false, the function prints the appropriate error message.
        * @param daemonPath The path to the pgld log file.
        * @param cmdPath The path to the pglcmd log file.
        */
        void setFilePaths( const QString &daemonPath, const QString &cmdPath );

    public slots:
        /**
         * Checks if there are any new items in the logs.
         * In that case, the appropriate signals, newLogItem() and newLogItemHits() are emitted.
         * This function also handles the FIFO queue in which the last MAX_LOG_SIZE log items are stored.
         */
        requestNewItem();
        /**
         * Empties the FIFO queue in which the LogItems are stored.
         */
        clear();

    signals:
        newLogItem( const LogItem & );
        newLogItemHits( const LogItem & );

    private:
        QList< LogItem > m_LogItemL;

};

#endif //LOGHANDLER_H
