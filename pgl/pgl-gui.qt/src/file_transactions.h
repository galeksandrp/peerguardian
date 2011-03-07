/***************************************************************************
 *   Copyright (C) 2007 by Dimitris Palyvos-Giannas   *
 *   jimaras@gmail.com   *
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

#ifndef FILE_TRANSACTIONS_H
#define FILE_TRANSACTIONS_H

#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QString>
#include <QtDebug>

/**
*
* @short Some simple functions to handle text file data
*
*/



/**
 * Get the raw file data from a text file.
 * @param path The path to the file you want to get the data from.
 * @return A vector containing the - trimmed - file lines as QStrings.
 */
QVector< QString > getFileData( const QString &path );
/**
 * Save the data from a vector containing QSrings to a text file.
 * @param data The vector containing the lines of text which will be saved.
 * @param path The path to the file where the data will be written.
 * @return True if the file was saved successfully, otherwise false.
 */
bool saveFileData( const QVector< QString > &data, const QString &path );
/**
 * Compare the data from two different text files.
 * @param pathA The path to the first file.
 * @param pathB The path to the second file.
 * @return True if the file data are the same, otherwise false
 */
bool compareFileData( const QString &pathA, const QString &pathB );


#endif
