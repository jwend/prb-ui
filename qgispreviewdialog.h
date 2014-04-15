#ifndef QGISPREVIEWDIALOG_H
#define QGISPREVIEWDIALOG_H

#include <QDialog>
#include <QString>

class QgisPreviewDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QgisPreviewDialog(QString fileName, QWidget *parent = 0);
    
signals:
    
public slots:
    
};

#endif // QGISPREVIEWDIALOG_H
