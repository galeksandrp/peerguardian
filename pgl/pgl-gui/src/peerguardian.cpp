
#include <QDebug>
#include <QMultiMap>
#include <QHash>
#include <QRegExp>
//#include <Action>
//#include <ActionButton>

#include "peerguardian.h"
#include "file_transactions.h"
#include "utils.h"
#include "gui_options.h"
#include "pgl_settings.h"

//using namespace PolkitQt1;
//using namespace PolkitQt1::Gui;

Peerguardian::Peerguardian( QWidget *parent ) :
	QMainWindow( parent )

{

	setupUi( this );


    PglSettings::loadSettings();
    guiOptions = new GuiOptions(this);
    inicializeVariables();
    inicializeSettings();
    startTimers();
    g_MakeTray();
    g_MakeMenus();
    g_MakeConnections();
    updateInfo();
    updateGUI();

    m_treeItemPressed = false;
    
    
    /*QStringList chains;
    chains << QString("IN") << QString("OUT") << QString("fwd");
    QStringList prots;
    prots << "tcp" << "udp";
    QStringList commands = m_Whitelist->insertPort("192.168.1.1", prots, chains, true);

    foreach(QString command, commands)
        qDebug() << command;*/
      
    
    //ActionButton *bt;
    //bt = new ActionButton(kickPB, "org.qt.policykit.examples.kick", this);
    //bt->setText("Kick... (long)");

	/*
	//Restore the window's previous state
	resize( m_ProgramSettings->value( "window/size", QSize( MAINWINDOW_WIDTH, MAINWINDOW_HEIGHT ) ).toSize() );
	restoreGeometry( m_ProgramSettings->value("window/geometry").toByteArray() );
	restoreState( m_ProgramSettings->value( "window/state" ).toByteArray() );*/

}

Peerguardian::~Peerguardian() {

	//Save column sizes
	/*for ( int i = 0; i < m_LogTreeWidget->columnCount() - 1; i++ ) {
		QString settingName = "log/column_";
		settingName += QVariant( i ).toString();
		m_ProgramSettings->setValue( settingName, m_LogTreeWidget->columnWidth( i ) );
	}

	//Save window settings
	m_ProgramSettings->setValue( "window/state", saveState() );
	m_ProgramSettings->setValue( "window/size", size() );
	m_ProgramSettings->setValue( "window/geometry", saveGeometry() );
	m_ProgramSettings->setValue( "settings/autoscroll", m_ListAutoScroll );
	m_ProgramSettings->setValue( "settings/slow_timer", m_SlowTimer->interval() );
	m_ProgramSettings->setValue( "settings/medium_timer", m_MediumTimer->interval() );
	m_ProgramSettings->setValue( "settings/fast_timer", m_FastTimer->interval() );

	//Free memory
	delete m_Settings;*/

    qWarning() << "~Peerguardian()";

    if ( m_ProgramSettings != NULL )
        delete m_ProgramSettings;
    if ( m_Root != NULL )
        delete m_Root;
    if ( m_Log != NULL )
        delete m_Log;
    if ( m_Info != NULL )
        delete m_Info;
    if ( m_List != NULL )
        delete m_List;
    if ( m_Whitelist != NULL )
		delete m_Whitelist;
    if ( m_Control != NULL )
        delete m_Control;

    delete m_FastTimer;
    delete m_MediumTimer;
    delete m_SlowTimer;
    delete guiOptions;
}

void Peerguardian::updateGUI()
{

    if ( PglSettings::getStoredValue("INIT") == "0" )
        m_StartAtBootBox->setChecked(false);
    else if ( PglSettings::getStoredValue("INIT") == "1" )
        m_StartAtBootBox->setChecked(true);


    if ( PglSettings::getStoredValue("CRON") == "0" )
        m_AutoListUpdateBox->setChecked(false);
    else
    {
        QString frequency = getUpdateFrequencyCurrentPath();

        if ( ! frequency.isEmpty() )
        {
            m_AutoListUpdateBox->setChecked(true);
            if (frequency.contains("daily/", Qt::CaseInsensitive))
                m_AutoListUpdateDailyRadio->setChecked(true);
            else if ( frequency.contains("weekly/", Qt::CaseInsensitive))
                m_AutoListUpdateWeeklyRadio->setChecked(true);
            else if ( frequency.contains("monthly/", Qt::CaseInsensitive))
                m_AutoListUpdateMonthlyRadio->setChecked(true);
        }
    }

    getLists();
    guiOptions->update();
}

void Peerguardian::startTimers()
{
    //Intiallize the fast generic timer
	m_FastTimer = new QTimer;
	m_FastTimer->setInterval( m_ProgramSettings->value( "settings/fast_timer",FAST_TIMER_INTERVAL ).toInt());
	m_FastTimer->start();
	//Intiallize the medium timer for less usual procedures
	m_MediumTimer = new QTimer;
	m_MediumTimer->setInterval( m_ProgramSettings->value("settings/medium_timer",MEDIUM_TIMER_INTERVAL ).toInt() );
	m_MediumTimer->start();
	//Intiallize the slow timer for less usual procedures
	m_SlowTimer = new QTimer;
	m_SlowTimer->setInterval( m_ProgramSettings->value("settings/slow_timer", SLOW_TIMER_INTERVAL ).toInt() );
	m_SlowTimer->start();


}

void Peerguardian::g_MakeConnections()
{
	//Log tab connections
    if ( m_Log != NULL )
    {
        connect( m_Log, SIGNAL( newItem( LogItem ) ), this, SLOT( addLogItem( LogItem ) ) );
        connect( m_FastTimer, SIGNAL( timeout() ), m_Log, SLOT( update() ) );
    }

	//connect( m_LogTreeWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( logTab_HandleLogChange() ) );
	connect( m_LogClearButton, SIGNAL( clicked() ), m_LogTreeWidget, SLOT( clear() ) );
	connect( m_LogClearButton, SIGNAL( clicked() ), m_Log, SLOT( clear() ) );

    connect( m_addExceptionButton, SIGNAL(clicked()), this, SLOT(g_ShowAddExceptionDialog()) );
    connect( m_addBlockListButton, SIGNAL(clicked()), this, SLOT(g_ShowAddBlockListDialog()) );

    connect( m_Log, SIGNAL( newItem( LogItem ) ), this, SLOT( addLogItem( LogItem ) ) );

    //Menu related
    //connect( a_Exit, SIGNAL( triggered() ), qApp, SLOT( quit() ) );
    connect( a_Exit, SIGNAL( triggered() ), this, SLOT( quit() ) );
    connect( a_AboutDialog, SIGNAL( triggered() ), this, SLOT( g_ShowAboutDialog() ) );

    //Control related
    if ( m_Control != NULL )
    {
        connect( m_startPglButton, SIGNAL( clicked() ), m_Control, SLOT( start() ) );
        connect( m_stopPglButton, SIGNAL( clicked() ), m_Control, SLOT( stop() ) );
        connect( m_restartPglButton, SIGNAL( clicked() ), m_Control, SLOT( restart() ) );
        connect( m_updatePglButton, SIGNAL( clicked() ), m_Control, SLOT( update() ) );
    }

    connect( m_MediumTimer, SIGNAL( timeout() ), this, SLOT( g_UpdateDaemonStatus() ) );
	connect( m_MediumTimer, SIGNAL( timeout() ), this, SLOT( updateInfo() ) );

    //status bar
	connect( m_Control, SIGNAL( actionMessage( QString, int ) ), m_StatusBar, SLOT( showMessage( QString, int ) ) );
	connect( m_Control, SIGNAL( finished() ), m_StatusBar, SLOT( clearMessage() ) );

	//Blocklist and Whitelist Tree Widgets
	connect(m_WhitelistTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(treeItemChanged(QTreeWidgetItem*, int)));
	connect(m_BlocklistTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(treeItemChanged(QTreeWidgetItem*, int)));
	connect(m_WhitelistTreeWidget, SIGNAL(itemPressed(QTreeWidgetItem*, int)), this, SLOT(treeItemPressed(QTreeWidgetItem*, int)));
	connect(m_BlocklistTreeWidget, SIGNAL(itemPressed(QTreeWidgetItem*, int)), this, SLOT(treeItemPressed(QTreeWidgetItem*, int)));

    /********************************Configure tab****************************/
    connect(m_UndoButton, SIGNAL(clicked()), this, SLOT(undoGuiOptions()));
    connect(m_ApplyButton, SIGNAL(clicked()), this, SLOT(applyChanges()));
    connect(m_StartAtBootBox, SIGNAL(clicked(bool)), this, SLOT(checkboxChanged(bool)));
    connect(m_AutoListUpdateBox, SIGNAL(clicked(bool)), this, SLOT(checkboxChanged(bool)));


    //connect update frequency radio buttons
    connect(m_AutoListUpdateDailyRadio, SIGNAL(clicked(bool)), this, SLOT(updateRadioButtonToggled(bool)));
    connect(m_AutoListUpdateWeeklyRadio, SIGNAL(clicked(bool)), this, SLOT(updateRadioButtonToggled(bool)));
    connect(m_AutoListUpdateMonthlyRadio, SIGNAL(clicked(bool)), this, SLOT(updateRadioButtonToggled(bool)));

    if ( m_Root )
        connect(m_Root, SIGNAL(finished()), this, SLOT(rootFinished()));

    //connect the remove buttons
    connect(m_rmBlockListButton, SIGNAL(clicked()), this, SLOT(removeListItems()));
    connect(m_rmExceptionButton, SIGNAL(clicked()), this, SLOT(removeListItems()));
    
    //tray iconPath
    connect(m_Tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onTrayIconClicked(QSystemTrayIcon::ActivationReason)));

}




void Peerguardian::quit()
{
    int answer;
    
    if (m_ApplyButton->isEnabled())
    {
	answer = confirm(tr("Apply changes?"), tr("You have unapplied changes, do you wish to apply them permanently before exiting?"), this);
	
	if ( answer == QMessageBox::Yes )
	    applyChanges();
    }
    
    quitApp = true;
    close();
}

void Peerguardian::closeEvent ( QCloseEvent * event )
{
    this->setVisible ( false );
    if ( ! quitApp )
	event->ignore();
}

void Peerguardian::onTrayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if ( reason == QSystemTrayIcon::Trigger )
	this->setVisible ( ! this->isVisible() );
}


void Peerguardian::removeListItems()
{
    QList<QTreeWidgetItem *> treeItems;
    QTreeWidget * tree;
    int i;
    bool whitelist;

    if ( sender()->objectName().contains("block", Qt::CaseInsensitive) )
    {
        tree  = m_BlocklistTreeWidget;
        whitelist = false;
    }
    else
    {
        tree  = m_WhitelistTreeWidget;
        whitelist = true;
    }

    foreach(QTreeWidgetItem *item, tree->selectedItems())
    {
        i = tree->indexOfTopLevelItem(item);
        tree->takeTopLevelItem(i);

        if ( whitelist )
        {
            guiOptions->addRemovedWhitelistItem(item);
        }
    }

    m_ApplyButton->setEnabled(guiOptions->isChanged());
    m_UndoButton->setEnabled(m_ApplyButton->isEnabled());
}

void Peerguardian::rootFinished()
{

    if ( QFile::exists("/tmp/pglcmd.conf") || QFile::exists("/tmp/blocklists.list") )
    {
        m_ApplyButton->setEnabled(true);
    }
    else
    {
        PglSettings::loadSettings();
        updateGUI();
        m_ApplyButton->setEnabled(false);
    }
    
    m_UndoButton->setEnabled(m_ApplyButton->isEnabled());
}

void Peerguardian::updateRadioButtonToggled(bool toggled)
{
    QRadioButton * item = qobject_cast<QRadioButton*>(sender());
     
    m_ApplyButton->setEnabled(guiOptions->isChanged());
    m_UndoButton->setEnabled(m_ApplyButton->isEnabled());
    
    if ( guiOptions->hasRadioButtonChanged(item) )
    {
        item->setIcon(QIcon(WARNING_ICON));
        item->setStatusTip(tr("You need to click the Apply button so the changes take effect"));
    }
    else
    {    
        item->setIcon(QIcon());
        item->setStatusTip("");
    }
    
    if ( item->objectName() != m_AutoListUpdateDailyRadio->objectName() )
    {
        m_AutoListUpdateDailyRadio->setIcon(QIcon());
        m_AutoListUpdateDailyRadio->setStatusTip("");
    }
    	
    if ( item->objectName() != m_AutoListUpdateWeeklyRadio->objectName() )
    {
        m_AutoListUpdateWeeklyRadio->setIcon(QIcon());
        m_AutoListUpdateWeeklyRadio->setStatusTip("");
    }
	
    if ( item->objectName() != m_AutoListUpdateMonthlyRadio->objectName() )
    {
        m_AutoListUpdateMonthlyRadio->setIcon(QIcon());
        m_AutoListUpdateMonthlyRadio->setStatusTip("");
    }
	
    
}


void Peerguardian::checkboxChanged(bool state)
{
    m_ApplyButton->setEnabled(guiOptions->isChanged());
    m_UndoButton->setEnabled(m_ApplyButton->isEnabled());
    
    QCheckBox *item = qobject_cast<QCheckBox*>(sender());

    
    if ( guiOptions->hasCheckboxChanged(item) )
    {
        item->setIcon(QIcon(WARNING_ICON));
        item->setStatusTip(tr("You need to click the Apply button so the changes take effect"));
    }
    else
    {
        item->setIcon(QIcon());
        item->setStatusTip("");
    }
}



QString Peerguardian::getUpdateFrequencyPath()
{
    QString path("/etc/cron.");
    QString script ("pglcmd");

    if ( m_AutoListUpdateDailyRadio->isChecked() )
        return path += "daily/" + script;
    else if ( m_AutoListUpdateWeeklyRadio->isChecked() )
        return path += "weekly/" + script;
    else if ( m_AutoListUpdateMonthlyRadio->isChecked() )
        return path += "monthly/" + script;

    return QString("");
}


QString Peerguardian::getUpdateFrequencyCurrentPath()
{
    QString path("/etc/cron.");
    QString script ("pglcmd");
    QStringList times;
    times << "daily/" << "weekly/" << "monthly/";;


    foreach(QString time, times)
        if ( QFile::exists(path + time + script ) )
            return path + time + script;

    return QString("");
}


void Peerguardian::applyChanges()
{

    QMap<QString, QString> filesToMove;
    QStringList pglcmdConf;
    bool updatePglcmdConf = guiOptions->hasToUpdatePglcmdConf();
    bool updateBlocklistsList = guiOptions->hasToUpdateBlocklistList();
    QString filepath;
    

    //apply new changes directly in iptables
    updateWhitelistItemsInIptables();
    
    m_Whitelist->updateSettings(getTreeItems(m_WhitelistTreeWidget), guiOptions->getPositionFirstAddedWhitelistItem());

    //================ update /etc/pgl/pglcmd.conf ================/
	if ( updatePglcmdConf )
    {
        //======== Whitelisted IPs are stored in pglcmd.conf too ==========/
        pglcmdConf = m_Whitelist->updatePglcmdConf(getTreeItems(m_WhitelistTreeWidget));

        //========start at boot option ( pglcmd.conf )==========/
        QString value (QString::number(int(m_StartAtBootBox->isChecked())));
        if ( hasValueInData("INIT", pglcmdConf) || PglSettings::getStoredValue("INIT") != value )
            pglcmdConf = replaceValueInData(pglcmdConf, "INIT", value);

        //====== update  frequency check box ( pglcmd.conf ) ==========/
        value = QString::number(int(m_AutoListUpdateBox->isChecked()));
        if ( hasValueInData("CRON", pglcmdConf) || PglSettings::getStoredValue("CRON") != value )
            pglcmdConf = replaceValueInData(pglcmdConf, "CRON", value);

        //add /tmp/pglcmd.conf to the filesToMove
        filesToMove["/tmp/" + getFileName(PGLCMD_CONF_PATH)] = PGLCMD_CONF_PATH;
        saveFileData(pglcmdConf, "/tmp/" + getFileName(PGLCMD_CONF_PATH));
    }

    //================ update /etc/pgl/blocklists.list ================/
    if ( updateBlocklistsList )
    {
        m_List->update(getTreeItems(m_BlocklistTreeWidget));
        filepath = "/tmp/" + m_List->getListPath().split("/").last();
        if ( QFile::exists(filepath) ) //update the blocklists.list file
            filesToMove[filepath] = m_List->getListPath();
    }

    //================ manage the local blocklists ====================/
    QHash<QString, bool> localFiles = m_List->getLocalLists();
    QString localBlocklistDir = m_List->getLocalBlocklistDir();

    foreach(QString filepath, localFiles.keys())
    {
        //if local blocklist is active
        if( localFiles[filepath] )
        {
            QString symFilepath = "/tmp/" + filepath.split("/").last();

            //if symlink to the file doesn't exist, create it
            if ( ! isPointingTo(localBlocklistDir, filepath) )
            {
                QFile::link(filepath, symFilepath);
                filesToMove[symFilepath] = localBlocklistDir;
            }
        }
        else
        {
            QString symFilepath = getPointer(localBlocklistDir, filepath);

            if ( ! symFilepath.isEmpty() ) //if symlink exists, delete it
                filesToMove[symFilepath] = "/dev/null"; //temporary hack
        }
    }


    //====== update  frequency radio buttons ==========/
    filepath = getUpdateFrequencyPath();
    if ( ! QFile::exists(filepath) )
        filesToMove[getUpdateFrequencyCurrentPath()] = filepath;

    if ( ! filesToMove.isEmpty() )
        m_Root->moveFiles(filesToMove);
    else
        rootFinished();

    
}

void Peerguardian::updateWhitelistItemsInIptables()
{
    QList<QTreeWidgetItem*> items = getTreeItems(m_WhitelistTreeWidget);
    QStringList values, connections, protocols;
    QList<bool> allows;
    int firstPos = guiOptions->getPositionFirstAddedWhitelistItem();
    QList<QTreeWidgetItem*> removedItems = guiOptions->getRemovedWhitelistItemsForIptablesRemoval();
    QList<QTreeWidgetItem*> itemsToIgnore;
    
    //append removed items
    if ( ! removedItems.isEmpty() )
        items += removedItems;
        
    //if the item has just been added, there's no point in removing it
    //from iptables, since i isn't even there yet
    if ( firstPos > 0 )
        for(int i=firstPos; i < items.size(); i++)
            if ( items[i]->checkState(0) == Qt::Unchecked  )
                itemsToIgnore << items[i];
    
    foreach ( QTreeWidgetItem * item, items )
    {
        //ignore added and unchecked items 
        if ( itemsToIgnore.contains(item) )
            continue;
        
        //if it has a warning icon and is not one of the newly added items
        if ( ! item->icon(0).isNull() )
        {
            values << item->text(0);
            connections << item->text(1);
            protocols << item->text(2);
            
            if ( item->checkState(0) == Qt::Checked )
                allows << true;
            else
                allows << false;
        }
    }
    
    QStringList commands = m_Whitelist->getCommands(values, connections, protocols, allows);

    if ( ! commands.isEmpty() )
        m_Root->executeCommands(commands);
}


/*void Peerguardian::allowIp(const QString& ip, const QStringList& chains )
{

    m_Root->execute(commands)
    
}*/

QList<QTreeWidgetItem*> Peerguardian::getTreeItems(QTreeWidget *tree, int checkState)
{
	QList<QTreeWidgetItem*> items;

	for (int i=0; i < tree->topLevelItemCount(); i++ )
		if ( tree->topLevelItem(i)->checkState(0) == checkState || checkState == -1)
			items << tree->topLevelItem(i);

	return items;
}


void Peerguardian::treeItemChanged(QTreeWidgetItem* item, int column)
{
    if ( ! m_treeItemPressed )
	return;

    m_treeItemPressed = false;

    m_ApplyButton->setEnabled(guiOptions->isChanged());
    m_UndoButton->setEnabled(m_ApplyButton->isEnabled());
    if ( guiOptions->isChanged(item) )
    {
        item->setIcon(0, QIcon(WARNING_ICON));
        item->setStatusTip(0, tr("You need to click the Apply button so the changes take effect"));
    }
    else
    {
        item->setIcon(0, QIcon());
        item->setStatusTip(0, "");
    }

}

void Peerguardian::treeItemPressed(QTreeWidgetItem* item, int column)
{

    m_treeItemPressed = true;

    if ( item->treeWidget()->objectName().contains("block", Qt::CaseInsensitive) )
    {
        m_rmExceptionButton->setEnabled(false);
        m_rmBlockListButton->setEnabled(true);
    }
    else
    {
        m_rmExceptionButton->setEnabled(true);
        m_rmBlockListButton->setEnabled(false);
    }

}

void Peerguardian::getLists()
{
    if ( m_List == NULL )
        return;


    m_List->updateListsFromFile();

    if ( m_BlocklistTreeWidget->topLevelItemCount() > 0 )
        m_BlocklistTreeWidget->clear();
    if ( m_WhitelistTreeWidget->topLevelItemCount() > 0 )
        m_WhitelistTreeWidget->clear();

    QStringList item_info;

    //get information about the blocklists being used
    foreach(ListItem* log_item, m_List->getValidItems())
    {
        item_info << log_item->name() << log_item->location();
        QTreeWidgetItem * tree_item = new QTreeWidgetItem(m_BlocklistTreeWidget, item_info);

        if ( log_item->isEnabled() )
            tree_item->setCheckState(0, Qt::Checked);
        else
            tree_item->setCheckState(0, Qt::Unchecked);

        m_BlocklistTreeWidget->addTopLevelItem( tree_item );
        item_info.clear();
    }

    //get local blocklists
    foreach(QFileInfo blocklist, m_List->getLocalBlocklists())
    {
        item_info << blocklist.fileName() << blocklist.absoluteFilePath();
        QTreeWidgetItem * tree_item = new QTreeWidgetItem(m_BlocklistTreeWidget, item_info);
        tree_item->setCheckState(0, Qt::Checked);
        item_info.clear();
    }

    QMap<QString, QStringList> items;
    QStringList values;
    QStringList info;

    //get enabled whitelisted IPs and ports
    items = m_Whitelist->getEnabledWhitelistedItems();
    foreach(QString key, items.keys())
    {
        values = items[key];
        foreach( QString value, values )
        {
            info << value << m_Whitelist->getTypeAsString(key) << m_Whitelist->getProtocol(key);
            QTreeWidgetItem * tree_item = new QTreeWidgetItem(m_WhitelistTreeWidget, info);
            tree_item->setCheckState(0, Qt::Checked );
            m_WhitelistTreeWidget->addTopLevelItem(tree_item);
            info.clear();
        }
    }

    //get disabled whitelisted IPs and ports
    items = m_Whitelist->getDisabledWhitelistedItems();
    foreach(QString key, items.keys())
    {
        values = items[key];
        foreach( QString value, values )
        {
            info << value << m_Whitelist->getTypeAsString(key) << m_Whitelist->getProtocol(key);
            QTreeWidgetItem * tree_item = new QTreeWidgetItem(m_WhitelistTreeWidget, info);
            tree_item->setCheckState(0, Qt::Unchecked );
            m_WhitelistTreeWidget->addTopLevelItem(tree_item);
            info.clear();
        }
    }
}

void Peerguardian::inicializeSettings()
{
	//Intiallize all pointers to NULL before creating the objects with g_Set==Path
	/*
	m_Settings = NULL;*/
    m_List = NULL;
    m_Whitelist = NULL;
    m_Log = NULL;
    m_Info = NULL;
    m_Root = NULL;
    m_Control = NULL;
    quitApp = false;

    m_ProgramSettings = new QSettings(QSettings::UserScope, "pgl", "pgl-gui");

    g_SetRoot();
    g_SetLogPath();
    g_SetListPath();
    g_SetControlPath();


}

void Peerguardian::inicializeVariables()
{
}

void Peerguardian::g_SetRoot( ) {

    QString filepath = SuperUser::getFilePath(m_ProgramSettings->value( "paths/super_user" ).toString());
    qDebug() << "superuser file: " << filepath;

    if ( ! filepath.isEmpty() )
    {
        if ( m_Root == NULL )
            m_Root = new SuperUser(filepath);

        if ( m_ProgramSettings->value( "paths/super_user" ).toString() != m_Root->getRootPath() )
            m_ProgramSettings->setValue( "paths/super_user", m_Root->getRootPath() );
    }
    else
        QMessageBox::warning( this, tr( "Could not locate sudo front-end" ), 
        tr( "Could not find gksu or kdesu executable. The program will not be able to perform any operation which requires super user privilleges.\n\nYou can manually change the path for the sudo front-end through the settings dialog." ), QMessageBox::Ok );

}

void Peerguardian::g_SetLogPath() {

    QString filepath = PeerguardianLog::getFilePath();

    qDebug() << filepath;

    if ( ! filepath.isEmpty() && m_Log == NULL )
    {
            m_Log = new PeerguardianLog();
			m_Log->setFilePath(filepath, true);

        if ( m_Info == NULL )
            m_Info = new PeerguardianInfo(m_Log->getLogPath());
    }
    //else
        //QMessageBox::warning( this, tr( "Log file not found!" ), tr( "Peerguardian's log file was NOT found." ), QMessageBox::Ok );

	//logTab_Init();
	//manageTab_Init();
}

void Peerguardian::g_SetListPath()
{
    QString filepath = PeerguardianList::getFilePath();

    if ( ! filepath.isEmpty() && m_List == NULL )
            m_List = new PeerguardianList(filepath);

    //whitelisted Ips and ports - /etc/pgl/pglcmd.conf and /etc/pgl/allow.p2p and
    //$HOME/.config/pgl/pgl-gui.conf for disabled items
    m_Whitelist = new PglWhitelist(m_ProgramSettings);

}

void Peerguardian::g_SetControlPath()
{
    QString filepath = PglCmd::getFilePath();

    if ( ! filepath.isEmpty() && m_Control == NULL )
    {
			m_Control = new PglCmd;
            m_Control->setFilePath(filepath, true);
    }

}

void Peerguardian::g_ShowAddDialog(int openmode) {
    AddExceptionDialog *dialog = NULL;
    bool newItems = false;

    if ( openmode == (ADD_MODE | EXCEPTION_MODE) )
    {
        dialog = new AddExceptionDialog( this, openmode, getTreeItems(m_WhitelistTreeWidget));
        dialog->exec();

        foreach(WhitelistItem whiteItem, dialog->getItems())
        {
            QStringList info; info << whiteItem.value() << whiteItem.connection() << whiteItem.protocol();
            QTreeWidgetItem * treeItem = new QTreeWidgetItem(m_WhitelistTreeWidget, info);
            treeItem->setCheckState(0, Qt::Checked);
            treeItem->setIcon(0, QIcon(WARNING_ICON));
            treeItem->setStatusTip(0, tr("You need to click the Apply button so the changes take effect"));
            m_WhitelistTreeWidget->addTopLevelItem(treeItem);
            newItems = true;

        }

	}
    else if (  openmode == (ADD_MODE | BLOCKLIST_MODE) )
    {

        dialog = new AddExceptionDialog( this, openmode, getTreeItems(m_BlocklistTreeWidget));
        dialog->exec();
        QString name;

        foreach(QString blocklist, dialog->getBlocklists())
        {
            if ( QFile::exists(blocklist) )
                name = blocklist.split("/").last();
            else
                name = blocklist;

            QStringList info; info << name <<  blocklist;
            QTreeWidgetItem * treeItem = new QTreeWidgetItem(m_BlocklistTreeWidget, info);
            treeItem->setCheckState(0, Qt::Checked);
            treeItem->setIcon(0, QIcon(WARNING_ICON));
            treeItem->setStatusTip(0, tr("You need to click the Apply button so the changes take effect"));
            m_BlocklistTreeWidget->addTopLevelItem(treeItem);
            newItems = true;

        }

        m_BlocklistTreeWidget->scrollToBottom();
    }

    if ( newItems )
    {
        m_ApplyButton->setEnabled(true);
        m_UndoButton->setEnabled(m_ApplyButton->isEnabled());
    }

	/*if ( dialog->exec() == QDialog::Accepted && dialog->isSettingChanged() ) {
		emit g_SettingChanged();
	}
	else {
		g_ReloadSettings();
		m_SettingChanged = false;
	}*/

    if ( dialog != NULL )
        delete dialog;
}


void Peerguardian::g_MakeTray() {

	m_Tray = new QSystemTrayIcon( QIcon( TRAY_ICON ) );
	m_Tray->setVisible( true );
	g_UpdateDaemonStatus();
}


void Peerguardian::g_UpdateDaemonStatus() {

    if ( m_Info == NULL )
        return;
	m_Info->updateDaemonState();
	bool running = m_Info->daemonState();
	static QString lastIcon;

	if ( ! running ) {
        m_controlPglButtons->setCurrentIndex(0);
		//Update the label and the icon in manage tab
		m_DaemonStatusLabel->setText( tr( "<FONT COLOR=\"#FF0000\">Pgld is not running</FONT>" ) );
		m_StatusIcon->setPixmap( QIcon( NOT_RUNNING_ICON ).pixmap( ICON_WIDTH, ICON_HEIGHT ) );
		//Update the tray
		if ( lastIcon != TRAY_DISABLED_ICON ) {
			m_Tray->setIcon( QIcon( TRAY_DISABLED_ICON ) );
			lastIcon = TRAY_DISABLED_ICON;
		}
		m_Tray->setToolTip( tr( "Pgld is not running" ) );

	}
	else
    {
        m_controlPglButtons->setCurrentIndex(1);
		QString message = tr( "<FONT COLOR=\"#008000\">Pgld is up and running</FONT>" );
		m_DaemonStatusLabel->setText( message );
		m_StatusIcon->setPixmap( QIcon( RUNNING_ICON ).pixmap( ICON_WIDTH, ICON_HEIGHT ) );

		if ( lastIcon != TRAY_ICON ) {
			m_Tray->setIcon( QIcon( TRAY_ICON ) );
			lastIcon = TRAY_ICON;
		}
		m_Tray->setToolTip( tr( "Pgld is up and running" ) );

	}
}

void Peerguardian::g_MakeMenus() {


	m_TrayMenu = new QMenu(this);
	m_TrayMenu->addAction( a_Start );
	m_TrayMenu->addAction( a_Stop );
	m_TrayMenu->addAction( a_Restart );
	/*m_TrayMenu->addSeparator();
	m_TrayMenu->addAction( a_AllowHttp );
	m_TrayMenu->addAction( a_AllowHttps );
	m_TrayMenu->addAction( a_AllowFtp );*/
	m_TrayMenu->addSeparator();
	m_TrayMenu->addAction( a_RestoreWindow );
	m_TrayMenu->addAction( a_Exit );
	m_Tray->setContextMenu(m_TrayMenu);

	//g_UpdateTrayActions();

	//a_ToggleAutoScrolling->setChecked( m_ListAutoScroll );

}

void Peerguardian::g_ShowAboutDialog() {

    QString message;
    message += QString("<b><i>PeerGuardian Linux version %1</b><br>A Graphical User Interface for PeerGuardian Linux<br><br>").arg( VERSION_NUMBER );
    message += "Copyright (C) 2007-2008 Dimitris Palyvos-Giannas<br>";
    message += "Copyright (C) 2011 Carlos Pais <br><br>";
    message += "This program is licenced under the GNU General Public Licence v3<br><br><font size=2>";
    message +="Using modified version of the crystal icon theme:<br>http://www.everaldo.com/<br>http://www.yellowicon.com/<br><br>";
    message += "Credits go to Morpheus, jre, TheBlackSun, Pepsi_One and siofwolves from phoenixlabs.org for their help and suggestions. <br>";
    message += "I would also like to thank Art_Fowl from e-pcmag.gr for providing valuable help with Qt4 and for helping me with the project's development. <br>";
    message += "Special credit goes to Evangelos Foutras for developing the old project's website, <a href=http://mobloquer.sourceforge.net>mobloquer.foutrelis.com</a></font></i>";

	QMessageBox::about( this, tr( "About PeerGuardian Linux GUI" ), tr( message.toUtf8() ));

}

void Peerguardian::updateInfo() {


    if ( m_Info == NULL )
        return;

	QString lRanges = m_Info->loadedRanges();
	QString dTime = m_Info->lastUpdateTime();

	if ( lRanges.isEmpty() )
		lRanges = "N/A";

	if ( m_Info->daemonState() == false )
		lRanges = "0";

	if ( dTime.isNull() )
		dTime = "Unknown";


	m_LoadedRangesLabel->setText( tr( "Blocked IP ranges: %1" ).arg( lRanges ) );
	m_LastUpdateLabel->setText( tr( "Last blocklist update: %1" ).arg( dTime ) );

}


void Peerguardian::addLogItem( LogItem item ) {

	QString iconPath;
	QString itemTypeStr;

	if ( item.type == IGNORE || item.type == ERROR ) {
		return;
	}
	else if ( item.type == BLOCK_IN ) {
		iconPath = LOG_LIST_INCOMING_ICON;
		itemTypeStr = tr(IN_STR);
	}
	else if ( item.type == BLOCK_OUT ) {
		iconPath = LOG_LIST_OUTOING_ICON;
		itemTypeStr = tr( OUT_STR );
	}
	else if ( item.type == BLOCK_FWD ) {
		iconPath = LOG_LIST_FORWARD_ICON;
		itemTypeStr = tr( FWD_STR );
	}


	QStringList item_info;
	item_info << item.blockTime();
	item_info << item.name();
	item_info << item.getIpSource();
	item_info << item.getIpDest();
	item_info << "TCP (change)";
	item_info << "Blocked";

	QTreeWidgetItem *newItem = new QTreeWidgetItem(m_LogTreeWidget, item_info);

	/*newItem->setText( LOG_TIME_COLUMN, item.blockTime() );
	newItem->setToolTip( LOG_TIME_COLUMN, item.blockTime() );
	newItem->setText( LOG_NAME_COLUMN, item.name() );
	newItem->setToolTip( LOG_NAME_COLUMN, item.name() );
	newItem->setText( LOG_IP_COLUMN, item.IP() );
	newItem->setToolTip( LOG_IP_COLUMN, item.IP() );
	newItem->setText( LOG_TYPE_COLUMN, itemTypeStr );
	newItem->setToolTip( LOG_TYPE_COLUMN, itemTypeStr );
	newItem->setIcon( LOG_TYPE_COLUMN, QIcon( iconPath ) );*/
	m_LogTreeWidget->addTopLevelItem( newItem );

	//Don't let the list become too big
	if ( m_LogTreeWidget->topLevelItemCount() > MAX_LOG_SIZE ) {
		m_LogTreeWidget->clearSelection();
		QTreeWidgetItem *toRem = m_LogTreeWidget->takeTopLevelItem( 0 );
		delete toRem;
		toRem = NULL;
	}

	//if ( m_ListAutoScroll == true )
	m_LogTreeWidget->scrollToBottom();

	//++m_BlockedConnections;

}


void Peerguardian::undoGuiOptions() 
{ 
    int answer = 0;
    
    answer = confirm(tr("Really Undo?"), tr("Are you sure you want to ignore the unsaved changes?"), this);
    
    if ( answer == QMessageBox::Yes)
    {
        guiOptions->undo(); 
        m_UndoButton->setEnabled(false);
        m_ApplyButton->setEnabled(false);
    }
}

