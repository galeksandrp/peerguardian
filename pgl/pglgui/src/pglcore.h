/***************************************************************************
 *   Copyright (C) 2013 by Carlos Pais <freemind@lavabit.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
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

#ifndef PGL_CORE_H
#define PGL_CORE_H

#include <QObject>
#include <QSettings>
#include <QList>
#include <QString>

#include "settings.h"
#include "add_exception_dialog.h"
#include "pgl_whitelist.h"
#include "pgl_lists.h"
#include "pgl_settings.h"
#include "option.h"


/**
*
* @short Class representing the core of the program. Contains an internal representation of the options in GUI.
*
*/

class PglCore : QObject {

    Q_OBJECT

    QList<Option*> mOptions;
    WhitelistManager* mWhitelistManager;
    BlocklistManager* mBlocklistManager;
    QString mBlocklistsFilePath;
    QString mLocalBlocklistsDir;
    QSettings* mSettings;

	public:
        PglCore(QObject* parent=0);
        virtual ~PglCore();
        Option* option(const QString&);
        void setOption(const QString&, const QVariant&, bool active=false);
        void addOption(const QString&, const QVariant&, bool active=false);
        QString getUpdateFrequencyCurrentPath();
        bool isChanged();
        BlocklistManager* blocklistManager();
        WhitelistManager* whitelistManager();
        bool hasToUpdatePglcmdConf();
        bool hasToUpdateBlocklistsFile();
        QStringList generatePglcmdConf();
        void undo();
        void load();
};	

#endif //PGL_CORE_H

