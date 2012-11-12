
#include <QDebug>
#include <QMultiMap>
#include <QHash>
#include <QRegExp>
#include <QDBusConnection>
#include <QApplication>
#include <QDesktopWidget>
#include <QScrollBar>
//#include <Action>
//#include <ActionButton>

#include "peerguardian.h"
#include "file_transactions.h"
#include "utils.h"
#include "gui_options.h"
#include "pgl_settings.h"
#include "viewer_widget.h"
#include "error_dialog.h"

//using namespace PolkitQt1;
//using namespace PolkitQt1::Gui;

Peerguardian::Peerguardian( QWidget *parent) :
	QMainWindow( parent )
{
	setupUi( this );
    
    m_LogTreeWidget->setContextMenuPolicy ( Qt::CustomContextMenu );
    
    if ( ! PglSettings::loadSettings() )
        QMessageBox::warning(this, tr("Error"), PglSettings::lastError(), QMessageBox::Ok);
    
    m_StopLogging = false;
    mAutomaticScroll = true;
    mIgnoreScroll = false;
    
    guiOptions = new GuiOptions(this);
    initializeSettings();
    startTimers();
    g_MakeTray();
    g_MakeMenus();
    g_MakeConnections();
    updateInfo();
    updateGUI();
    
    //resize columns in log view
    QHeaderView * header = m_LogTreeWidget->header();
    header->resizeSection(0, header->sectionSize(0) / 1.5 );
    header->resizeSection(1, header->sectionSize(0) * 3 );
    header->resizeSection(3, header->sectionSize(0) / 1.4 );
    header->resizeSection(5, header->sectionSize(0) / 1.4 );
    header->resizeSection(6, header->sectionSize(6) / 2);
    
    //resize column in whitelist view
    header = m_WhitelistTreeWidget->header();
    header->resizeSection(0, header->sectionSize(0) * 2);
    header->resizeSection(2, header->sectionSize(2) / 2);
        
    a_whitelistIpTemp = new QAction(tr("Allow temporarily"), this);
    a_whitelistIpTemp->setToolTip(tr("Allows until pgld is restarted."));
    a_whitelistIpPerm = new QAction(tr("Allow permanently"), this);
    a_whitelistPortTemp = new QAction(tr("Allow temporarily"), this);
    a_whitelistPortTemp->setToolTip(tr("Allows until pgld is restarted."));
    a_whitelistPortPerm = new QAction(tr("Allow permanently"), this);
    aWhoisIp = new QAction(tr("Whois "), this);

    m_ConnectType["OUT"] = tr("Outgoing"); 
    m_ConnectType["IN"] = tr("Incoming");
    m_ConnectType["FWD"] = tr("Forward");
    
    m_ConnectIconType[tr("Outgoing")] = QIcon(LOG_LIST_OUTGOING_ICON);
    m_ConnectIconType[tr("Incoming")] = QIcon(LOG_LIST_INCOMING_ICON);
    m_ConnectIconType[tr("Forward")] = QIcon();
    

    QDBusConnection connection (QDBusConnection::systemBus());
    QString service("");
    QString name("pgld_message");
    QString path("/org/netfilter/pgl");
    QString interface("org.netfilter.pgl");
    
    bool ok = connection.connect(service, path, interface, name, qobject_cast<QObject*>(this), SLOT(addLogItem(QString)));

    if ( ! ok )
        qDebug() << "Connection to DBus failed.";
    else
        qDebug() << "Connection to DBus was successful.";

    QDesktopWidget *desktop = QApplication::desktop();
    int yy = desktop->height()/2-height()/2;
    int xx = desktop->width() /2-width()/2;
    move(xx, yy);
    
    m_LogTreeWidget->verticalScrollBar()->installEventFilter(this);
    
    connect(aWhoisIp, SIGNAL(triggered()), this, SLOT(onWhoisTriggered()));
    connect(a_whitelistIpTemp, SIGNAL(triggered()), this, SLOT(whitelistItem()));
    connect(a_whitelistIpPerm, SIGNAL(triggered()), this, SLOT(whitelistItem()));
    connect(a_whitelistPortTemp, SIGNAL(triggered()), this, SLOT(whitelistItem()));
    connect(a_whitelistPortPerm, SIGNAL(triggered()), this, SLOT(whitelistItem()));
    
    //ActionButton *bt;
    //bt = new ActionButton(kickPB, "org.qt.policykit.examples.kick", this);
    //bt->setText("Kick... (long)");

	
	//Restore to the window's previous state
    if (m_ProgramSettings->contains("window/geometry"))
        restoreGeometry( m_ProgramSettings->value("window/geometry").toByteArray() );
    
    if (m_ProgramSettings->contains("window/state"))
        restoreState( m_ProgramSettings->value( "window/state" ).toByteArray() );
    
    for (int i = 0; i < m_LogTreeWidget->columnCount(); i++ ) {
        QString settingName = "logTreeView/column_" + QString::number(i);
        if (m_ProgramSettings->contains(settingName)) {
            int value = m_ProgramSettings->value(settingName).toInt(&ok);
            if (ok)
                m_LogTreeWidget->setColumnWidth(i, value);
        }
    }

    addLogItem("OUT: 192.168.1.109:7881     147.158.36.82:18935    UDP  || Invermay Research Centre | Concurrent Technologies Corporation");
    
}

Peerguardian::~Peerguardian() {

    qWarning() << "~Peerguardian()";

	//Save column sizes
    for (int i = 0; i < m_LogTreeWidget->columnCount() ; i++ ) {
		QString settingName = "logTreeView/column_";
		settingName += QVariant( i ).toString();
		m_ProgramSettings->setValue( settingName, m_LogTreeWidget->columnWidth(i) );
	}
	/*for ( int i = 0; i < m_LogTreeWidget->columnCount() - 1; i++ ) {
		QString settingName = "log/column_";
		settingName += QVariant( i ).toString();
		m_ProgramSettings->setValue( settingName, m_LogTreeWidget->columnWidth( i ) );
	}
    **/

	//Save window settings
	m_ProgramSettings->setValue( "window/state", saveState() );
	m_ProgramSettings->setValue( "window/geometry", saveGeometry() );

    //Free memory
    if ( m_List )
        delete m_List;
    if ( m_Whitelist )
		delete m_Whitelist;
    if (guiOptions)
        delete guiOptions;
}

void Peerguardian::addLogItem(QString itemString)
{
    if ( m_StopLogging )
        return;
    
    qDebug() << itemString;
    
    if ( itemString.contains("INFO:") && itemString.contains("Blocking") )
    {
        
        QStringList parts = itemString.split("INFO:", QString::SkipEmptyParts);
        m_Info->setLoadedIps(parts[0]);
        return;
    }
    
    if ( itemString.contains("||") )
    {
        QStringList parts = itemString.split("||", QString::SkipEmptyParts);
        QStringList firstPart = parts.first().split(" ", QString::SkipEmptyParts);
        QString connectType, srcip, destip, srcport, destport;
        
        if ( firstPart.first().contains(":") )
            connectType = m_ConnectType[firstPart.first().split(":")[0]];
        else
            connectType = m_ConnectType[firstPart.first()];
            
        if ( firstPart[3] == "TCP" || firstPart[3] == "UDP" )
        {
            srcip = firstPart[1].split(":", QString::SkipEmptyParts)[0];
            srcport = firstPart[1].split(":", QString::SkipEmptyParts)[1];
            destip = firstPart[2].split(":", QString::SkipEmptyParts)[0];
            destport = firstPart[2].split(":", QString::SkipEmptyParts)[1];
        }
        else
        {
            srcip = firstPart[1];
            srcport = "";
            destip = firstPart[2];
            destport = "";
        }
            
        QStringList info;
        
        if ( m_LogTreeWidget->topLevelItemCount() > m_MaxLogSize ) {
            //mIgnoreScroll = true;
            m_LogTreeWidget->takeTopLevelItem(0);
            //mIgnoreScroll = false;
        }
        
        info << QTime::currentTime().toString("hh:mm:ss") << parts.last() << srcip << srcport << destip << destport << firstPart[3] << connectType;
        QTreeWidgetItem * item = new QTreeWidgetItem(m_LogTreeWidget, info);
        item->setIcon(7, m_ConnectIconType[connectType]);
        m_LogTreeWidget->addTopLevelItem(item);
            
        
        if (mAutomaticScroll)
            m_LogTreeWidget->scrollToBottom();
    }
    

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

    updateBlocklist();
    updateWhitelist();
    guiOptions->update();
}

void Peerguardian::startTimers()
{
	//Intiallize the medium timer for less usual procedures
	m_MediumTimer = new QTimer(this);
	m_MediumTimer->setInterval( m_ProgramSettings->value("settings/medium_timer",MEDIUM_TIMER_INTERVAL ).toInt() );
	m_MediumTimer->start();
	//Intiallize the slow timer for less usual procedures
	m_SlowTimer = new QTimer(this);
	m_SlowTimer->setInterval( m_ProgramSettings->value("settings/slow_timer", SLOW_TIMER_INTERVAL ).toInt() );
	m_SlowTimer->start();

}

void Peerguardian::g_MakeConnections()
{
	//Log tab connections
    connect( m_LogTreeWidget, SIGNAL(customContextMenuRequested ( const QPoint &)), this, SLOT(showLogRightClickMenu(const QPoint &)));
    connect( m_LogTreeWidget->verticalScrollBar(), SIGNAL(sliderMoved(int)), this, SLOT(onLogViewVerticalScrollbarMoved(int)));
    connect( m_LogTreeWidget->verticalScrollBar(), SIGNAL(actionTriggered(int)), this, SLOT(onLogViewVerticalScrollbarActionTriggered(int)));
    connect( m_LogClearButton, SIGNAL( clicked() ), m_LogTreeWidget, SLOT( clear() ) );
    connect(m_StopLoggingButton, SIGNAL(clicked()), this, SLOT(startStopLogging()));


    connect( m_addExceptionButton, SIGNAL(clicked()), this, SLOT(g_ShowAddExceptionDialog()) );
    connect( m_addBlockListButton, SIGNAL(clicked()), this, SLOT(g_ShowAddBlockListDialog()) );

    //Menu related
    connect( a_Exit, SIGNAL( triggered() ), this, SLOT( quit() ) );
    connect( a_AboutDialog, SIGNAL( triggered() ), this, SLOT( g_ShowAboutDialog() ) );
    connect(viewPglcmdLogAction, SIGNAL(triggered()), this, SLOT(onViewerWidgetRequested()));
    connect(viewPgldLogAction, SIGNAL(triggered()), this, SLOT(onViewerWidgetRequested()));

    //Control related
    if ( m_Control )
    {
        connect( m_startPglButton, SIGNAL( clicked() ), m_Control, SLOT( start() ) );
        connect( m_stopPglButton, SIGNAL( clicked() ), m_Control, SLOT( stop() ) );
        connect( m_restartPglButton, SIGNAL( clicked() ), m_Control, SLOT( restart() ) );
        connect( m_reloadPglButton, SIGNAL( clicked() ), m_Control, SLOT( reload() ) );
        connect( a_Start, SIGNAL( triggered() ), m_Control, SLOT( start() ) );
        connect( a_Stop, SIGNAL( triggered() ), m_Control, SLOT( stop() ) );
        connect( a_Restart, SIGNAL( triggered() ), m_Control, SLOT( restart() ) );
        connect( a_Reload, SIGNAL( triggered() ), m_Control, SLOT( reload() ) );
        connect( m_updatePglButton, SIGNAL( clicked() ), m_Control, SLOT( update() ) );
        connect( m_Control, SIGNAL(error(const QString&)), this, SLOT(rootError(const QString&)));
        connect( m_Control, SIGNAL(error(const CommandList&)), this, SLOT(rootError(const CommandList&)));
        
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
    {
        connect(m_Root, SIGNAL(finished()), this, SLOT(rootFinished()));
        connect(m_Root, SIGNAL(error(const QString&)), this, SLOT(rootError(const QString&)));
        connect(m_Root, SIGNAL(error(const CommandList&)), this, SLOT(rootError(const CommandList&)));
    }
    
    //connect the remove buttons
    connect(m_rmBlockListButton, SIGNAL(clicked()), this, SLOT(removeListItems()));
    connect(m_rmExceptionButton, SIGNAL(clicked()), this, SLOT(removeListItems()));
    
    //tray iconPath
    connect(m_Tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onTrayIconClicked(QSystemTrayIcon::ActivationReason)));

    connect(a_SettingsDialog, SIGNAL(triggered()), this, SLOT(openSettingsDialog()));
}

void Peerguardian::quit()
{
    int answer;
    
    if (m_ApplyButton->isEnabled())
    {
        answer = confirm(tr("Really quit?"), tr("You have <b>unapplied</b> changes, do you really want to quit?"), this);
        
        if ( answer == QMessageBox::No )
        {
            return;
        }
    }
    
    qApp->quit();
}

void Peerguardian::closeEvent ( QCloseEvent * event )
{
    this->hide();
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
    if ( m_FilesToMove.isEmpty() )
        return;
        
    QString pglcmd_conf = PglSettings::getStoredValue("CMD_CONF").split("/").last();
    QString blocklists_list = PglSettings::getStoredValue("BLOCKLISTS_LIST").split("/").last();
    QString tmp_pglcmd = QString("/tmp/%1").arg(pglcmd_conf);
    QString tmp_blocklists = QString("/tmp/%1").arg(blocklists_list);

    if ( ( m_FilesToMove.contains(tmp_pglcmd) && QFile::exists(tmp_pglcmd) ) ||  
        (m_FilesToMove.contains(tmp_blocklists) && QFile::exists(tmp_blocklists)))
    {
        m_ApplyButton->setEnabled(true);
    }
    else
    {
        m_Whitelist->updateSettings(getTreeItems(m_WhitelistTreeWidget));
        m_Whitelist->load();
        PglSettings::loadSettings();
        updateGUI();
        m_ApplyButton->setEnabled(false);
        
        if ( m_FilesToMove.contains(tmp_blocklists) ) 
            m_Control->reload();
    }
    
    m_FilesToMove.clear();
    m_UndoButton->setEnabled(m_ApplyButton->isEnabled());
}


void Peerguardian::rootError(const CommandList& failedCommands)
{
    ErrorDialog dialog(failedCommands);
    dialog.exec();

    
    /*QString errorMsg = QString("%1<br/><br/><i>%2</i><br/><br/>%3").arg(tr("The following commands failed:"))
                                                    .arg(commands.join("<br/>"))
                                                    .arg(tr("Please, check pgld's and/or pglcmd's log. You can do so through the <i>View menu</i>."));
    QMessageBox::warning( this, tr("Error (One or more command(s) failed)"), errorMsg,
	QMessageBox::Ok
    );*/
 
    m_ApplyButton->setEnabled(guiOptions->isChanged());
    m_UndoButton->setEnabled(m_ApplyButton->isEnabled());
}

void Peerguardian::rootError(const QString& errorMsg)
{
    QMessageBox::warning( this, tr("Error"), errorMsg,
	QMessageBox::Ok
    );
    
    m_ApplyButton->setEnabled(guiOptions->isChanged());
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
    QString pglcmdConfPath = PglSettings::getStoredValue("CMD_CONF");
    bool updatePglcmdConf = guiOptions->hasToUpdatePglcmdConf();
    bool updateBlocklistsList = guiOptions->hasToUpdateBlocklistList();
    QString filepath;
    
    if ( pglcmdConfPath.isEmpty() )
    {
        QString errorMsg = tr("Could not determine pglcmd.conf path! Did you install pgld and pglcmd?");
        QMessageBox::warning( this, tr("Error"), errorMsg, QMessageBox::Ok);
        qWarning() << errorMsg;
        return;
    }
    
    //only apply IPtables commands if the daemon is running
    if ( m_Info->daemonState() )
    {
        //apply new changes directly in iptables
        QStringList iptablesCommands = m_Whitelist->updateWhitelistItemsInIptables(getTreeItems(m_WhitelistTreeWidget), guiOptions);
        if ( ! iptablesCommands.isEmpty() )
            m_Root->executeCommands(iptablesCommands, false);
    }
    
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
        filesToMove["/tmp/" + getFileName(pglcmdConfPath)] = pglcmdConfPath;
        saveFileData(pglcmdConf, "/tmp/" + getFileName(pglcmdConfPath));
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
        
    m_FilesToMove = filesToMove.keys();

    if ( ! filesToMove.isEmpty() )
        m_Root->moveFiles(filesToMove, false);

    m_Whitelist->updateSettings(getTreeItems(m_WhitelistTreeWidget), guiOptions->getPositionFirstAddedWhitelistItem(), false);
    guiOptions->updateWhitelist(guiOptions->getPositionFirstAddedWhitelistItem(), false);
    m_ApplyButton->setEnabled(false); //assume changes will be applied, if not this button will be enabled afterwards
    m_UndoButton->setEnabled(false);

    m_Root->executeScript(); //execute previous gathered commands
}


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
    m_WhitelistTreeWidget->blockSignals(true);
    m_BlocklistTreeWidget->blockSignals(true);
    
    bool changed = guiOptions->isChanged();
    m_ApplyButton->setEnabled(changed);
    m_UndoButton->setEnabled(changed);
    
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
    
    m_WhitelistTreeWidget->blockSignals(false);
    m_BlocklistTreeWidget->blockSignals(false);
}

void Peerguardian::treeItemPressed(QTreeWidgetItem* item, int column)
{
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

void Peerguardian::updateBlocklist()
{
    if (! m_List )
        return;

    m_List->updateListsFromFile();

    m_BlocklistTreeWidget->blockSignals(true);
    if ( m_BlocklistTreeWidget->topLevelItemCount() > 0 )
        m_BlocklistTreeWidget->clear();

    QStringList item_info;

    //get information about the blocklists being used
    foreach(ListItem* log_item, m_List->getValidItems())
    {
        item_info << log_item->name() << log_item->location();
        QTreeWidgetItem * tree_item = new QTreeWidgetItem(m_BlocklistTreeWidget, item_info);
        tree_item->setToolTip(0, log_item->location());
        
        if ( log_item->isEnabled() )
            tree_item->setCheckState(0, Qt::Checked);
        else
            tree_item->setCheckState(0, Qt::Unchecked);

        item_info.clear();
    }

    //get local blocklists
    foreach(const QFileInfo& blocklist, m_List->getLocalBlocklists())
    {
        item_info << blocklist.fileName() << blocklist.absoluteFilePath();
        QTreeWidgetItem * tree_item = new QTreeWidgetItem(m_BlocklistTreeWidget, item_info);
        tree_item->setToolTip(0, blocklist.absoluteFilePath());
        tree_item->setCheckState(0, Qt::Checked);
        item_info.clear();
    }
    
    m_BlocklistTreeWidget->blockSignals(false);
}

void Peerguardian::updateWhitelist()
{
    QMap<QString, QStringList> items;
    QStringList values;
    QStringList info;
    
    m_WhitelistTreeWidget->blockSignals(true);
    
    if ( m_WhitelistTreeWidget->topLevelItemCount() > 0 )
        m_WhitelistTreeWidget->clear();
    
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
    
    m_WhitelistTreeWidget->setSortingEnabled(true);
    
    m_WhitelistTreeWidget->blockSignals(false);
}

void Peerguardian::initializeSettings()
{
	//Intiallize all pointers to NULL before creating the objects with g_Set==Path
	/*
	m_Settings = NULL;*/
    m_List = 0;
    m_Whitelist = 0;
    m_Info = 0;
    m_Root = 0;
    m_Control = 0;
    quitApp = false;
    mLastRunningState = false;
    
    mTrayIconDisabled = QIcon(TRAY_DISABLED_ICON);
    mTrayIconEnabled = QIcon(TRAY_ICON);
    
    setWindowIcon(mTrayIconDisabled);

    m_ProgramSettings = new QSettings(QSettings::UserScope, "pgl", "pglgui", this);
    
    QString max = m_ProgramSettings->value("maximum_log_entries").toString();
    if ( max.isEmpty() )
    {
        m_ProgramSettings->setValue("maximum_log_entries", MAX_LOG_SIZE);
        m_MaxLogSize = MAX_LOG_SIZE;
    }
    else
    {
        bool ok; 
        m_MaxLogSize = max.toInt(&ok);
        if ( ! ok )
            m_MaxLogSize = MAX_LOG_SIZE;
    }
    
    g_SetRoot();
    g_SetInfoPath();
    g_SetListPath();
    g_SetControlPath();
}

void Peerguardian::g_SetRoot( ) {
    
    if ( m_Root )
        delete m_Root;
    
    m_Root = new SuperUser(this, m_ProgramSettings->value("paths/super_user", "").toString());
}

void Peerguardian::g_SetInfoPath() {

    if (! m_Info )
        m_Info = new PeerguardianInfo(PglSettings::getStoredValue("DAEMON_LOG"), this);
}

void Peerguardian::g_SetListPath()
{
    QString filepath = PeerguardianList::getFilePath();

    if ( ! filepath.isEmpty() && m_List == NULL )
            m_List = new PeerguardianList(filepath);

    //whitelisted Ips and ports - /etc/pgl/pglcmd.conf and /etc/pgl/allow.p2p and
    //$HOME/.config/pgl/pglgui.conf for disabled items
    m_Whitelist = new PglWhitelist(m_ProgramSettings, guiOptions);

}

void Peerguardian::g_SetControlPath()
{
    QString  gSudo = m_ProgramSettings->value("paths/super_user").toString();
    m_Control = new PglCmd(this, PglSettings::getStoredValue("CMD_PATHNAME"), gSudo);
    
}

void Peerguardian::g_ShowAddDialog(int openmode) {
    AddExceptionDialog *dialog = 0;
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

    if ( dialog )
        delete dialog;
}


void Peerguardian::g_MakeTray() 
{
	m_Tray = new QSystemTrayIcon( mTrayIconDisabled );
	m_Tray->setVisible( true );
    m_Tray->setToolTip(tr("Pgld is not running"));
	g_UpdateDaemonStatus();
}


void Peerguardian::g_UpdateDaemonStatus() {

    if (! m_Info )
        return;
    
	m_Info->updateDaemonState();
	bool running = m_Info->daemonState();
    
    if ( mLastRunningState != running ) {
        if ( ! running ) {
            m_controlPglButtons->setCurrentIndex(0);
            m_Tray->setIcon(mTrayIconDisabled);
            setWindowIcon(mTrayIconDisabled);
            m_Tray->setToolTip(tr("Pgld is not running"));
        }
        else {
            m_controlPglButtons->setCurrentIndex(1);
            m_Tray->setIcon(mTrayIconEnabled);
            setWindowIcon(mTrayIconEnabled);
            m_Tray->setToolTip(tr("Pgld is up and running"));
        }
        
        mLastRunningState = running;
    }
}

void Peerguardian::g_MakeMenus() {


    //tray icon menu
	m_TrayMenu = new QMenu(this);
	m_TrayMenu->addAction( a_Start );
	m_TrayMenu->addAction( a_Stop );
	m_TrayMenu->addAction( a_Restart );
    m_TrayMenu->addAction( a_Reload );
	/*m_TrayMenu->addSeparator();
	m_TrayMenu->addAction( a_AllowHttp );
	m_TrayMenu->addAction( a_AllowHttps );
	m_TrayMenu->addAction( a_AllowFtp );*/
	m_TrayMenu->addSeparator();
	m_TrayMenu->addAction( a_Exit );
	m_Tray->setContextMenu(m_TrayMenu);
    

}

void Peerguardian::g_ShowAboutDialog() {

    QString message;
    message += QString("<b><i>PeerGuardian Linux version %1</b><br>A Graphical User Interface for PeerGuardian Linux<br><br>").arg( VERSION_NUMBER );
    message += "Copyright (C) 2007-2008 Dimitris Palyvos-Giannas<br>";
    message += "Copyright (C) 2011-2012 Carlos Pais <br><br>";
    message += "pgl is licensed under the GNU General Public License v3, or (at\
                your option) any later version. This program comes with\
                ABSOLUTELY NO WARRANTY. This is free software, and you are\
                welcome to modify and/or redistribute it.<br><br><font size=2>";
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
		lRanges = "";
    else
        lRanges.insert(0, " - ");

	//if ( dTime.isNull() )
	//	dTime = "Unknown";
        
    if ( windowTitle() != DEFAULT_WINDOW_TITLE + lRanges )
        setWindowTitle(DEFAULT_WINDOW_TITLE + lRanges);
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

void Peerguardian::startStopLogging()
{ 
    m_StopLogging = ! m_StopLogging;
    
    QPushButton *button = qobject_cast<QPushButton*> (sender()); 
    
    if ( m_StopLogging )
    {
        button->setIcon(QIcon(ENABLED_ICON));
        button->setText(tr("Start Logging"));
    }
    else
    {
        button->setIcon(QIcon(DISABLED_ICON));
        button->setText(tr("Stop Logging"));
    }
}

void Peerguardian::openSettingsDialog()
{
    SettingsDialog * dialog = new SettingsDialog(m_ProgramSettings, this);
    
    int exitCode = dialog->exec();
    
    if ( exitCode )
    {
        m_ProgramSettings->setValue("paths/super_user", dialog->file_GetRootPath());
        SuperUser::setSudoCommand(m_ProgramSettings->value("paths/super_user", SuperUser::sudoCommand()).toString());
        m_MaxLogSize = dialog->getMaxLogEntries();
        m_ProgramSettings->setValue("maximum_log_entries", QString::number(m_MaxLogSize));
    }
    
    if ( dialog )
        delete dialog; 
}


void Peerguardian::showLogRightClickMenu(const QPoint& p)
{
    QTreeWidgetItem * item = m_LogTreeWidget->itemAt(p);
    
    if ( ! item )
        return;
    
    QMenu menu(this);
    QMenu *menuIp;
    QMenu *menuPort;
    int index = 4;
    
    if ( item->text(7) == "Incoming" )
        index = 2;
    
    QVariantMap data;
    data.insert("ip", item->text(index));
    data.insert("port", item->text(5));
    data.insert("prot", item->text(6));
    data.insert("type", item->text(7));
    
    a_whitelistIpTemp->setData(data);
    a_whitelistIpPerm->setData(data);
    a_whitelistPortTemp->setData(data);
    a_whitelistPortPerm->setData(data);
    
    menuIp =  menu.addMenu("Allow IP " + item->text(index));
    menuPort = menu.addMenu("Allow Port " + item->text(5));
    
    menu.addSeparator();
    aWhoisIp->setData(item->text(index));
    aWhoisIp->setText(tr("Whois ") + item->text(index));
    menu.addAction(aWhoisIp);
    
    menuIp->addAction(a_whitelistIpTemp);
    menuIp->addAction(a_whitelistIpPerm);
    menuPort->addAction(a_whitelistPortTemp);
    menuPort->addAction(a_whitelistPortPerm);

    menu.exec(m_LogTreeWidget->mapToGlobal(p));
}

void Peerguardian::whitelistItem()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (! action)
        return;
    
    if (! m_Info->daemonState()) {
        QMessageBox::information(this, tr("Peerguardian is not running"), tr("It's not possible to whitelist while Peerguardian is not running."));
        return;
    }
    
    QVariantMap data = action->data().toMap();
    QString ip = data.value("ip").toString();
    QString port = data.value("port").toString();
    QString type = data.value("type").toString();
    QString prot = data.value("prot").toString();
    QString value = "";
    if (action == a_whitelistIpPerm || action == a_whitelistIpTemp)
        value = ip;
    else
        value = port;
        
    if ( action == a_whitelistIpTemp || action ==  a_whitelistPortTemp )
    {
        QStringList iptablesCommands = m_Whitelist->getCommands(QStringList() << value, QStringList() << type, QStringList() << prot, QList<bool>() << true);
        QString testCommand = m_Whitelist->getIptablesTestCommand(ip, type, prot);
        m_Root->executeCommands(iptablesCommands, false);
        m_Root->executeScript();
    }
    else if (  action == a_whitelistIpPerm || action == a_whitelistPortPerm )
    {
        if ( ! m_Whitelist->isInPglcmd(value, type, prot) )
        {
            QStringList info;
            info << value << type << prot;
            QTreeWidgetItem * treeItem = new QTreeWidgetItem(m_WhitelistTreeWidget, info);
            treeItem->setCheckState(0, Qt::Checked);
            treeItem->setIcon(0, QIcon(WARNING_ICON));
            treeItem->setStatusTip(0, tr("You need to click the Apply button so the changes take effect"));
            m_WhitelistTreeWidget->addTopLevelItem(treeItem);
            applyChanges();
        }
    }
}

void Peerguardian::onViewerWidgetRequested()
{
    QString path("");

    if ( viewPglcmdLogAction == sender() ) {
        path = PglSettings::getStoredValue("CMD_LOG");
    }
    else if (viewPgldLogAction == sender()) {
        path = PglSettings::getStoredValue("DAEMON_LOG");
    }
    
    ViewerWidget viewer(path);
    viewer.exec();
}

bool Peerguardian::eventFilter(QObject* obj, QEvent* event)
{
    //if (obj == m_LogTreeWidget->verticalScrollBar() && mIgnoreScroll)
    //    return true;
    
    if (obj == m_LogTreeWidget->verticalScrollBar() && event->type() == QEvent::Wheel) {
        
        if (m_LogTreeWidget->verticalScrollBar()->value() == m_LogTreeWidget->verticalScrollBar()->maximum()) 
            mAutomaticScroll = true;
        else
            mAutomaticScroll = false;
    }
    
    return false;
}

void Peerguardian::onLogViewVerticalScrollbarMoved(int value)
{
    QScrollBar *bar = static_cast<QScrollBar*>(sender());

    if (bar->maximum() == value) 
        mAutomaticScroll = true;
    else
        mAutomaticScroll = false;
}

void Peerguardian::onLogViewVerticalScrollbarActionTriggered(int action) 
{
    QScrollBar *scrollBar = static_cast<QScrollBar*>(sender());
    
    if (mAutomaticScroll && scrollBar->value() < scrollBar->maximum())
        scrollBar->setSliderPosition(scrollBar->maximum());
        
}

void Peerguardian::showCommandsOutput(const CommandList& commands) 
{
    
    QString output("");
    QString title("");
    foreach(const Command& command, commands) {
        output += command.output();
        output += "\n";
        
        if (! title.isEmpty())
            title += tr(" and ");
        title += command.command();
    }
    
    ViewerWidget viewer(output);
    viewer.setWindowTitle(title);
    viewer.exec();
} 

void Peerguardian::onWhoisTriggered() 
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (! action)
        return;
    ProcessT *process = new ProcessT(this);
    connect(process, SIGNAL(finished(const CommandList&)), this, SLOT(showCommandsOutput(const CommandList&)));
    if (action->text().contains(" "))
        process->execute("whois", QStringList() << action->data().toString());
}
    
    
