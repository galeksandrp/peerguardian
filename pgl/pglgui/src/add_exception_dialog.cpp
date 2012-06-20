
#include <QEvent>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QCompleter>
#include <QFileSystemModel>

#include "add_exception_dialog.h"
#include "utils.h"
#include "file_transactions.h"
#include "pgl_lists.h"

class WhitelistItem;

AddExceptionDialog::AddExceptionDialog(QWidget *p, int mode, QList<QTreeWidgetItem*> treeItems) :
	QDialog( p )
{
	setupUi( this );

    m_validExtensions << ".p2p" << ".zip" << ".7z" << ".gzip" << ".dat";
    QString help;

    groupBox_2->hide();

    if ( mode == (ADD_MODE | EXCEPTION_MODE) )
    {

        setPortsFromFile();
        //completer for the ports' names
        QCompleter * completer = new QCompleter(ports.keys(), m_addEdit);
        m_addEdit->setCompleter(completer);
        m_browseButton->hide();
        QString value;

        help  = QObject::tr("Valid Inputs:") + "\n";
        help += QObject::tr("You can enter an IP Address with or without mask,");
        help += QObject::tr("a range of ports (eg. 1:50) or a single port number/name (e.g. 80, http, ftp, etc).") + "\n\n";
        help += QObject::tr("You can enter multiple items separated by spaces.") + "\n\n";
        help += QObject::tr("Please note that allowing an outgoing port is a certain security risk:");
        help += QObject::tr("a malicious host may listen on just this port. It is safer to allow traffic to certain IPs instead.");

        foreach(QTreeWidgetItem *treeItem, treeItems)
        {
            value = treeItem->text(0);
            WhitelistItem item = WhitelistItem(value, treeItem->text(1), treeItem->text(2));

            foreach(Port port, m_ports)
                if ( port == item )
                    item.addAliases(port.values());

            m_Items.push_back(item);
        }

        connect(m_addEdit, SIGNAL(returnPressed()), this, SLOT(addEntry()));
        connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(addEntry()));
        
        
        resize(width(), minimumSize().height());
    }
    else if ( mode == (ADD_MODE | BLOCKLIST_MODE) )
    {

            QFileSystemModel * model = new QFileSystemModel(m_addEdit);
            model->setRootPath(QDir::homePath());
            //completer for the ports' names
            QCompleter * completer = new QCompleter(model, m_addEdit);
            m_addEdit->setCompleter(completer);

            help = QObject::tr("Valid Inputs: You can enter a local path or an address to a valid blocklist.") + "\n";
            help += QObject::tr("You can enter multiple items separated by spaces!") + "\n\n";
            help += QObject::tr("If you are adding a local blocklist, please remember that the file you're adding won't be copied! ");
            help += QObject::tr("In other words, if you delete the file you're adding, it will be no longer available or used in pgl after a restart or reload") + "\n\n";
            help += QObject::tr("If you are adding an URL please prepend \"http://\", \"https://\" or \"ftp://\" in case your address doesn't have it!");

            setWindowTitle(QObject::tr("Add Blocklists"));
            groupBox->setTitle(QObject::tr("Add one or more Blocklists"));
            m_ConnectionGroup->hide();
            m_ProtocolGroup->hide();
            m_ipRadio->hide();
            m_portRadio->hide();
            connect(m_browseButton, SIGNAL(clicked()), this, SLOT(selectLocalBlocklist()));


            m_notValidTreeWidget->header()->hideSection(1);
            m_notValidTreeWidget->header()->hideSection(2);


        connect(m_addEdit, SIGNAL(returnPressed()), this, SLOT(addBlocklist()));
        connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(addBlocklist()));
        
       setMinimumSize(0, 200);
       resize(width(), height()/1.8);
    }
    
    m_helpLabel->setText(QObject::tr(help.toUtf8 ()));

    /*else if ( mode == (EDIT_MODE | EXCEPTION_MODE) )
    {
            m_exceptionTreeWidget->hide();
            m_helpLabel->hide();
            resize(width(), 300);
    }
    else if ( mode == (EDIT_MODE | BLOCKLIST_MODE))
    {
    }*/

    this->mode = mode;


}


//************************** Whitelist dialog ***************************//

AddExceptionDialog::~AddExceptionDialog()
{

    qDebug() << "~AddExceptionDialog()";
}

Port AddExceptionDialog::getPortFromLine(QString line)
{
    QStringList elements;
    int port_num;
    QString protocol;
    Port port;

    line = line.simplified();

    if ( line.isEmpty() || line.startsWith("#") )
        return Port();

    elements = line.split(" ");

    port_num = elements[1].split("/")[0].toInt();
    protocol = elements[1].split("/")[1];

    port = Port(elements[0], protocol, port_num);

    if ( elements.size() >= 3 && ( ! elements[2].startsWith("#")) )
        port.addAlias(elements[2]);

    return port;
}

void AddExceptionDialog::setPortsFromFile()
{
    QStringList fileData = getFileData("/etc/services");
    QStringList elements;
    int port_num;
    QString protocol;
    Port port1, port2;

    //old way for backwards compatibility
    foreach(QString line, fileData)
    {
        line = line.simplified();

        if ( line.isEmpty() || line.startsWith("#") )
            continue;

        elements = line.split(" ");

        port_num = elements[1].split("/")[0].toInt();
        protocol = elements[1].split("/")[1];

        ports[elements[0]] = port_num;

        //check for alternative names
        if ( elements.size() >= 3 && ( ! elements[2].startsWith("#")) )
            ports[elements[2]] = port_num;
    }

    for ( int i=0; i < fileData.size(); i++ )
    {

        port1 = getPortFromLine(fileData[i]);

        if ( i < fileData.size()-1 )
        {
            //get next line
            port2 = getPortFromLine(fileData[i+1]);

            if ( port1.value() == port2.value() )
            {
                port1.addProtocols(port2.protocols());
                ++i; //ignores next line
            }
        }

        m_ports.push_back(port1);
    }

}



bool AddExceptionDialog::isValidException(QString& text)
{
    if ( isPort( text ) )
        return true;
    else if( isValidIp(text) )
        return true;

    return false;
}


QStringList AddExceptionDialog::getConnections()
{
    QStringList connections;

    if ( m_OutCheck->checkState() == Qt::Checked )
        connections << m_OutCheck->text();

    if ( m_InCheck->checkState() == Qt::Checked )
        connections << m_InCheck->text();

    if ( m_FwdCheck->checkState() == Qt::Checked )
        connections << m_FwdCheck->text();

    return connections;
}

QStringList AddExceptionDialog::getProtocols(bool isIp)
{
    QStringList protocols;

    if ( isIp )
        protocols << "IP";
    else
    {
        if ( m_TcpCheck->checkState() ==  Qt::Checked )
            protocols << m_TcpCheck->text();
        if ( m_UdpCheck->checkState() ==  Qt::Checked )
            protocols << m_UdpCheck->text();
    }

    return protocols;
}



QStringList AddExceptionDialog::getParams(const QString& text)
{
    if ( mode == (ADD_MODE | BLOCKLIST_MODE) )
    {
        QStringList params, paths("");
        QString param;
        QFileInfo file("");
        bool append = false;
        params = text.split(' ');
 
 
        for(int i=0; i < params.size(); i++)
        {
            
            if ( append )
                param += params[i];
            else
                param = params[i];
                
            //if it's a filepath
            if ( param.trimmed()[0] == '/' || append)
            {
                file.setFile(param.trimmed());

                //if the file exists and it's not a directory
                //it means it's the full path
                if ( file.exists() && ( ! file.isDir()) )
                {
                    paths << param.trimmed();
                    param.clear();
                    append = false;
                }
                else  //or else we assume it's a part of the path
                {
                    param += " "; //since the string was splitted by spaces, we have to re add them to reconstruct the string properly
                    append = true;
                }
            }
            else if ( ! param.simplified().isEmpty() )//if it's not an empty string it might be an URL
            {
                paths << param;
                param.clear();
            }
        }
        
        if ( ! param.isEmpty() )
            paths << param;
        
        return paths;
    }
    else
        return text.split(' ', QString::SkipEmptyParts);
}

bool AddExceptionDialog::isPort(QString & p)
{
    if ( p.contains(":") )
    {
        QStringList ports = p.split(":");
        
        if ( ports.size() > 2 )
            return false;
        
        foreach(QString port, ports)
            if ( port != QString::number(port.toInt()) )
                return false;
                
        return true;
    }
    
    
    if ( p == QString::number(p.toInt()) )
        return true;

    if ( ports.contains(p.toLower()) )
        return true;

    return false;
}


bool AddExceptionDialog::isValidWhitelistItem(WhitelistItem& whiteItem, QString& reason)
{
    reason = "";

    //checks if the new item doesn't already exist
    foreach(WhitelistItem tempItem, m_Items)
        if ( tempItem == whiteItem )
        {
            reason = QObject::tr("It's already added");
            return false;
        }

    foreach(Port port, m_ports)
        if ( port == whiteItem )
            if ( ! port.hasProtocol(whiteItem.protocol()) )
            {
                reason += whiteItem.value();
                reason += QObject::tr(" doesn't work over ") + whiteItem.protocol();
                return false;
            }

    return true;
}

void AddExceptionDialog::setWhitelistItems(QString& value, bool isIp)
{
    QList<WhitelistItem> items;
    QStringList protocols, connections;

    protocols = getProtocols(isIp);
    connections = getConnections();

    QString reason;

    foreach(QString protocol, protocols)
    {
        foreach(QString connection, connections)
        {
            WhitelistItem item = WhitelistItem(value, connection, protocol);

            if ( isValidWhitelistItem(item, reason) )
                m_validItems << item;
            else
            {
                m_invalidItems << item;
                reasons << reason;
            }
        }

    }
}

int AddExceptionDialog::getPort(const QString& p)
{
    if ( ports.contains(p.toUpper()) )
        return ports[p.toUpper()];

    if ( p == QString::number(p.toInt()) )
        return p.toInt();

    return -1;
}

void AddExceptionDialog::addEntry()
//Used by exception (whitelist) window
{
    if ( m_addEdit->text().isEmpty() )
        return;

    m_invalidItems.clear();
    m_validItems.clear();
    reasons.clear();

    QStringList values, info;
    bool ip = false;
    QStringList unrecognizedValues;


    values = getParams(m_addEdit->text());

    foreach(QString param, values)
    {
        param = param.trimmed();

        if ( param.isEmpty() )
            continue;

        if ( isPort(param) )
            ip = false;
        else if ( isValidIp(param ) )
            ip = true;
        else
        {
            unrecognizedValues << param;
            continue;
        }

        setWhitelistItems(param, ip);
    }

    m_notValidTreeWidget->clear();

    if ( ! unrecognizedValues.isEmpty() || ! m_invalidItems.isEmpty() )
    {
         QStringList connections, protocols;

        if ( ! groupBox_2->isVisible() )
            groupBox_2->setVisible(true);

        foreach(QString value, unrecognizedValues)
        {
            QStringList info; info << value << "ANY" << "ANY" << "Not a valid IP nor a Port";
            QTreeWidgetItem *item = new QTreeWidgetItem(m_notValidTreeWidget, info);
            m_notValidTreeWidget->addTopLevelItem(item);
        }

        for(int i=0; i < m_invalidItems.size(); i++)
        {
            WhitelistItem whiteItem = m_invalidItems[i];
            QStringList info; info << whiteItem.value() << whiteItem.connection() << whiteItem.protocol() << reasons[i];
            QTreeWidgetItem *item = new QTreeWidgetItem(m_notValidTreeWidget, info);
            m_notValidTreeWidget->addTopLevelItem(item);
        }

    }
    else
        emit( accept() );

}

//************************** Blocklist dialog ***************************//

void AddExceptionDialog::selectLocalBlocklist()
{
    QString filter;
    filter += "All Supported files  (*.p2p *.gz *.7z *.zip *.dat );;";
    filter += "P2P (*.p2p);;Zip (*.zip);; 7z (*.7z);;Gzip (*.gz);;Dat (*.dat)";

    QStringList files = selectFiles(this, filter);

    QString text = m_addEdit->text();
    text += files.join(QString(" "));
    text += " ";
    m_addEdit->setText(text);
}


void AddExceptionDialog::addBlocklist()
{
    QStringList values = getParams(m_addEdit->text());
    m_notValidTreeWidget->clear();
    QStringList invalidValues;
    m_blocklists.clear();

    foreach(const QString& value, values)
    {
        if ( value.simplified().isEmpty() )
            continue;
        
        if ( ! ListItem::isValidBlockList(value) )
            invalidValues << value;
        else
            m_blocklists.push_back(value);
    }

    if ( invalidValues.isEmpty() )
        emit(accept());
    else
    {
        if ( ! groupBox_2->isVisible() )
        {
            resize(width(), height()*2);
            groupBox_2->setVisible(true);
        }
        
        for(int i=0; i < invalidValues.size(); i++)
        {
            QStringList info;
            QString reason(tr("Not recognized as a valid local path nor it seems a valid URL (did you forget to prepend http, https or ftp?)."));
            info << invalidValues[i] << "N/A" << "N/A" << reason;
            QTreeWidgetItem *item = new QTreeWidgetItem(m_notValidTreeWidget, info);
            item->setToolTip(3, reason);
        }
    }
}




//Dialog specific

void AddExceptionDialog::keyPressEvent ( QKeyEvent * e )
{
    if (e->key() == Qt::Key_Escape)
        QDialog::keyPressEvent (e);

}




