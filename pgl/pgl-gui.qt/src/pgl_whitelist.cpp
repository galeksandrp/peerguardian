
#include "pgl_whitelist.h"
#include "utils.h"
#include "pgl_settings.h"
#include "file_transactions.h"
#include <QDebug>


WhitelistItem::WhitelistItem(QString& value, int connection, bool enabled, QString& group)
{
    m_Value = value;
    
    m_Type = PORT;
    /*if ( isValidIp(value) )
        m_Type = IP;
    else if ( getPort(value)  != -1 )
        m_Type = PORT;
    else
        m_Type = INVALID;*/
    
    m_Transport = TCP;
    m_Connection = connection;
    m_Group = group;
    m_Enabled = enabled;
}

QString WhitelistItem::getConnectionAsString()
{
    switch(m_Connection)
    {
        case TYPE_INCOMING: return QString("Incoming");
        case TYPE_OUTGOING: return QString("Outgoing");
        case TYPE_FORWARD: return QString("Forward"); 
        default: return QString("");
    }
}

PglWhitelist::PglWhitelist(QSettings* settings)
{
    m_WhitelistFile = getVariable(PGLCMD_DEFAULTS_PATH, "CONFDIR") + QString("/");
    m_WhitelistFile += getVariable(PGLCMD_DEFAULTS_PATH, "CMD_NAME") + QString(".conf");
    
    m_Settings = settings;
    
    if ( m_WhitelistFile.isEmpty() )
        return;
        
    m_Group[WHITE_IP_IN] = TYPE_INCOMING;
    m_Group[WHITE_IP_OUT] = TYPE_OUTGOING;
    m_Group[WHITE_IP_FWD] = TYPE_FORWARD;
    m_Group[WHITE_TCP_IN] = TYPE_INCOMING;
    m_Group[WHITE_UDP_IN] = TYPE_INCOMING;
    m_Group[WHITE_TCP_OUT] = TYPE_OUTGOING;
    m_Group[WHITE_UDP_OUT] = TYPE_OUTGOING;
    m_Group[WHITE_TCP_FWD] = TYPE_FORWARD;
    m_Group[WHITE_UDP_FWD] = TYPE_FORWARD;
    
    QStringList fileData = getFileData(m_WhitelistFile);
    
    foreach(QString line, fileData)
    {
        if( line.startsWith("#") )
            continue;
        
        foreach(QString key, m_Group.keys())
        {
            if ( line.contains(key) )
            {
                QString variable = getVariable(line);
                m_WhitelistEnabled[key] = variable.split(" ", QString::SkipEmptyParts);
                
                break;
            }
        }
    }
    
    //Since disabled whitelisted items (IPs or Ports) can't be easily stored
    //in /etc/plg/pglcmd.conf, the best option is to store them on the GUI settings file
    QString disabled_items;
    
    foreach ( QString key, m_Group.keys() )
    {
        disabled_items = settings->value(QString("whitelist/%1").arg(key)).toString();
        m_WhitelistDisabled[key] = disabled_items.split(" ", QString::SkipEmptyParts);
    }
}

QString PglWhitelist::getProtocol(QString& key)
{
    return key.split("_")[1];
}

QString PglWhitelist::getTypeAsString(QString& key)
{
    switch(m_Group[key])
    {
        case TYPE_INCOMING: return QString("Incoming");
        case TYPE_OUTGOING: return QString("Outgoing");
        case TYPE_FORWARD: return QString("Forward"); 
        default: return QString("");
    }
    
}

QString PglWhitelist::getGroup(QStringList& info)
{
    /*info should contain the value (a port or an ip address) and the connection type (in, out or fwd)*/
    if ( info.size() != 2 )
        return "";
        
    QMap<QString, QString> connection;
    connection["Incoming"] = "IN";
    connection["Outgoing"] = "OUT";
    connection["Forward"] = "FWD";
    
    QString value = info[0];
    QString conn_type = info[1];
    QString protocol = info[2];
    QString key = "WHITE_";
    
    if ( isPort(value) )
        key += protocol + "_";
    else
        key += "IP_";
    
    key += connection[conn_type];
    
    return key;
    
}

void PglWhitelist::updateWhitelistFile()
{

    QStringList fileData = getFileData(m_WhitelistFile);
    QStringList newData;
    
    foreach(QString line, fileData)
    {
        if ( line.startsWith("#") || line.isEmpty() )
        {
            newData << line;
            continue;
        }
        
        foreach(QString key, m_WhitelistEnabled.keys())
            if ( line.contains( key ) )
                newData << key + "=\"" + m_WhitelistEnabled[key].join(" ") + "\""; 
    }
    
    QString filepath = "/tmp/" + m_WhitelistFile.split("/").last();
    saveFileData(newData, filepath);
}


void PglWhitelist::updateSettings()
{
    foreach(QString key, m_WhitelistDisabled.keys() )
        m_Settings->setValue(QString("whitelist/%1").arg(key), m_WhitelistDisabled[key].join(" "));
}

void PglWhitelist::update(QList<QTreeWidgetItem*> treeItems)
{
    if ( m_WhitelistFile.isEmpty() )    
        return;
    
    QStringList fileData = getFileData(m_WhitelistFile);
    QStringList info;
    QString group;

    //Update the Enabled Whitelist
    m_WhitelistEnabled.clear();
    foreach(QTreeWidgetItem* treeItem, treeItems)
    {
        if ( treeItem->checkState(0) == Qt::Unchecked )
            continue;
        
        info << treeItem->text(0) << treeItem->text(1) << treeItem->text(2);
        
        group = getGroup(info);
        if ( m_Group.contains(group) )
            m_WhitelistEnabled[group] << treeItem->text(0);
        
        info.clear();
    }
    
    updateWhitelistFile();
    
    
    //Update the Disabled Whitelist
    m_WhitelistDisabled.clear();
    foreach(QTreeWidgetItem* treeItem, treeItems)
    {
        if ( treeItem->checkState(0) == Qt::Checked )
            continue;
        
        info << treeItem->text(0) << treeItem->text(1) << treeItem->text(2);
        
        group = getGroup(info);
        if ( m_Group.contains(group) )
            m_WhitelistDisabled[group] << treeItem->text(0);
        
        info.clear();
    }
    
    updateSettings();
}

