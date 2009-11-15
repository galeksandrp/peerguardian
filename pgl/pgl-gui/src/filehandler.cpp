/**************************************************************************
*   Copyright (C) 2009-2010 by Dimitris Palyvos-Giannas                   *
*   jimaras@gmail.com                                                     *
*                                                                         *
*   This is a part of pgl-gui.                                            *
*   Pgl-gui is free software; you can redistribute it and/or modify       *
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

#include <QObject>
#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>

#include <QDebug>

#include "filehandler.h"

FileHandler::FileHandler( QObject *parent ) :
    QObject( parent ) {

        this->setAutoEmit( false );
}

FileHandler::FileHandler( const QString &filename, const bool &autoE, QObject *parent ) :
    QObject( parent ) {

    this->setAutoEmit( autoE );
    this->load( filename );

}

FileHandler::~FileHandler() {


}

QString FileHandler::getFilename() const {

    return m_Filename;

}

bool FileHandler::hasData() const {

    if ( ! m_FileContents.isEmpty() ) {
        for ( QVector< QString >::const_iterator s = m_FileContents.begin(); s != m_FileContents.end(); s++ ) {
            if ( ! s->isEmpty() ) {
                return true;
            }
        }
    }

    return false;

}

QVector< QString > FileHandler::getDataV() const {

    return m_FileContents;

}

QString FileHandler::getDataS() const {

    QString data;

    for ( QVector< QString >::const_iterator s = m_FileContents.begin(); s != m_FileContents.end(); s++ ) {
        data += *s;
        data += '\n';
    }

    return data;

}

void FileHandler::setAutoEmit( const bool &autoE ) {

    m_AutoEmitSignal = autoE;

}

void FileHandler::setData( const QVector< QString > &newD ) {


    if ( newD.isEmpty() ) {
        qWarning() << Q_FUNC_INFO << "Trying to set data of FileHandler to NULL!";
    }

    m_FileContents = newD;

    this->trimLines();

}

void FileHandler::setData( const QString &newD ) {

    if ( newD.isEmpty() ) {
        qWarning() << Q_FUNC_INFO << "Trying to set data of FileHandler to NULL!";
    }
    //Clear the vector and make this string its only element
    m_FileContents.clear();
    m_FileContents.append( newD.trimmed() );

}

void FileHandler::appendData( const QVector< QString > &newD ) {


    if ( newD.isEmpty() ) {
        qWarning() << Q_FUNC_INFO << "Trying to append an empty data vector!";
    }

    m_FileContents += newD ;

    this->trimLines();
    
}

void FileHandler::appendData( const QString &newD ) {


    if ( newD.isEmpty() ) {
        qWarning() << Q_FUNC_INFO << "Trying to append an empty data string!";
    }

    m_FileContents.append( newD.trimmed() );

}

bool FileHandler::operator==( const FileHandler &second ) const {

    //If one file is empty
    if ( ! ( this->hasData() && second.hasData() ) ) {
        return false;
    }

    return ( this->m_FileContents == second.m_FileContents );

}

void FileHandler::operator=( const FileHandler &second ) {


    this->m_FileContents = second.m_FileContents;

}


bool FileHandler::load( const QString &filename ) {

    QFile file( filename );
    if ( filename.isEmpty() ) {
        qWarning() << Q_FUNC_INFO << "Empty file filename given, doing nothing.";
        return false;
    }
    else if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
        qWarning() << Q_FUNC_INFO << "Could not read from file:" << filename;
        return false;
    }
    m_Filename = filename;

    QTextStream in( &file );
    while ( !in.atEnd() ) {
        QString line = in.readLine();
        line = line.trimmed();
        m_FileContents.append(line);
    }

    //Emit signal here
    if ( m_AutoEmitSignal == true ) {
        emit fileDataV( m_FileContents );
    }

    return true;

}

bool FileHandler::reload() {

    this->discard();
    return this->load( m_Filename );

}

void FileHandler::discard() {

    m_FileContents.clear();

}

bool FileHandler::save( const QString &filename ) {

    if ( ! filename.isNull() ) {
        m_Filename = filename;
    }

    if ( m_Filename.isEmpty() ){
        qWarning() << Q_FUNC_INFO << "Cannot save to file with no name.";
        return false;
    }

    QFile file( m_Filename );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
        qWarning() << Q_FUNC_INFO << "Could not write to file:" << m_Filename;
        return false;
    }
    QTextStream out(&file);
    for ( QVector< QString >::const_iterator s = m_FileContents.begin(); s != m_FileContents.end(); s++ ) {
        out << *s << "\n";
    }

    return true;

}

void FileHandler::trimLines() {


    for ( QVector< QString >::iterator s = m_FileContents.begin(); s != m_FileContents.end(); s++ ) {
        *s = s->trimmed();
    }

}

void FileHandler::requestData() {

    emit fileDataV( m_FileContents );

}

#include "filehandler.moc" //Required for CMake, do not remove.