 
#ifndef ADD_EXCEPTION_DIALOG_H
#define ADD_EXCEPTION_DIALOG_H

#include <QDialog>
#include <QWidget>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QProcess>
#include <QString>
#include <QVector>
#include <QVariant>
#include <QList> 
#include <QDebug>
#include <QAbstractButton>
#include <QKeyEvent>
#include <QObject>

#include "ui_add_exception.h"

typedef enum openMode { ADD_MODE=0x000,
                EDIT_MODE,
                BLOCKLIST_MODE,
                EXCEPTION_MODE
            };
        
            

class AddExceptionDialog : public QDialog, private Ui::AddExceptionDialog {

	Q_OBJECT
    
    QMap<QString, int> ports;
	
	public:
        AddExceptionDialog(QWidget *parent = 0, int mode=0);
        ~AddExceptionDialog();
        int getPort(const QString&);
        bool isValidIp(const QString &)const;
    
    public slots:
        void addEntry();
    
    protected:
        void keyPressEvent ( QKeyEvent * e );
        
};


#endif
