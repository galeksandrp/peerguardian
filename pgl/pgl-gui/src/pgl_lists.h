/***************************************************************************
 *   Copyright (C) 2007-2008 by Dimitris Palyvos-Giannas   *
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

#ifndef PEERGUARDIAN_LISTS_H
#define PEERGUARDIAN_LISTS_H

#include <QVector>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QTreeWidgetItem>
#include <QFileInfo>
#include <QHash>

#include "file_transactions.h"



enum itemMode { ENABLED_ITEM, DISABLED_ITEM, COMMENT_ITEM };

/**
*
* @short Class representing both blocklist entries and comments in the blockcontrol blocklists file.
*
*/

class ListItem {

	public:
		/**
		 * Constructor. Creates a ListItem and analyzes the raw line from the configuration file.
		 * @param itemRawLine The line from the blockcontrol blocklists file.
		 */
		ListItem( const QString &itemRawLine );
		/**
		 * Constructor, Creates an empty ListItem.
		 */
		ListItem() { mode = ENABLED_ITEM; }
		/**
		 * Destructor.
		 */
		~ListItem() { }

		/**
		 * The mode of the ListItem.
		 * ENABLED_ITEM: Enabled blocklist entry.
		 * DISABLED_ITEM: Disabled blocklist entry, line starts with #.
		 * COMMENT_ITEM: Not a blocklist. Line starts with # but it's not valid.
		 */
		itemMode mode;
		/**
		 * @return The name of the list without the extension.
		 * For example "www.bluetack.co.uk/config/nipfilter.dat.gz" will return "nipfilter"
		 */
		inline QString name() const { return m_Name; }
		/**
		 * @return The blocklist's location.
		 * Example: "www.bluetack.co.uk/config/nipfilter.dat.gz"
		 */
		inline QString location() const { return m_Location; }

		/**
		 * Check if this ListItem matches another.
		 * This function compares only the locations of the ListItems.
		 * @param other The second ListItem.
		 * @return True if the ListItems are the same, otherwise false.
		 */
		bool operator==( const ListItem &other );

        bool isEnabled();
        bool isDisabled();

		static bool isValidBlockList(const QString&);
		QString getListName(const QString& );

	private:
		QString m_Name;
		QString m_Location;

};

/**
*
* @short Class representing the data in the blockcontrol blocklists file.
*
*/


class PeerguardianList {

	public:
		/**
		 * Constructor. Creates a PeerguardianList object and loads the data from the blockcontrol blocklists file.
		 * @param path The path to the blockcontrol blocklists file.
		 */
		PeerguardianList( const QString &path );
		/**
		 * Constructor. Creates an emtpy PeerguardianList object with no data loaded.
		 */
		PeerguardianList() { };
		/**
		 * Destructor.
		 */
		~PeerguardianList() { }
		/**
		 * Set the file path to the blockcontrol blocklists file.
		 * If the path is invalid and no path is already set, PGL_LIST_PATH is used.
		 * @param path The path to the blockcontrol blocklists file.
		 */
		void setFilePath( const QString &path, bool verified=false );
        QString getListPath();
		/**
		 * Insert a new blocklist item into the blocklists file.
		 * @param newItem The new ListItem to be added.
		 */
		void addItem( const ListItem &newItem );
		/**
		 * Insert a new blocklist item into the blocklists file.
		 * @param line The raw line from the blocklists file.
		 */
		void addItem( const QString &line );
		/**
		 * Change the mode of a ListItem which already exists in the blockcontrol blocklists file.
		 * @param item The ListItem the mode of which is to be changed.
		 * @param newMode The new mode of the ListItem.
		 */
		void setMode( const ListItem &item, const itemMode &newMode );
		/**
		 * Change the mode of a ListItem which already exists in the blockcontrol blocklists file.
		 * The ListItem will be found according to its location.
		 * @param location The ListItem's location.
		 * @param newMode The ListItem's new mode.
		 */
		void setModeByLocation( const QString &location, const itemMode &newMode );
		/**
		 * Change the mode of a ListItem which already exists in the blockcontrol blocklists file.
		 * The ListItem will be found according to its name.
		 * @param location The ListItem's name.
		 * @param newMode The ListItem's new mode.
		 */
		void setModeByName( const QString &name, const itemMode &newMode );
		/**
		 * Remove a ListItem completely from the blocklists file.
		 * WARNING: Searches by name are NOT safe and should be avoided.
		 * @param item The item to be found and removed.
		 */
		void removeItem( const ListItem &item );
		/**
		 * Remove a ListItem completely from the blocklists file.
		 * @param location The item's location.
		 */
		void removeItemByLocation( const QString &location );
		/**
		 * Remove a ListItem completely from the blocklists file.
		 * @param name The item's name.
		 */
		void removeItemByName( const QString &name );
		/**
		 * Get a ListItem which already exists in the blocklists file by its name.
		 * If no item is found a NULL pointer is returned.
		 * WARNING: Searches by name are NOT safe and should be avoided.
		 * @param name The name of the ListItem.
		 * @return A pointer to the ListItem requested, or a NULL pointer if the item is not found.
		 */
		ListItem *getItemByName( const QString &name );
		/**
		 * Get a ListItem which already exists in the blocklists file by its location.
		 * If no item is found a NULL pointer is returned.
		 * WARNING: Searches by name are NOT safe and should be avoided.
		 * @param location The location of the ListItem.
		 * @return A pointer to the ListItem requested, or a NULL pointer if the item is not found.
		 */
		ListItem *getItemByLocation( const QString &location );
		/**
		 * Get the items with the speicified mode.
		 * @param mode The mode of the ListItems.
		 * @return A vector of pointers containing the items with the requested mode.
		 */
		QVector< ListItem * > getItems( const itemMode &mode );
 		/**
 		 * Get all the ListItems which exist in the blockcontrol blocklists file.
 		 * Items with mode COMMENT_ITEM are not returned by this function.
 		 * @return A vector of pointers containing the ListItems requested.
 		 */
 		QVector< ListItem * > getValidItems();
        QVector< ListItem * > getEnabledItems();
        QVector< ListItem * > getDisabledItems();
		/**
		 * Export the PeerguardianList object to a file as a blockcontrol blocklists file.
		 * @param filename The path to the file where the object is to be exported.
		 * @return True if the file was exported, otherwise false.
		 */
		bool exportToFile( const QString &filename );

        static QString getFilePath();
        static QString getFilePath(const QString &path);
        void update(QList<QTreeWidgetItem*>);
        QHash<QString, bool> getLocalLists(){ return m_localLists; }
        QString getLocalBlocklistDir(){ return m_localBlocklistDir; }
        QFileInfoList getLocalBlocklists();
        void updateListsFromFile();

	private:
		int indexOfName( const QString &name );
		QVector< ListItem > m_ListsFile;
		QString m_FileName;
        QHash<QString, bool> m_localLists;
        QString m_localBlocklistDir;

};

#endif
