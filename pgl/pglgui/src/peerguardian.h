
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


#ifndef PEERGUARDIAN_H
#define PEERGUARDIAN_H

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QCloseEvent>
#include <QEvent>
#include <QVariant>
#include <QByteArray>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QPushButton>
#include <QIcon>
#include <QMenu>
#include <QCursor>
#include <QMessageBox>
#include <QFile>
#include <QFont>
#include <QTreeWidgetItem>
#include <QTimer>
#include <QDate>
#include <QTime>
#include <QFileDialog>
#include <QStringList>
#include <QList>
#include <QTreeWidgetItem>

#include "ui_main_window.h"

#include "file_transactions.h"
#include "pglcore.h"
#include "pgl_settings.h"
#include "pgl_lists.h"
#include "peerguardian_info.h"
#include "pglcmd.h"
#include "super_user.h"

#include "settings.h"
#include "add_exception_dialog.h"
#include "pgl_whitelist.h"


#define VERSION_NUMBER VERSION
#define DEFAULT_WINDOW_TITLE "PeerGuardian Linux"

//Time related defines
#define MEDIUM_TIMER_INTERVAL 2000
#define SLOW_TIMER_INTERVAL 5000
#define INFOMSG_DELAY 5000


//Icon related defines
#define LOG_LIST_INFO_ICON ":/images/info.png"
#define LOG_LIST_OUTGOING_ICON ":/images/outgoing.png"
#define LOG_LIST_INCOMING_ICON ":/images/incoming.png"
#define LOG_LIST_FORWARD_ICON ":/images/forward.png"
#define LOG_LIST_ERROR_ICON ":/images/error.png"
#define TRAY_ICON ":/images/tray.png"
#define TRAY_DISABLED_ICON ":/images/tray_disabled.png"
#define ENABLED_ICON ":/images/ok.png"
#define DISABLED_ICON ":/images/cancel.png"
#define RUNNING_ICON ":/images/ok.png"
#define NOT_RUNNING_ICON ":/images/cancel.png"
#define WARNING_ICON ":/images/warning.png"
#define ICON_WIDTH 24
#define ICON_HEIGHT 24

#define UNCHECKED 0
#define CHECKED 2

#define MAX_LOG_SIZE 512


class GuiOptions;
class QTreeWidgetItem;
class QApplication;

/**
*
* @short Class representing the main window of the program. Handles everything that has to do with the GUI.
*
*/

class Peerguardian : public QMainWindow {

	Q_OBJECT

    QSettings *m_ProgramSettings;
    QString m_Loaded_RootFile;
    SuperUser *m_Root;
    PeerguardianInfo *m_Info;
    PglCmd *m_Control;
    QSystemTrayIcon *m_Tray;
    QIcon mTrayIconEnabled;
    QIcon mTrayIconDisabled;
    QMenu * m_TrayMenu;
    QMenu * m_LogMenu;
    //Timers
    QTimer *m_MediumTimer;
    QTimer *m_SlowTimer;
    bool quitApp;
    bool m_WhitelistItemPressed;
    bool m_BlocklistItemPressed;
    QStringList m_FilesToMove;
    bool m_StopLogging;
    QHash<QString, QString> m_ConnectType;
    QHash<QString, QIcon> m_ConnectIconType;
    int m_MaxLogSize;
    //whitelist QActions
    QAction *a_whitelistIpPerm;
    QAction *a_whitelistIpTemp;
    QAction *a_whitelistPortPerm;
    QAction *a_whitelistPortTemp;
    bool mLastRunningState;
    CommandList mFailedCommands;
    volatile bool mAutomaticScroll;
    volatile bool mIgnoreScroll;
    QAction *aWhoisIp;
    PglCore *mPglCore;
    Ui::MainWindow mUi;
    bool mReloadPgl;

	public:
		/**
		 * Constructor.
		 * This function calls several other functions to do the following:
		 * a)Setup the GUI.
		 * b)Create objects of the main classes.
		 * c)Create the connections between the objects.
		 * @param parent The QWidget parent of the object.
		 */
		Peerguardian( QWidget *parent = 0);
		/**
		 * Destructor. Takes the appropriate actions when Peerguardian is destroyed.
		 * Deletes object pointers and saves settings.
		 */
		virtual ~Peerguardian();

        void g_MakeConnections();
        void initCore();
		void g_SetRoot();
		void g_SetInfoPath();
        void g_SetControlPath();
        void g_MakeTray();
        void g_MakeMenus();
        void g_ShowAddDialog(int);
        void startTimers();
        void updateGUI();
        QList<QTreeWidgetItem*> getTreeItems(QTreeWidget *tree, int checkState=-1);
        QRadioButton * getAutoListUpdateDailyRadio() {return mUi.updateDailyRadio;}
        QRadioButton * getAutoListUpdateWeeklyRadio() {return mUi.updateWeeklyRadio;}
        QRadioButton * getAutoListUpdateMonthlyRadio() {return mUi.updateMonthlyRadio;}
        QCheckBox * getAutoListUpdateBox() { return mUi.updateAutomaticallyCheckBox; }
        QCheckBox * getStartAtBootBox() { return mUi.startAtBootCheckBox; }
        QTreeWidget * getBlocklistTreeWidget() { return mUi.blocklistTreeWidget; }
        QTreeWidget * getWhitelistTreeWidget() { return mUi.whitelistTreeWidget; }
        QString getUpdateFrequencyPath();
        QString getUpdateFrequencyCurrentPath();
        void updateWhitelistWidget();
        void updateBlocklistWidget();
        int getMaxLogSize(){ return m_MaxLogSize; }
        virtual bool eventFilter(QObject*, QEvent*);

    public slots:
        void showAddExceptionDialog();
        void showAddBlocklistDialog();
        void g_ShowAboutDialog();
        void updateInfo();
        void g_UpdateDaemonStatus();
        void treeItemChanged(QTreeWidgetItem*, int);
        void blocklistItemChanged(QTreeWidgetItem*, int);
        void whitelistItemChanged(QTreeWidgetItem*, int);
        void treeItemPressed(QTreeWidgetItem* item, int column);
        void applyChanges();
        void updateRadioButtonToggled(bool);
        void rootFinished(const CommandList&);
        void rootError(const QString&);
        void controlFinished(const CommandList&);
        void removeListItems();
        void onTrayIconClicked(QSystemTrayIcon::ActivationReason);
        void checkboxChanged(bool);
        void undoAll();
        void addLogItem(QString);
        void startStopLogging();
        void openSettingsDialog();
        void showLogRightClickMenu(const QPoint&);
        void whitelistItem();


    protected:
        void closeEvent ( QCloseEvent * event );
        void updateBlocklists();

    protected slots:
        void quit();
        void onViewerWidgetRequested();
    
    private slots:
        void onLogViewVerticalScrollbarMoved(int);
        void onLogViewVerticalScrollbarActionTriggered(int);
        void onWhoisTriggered();
        void showCommandsOutput(const CommandList&);

private:
        void setApplyButtonEnabled(bool enable);
        void setButtonChanged(QAbstractButton*, bool);
        void setTreeWidgetItemChanged(QTreeWidgetItem*, bool, bool blockSignals=true);
        void restoreSettings();
        void saveSettings();
};	

#endif //PEERGUARDIAN_H

