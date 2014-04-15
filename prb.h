#ifndef PRB_H
#define PRB_H

#include <QWidget>
#include <QString>
#include <QMap>

namespace Ui {
class Prb;
}

class Prb : public QWidget
{
    Q_OBJECT
    
public:
    explicit Prb(QWidget *parent = 0);
    ~Prb();


private slots:
    void on_reprojectButton_clicked();
    void on_inputPreviewButton_clicked();
    void on_outputPreviewButton_clicked();
    void on_inputFileNameButton_clicked();
    void on_outputFileNameButton_clicked();

private:
    Ui::Prb *ui;
    QMap<QString, int>  srsValues;
    void popupMessageBox(QString message);
    QString validatePartitionSize(QString partitionSize);
    QString validateInputFileName(QString fileName);
    QString validateOutputFileName(QString fileName);
    QString validateSrsValue(QString srs);
    QString validateElipsoidValue(QString aStr, QString aOrB);
    QString validateFillValue(QString fillValue);

};

#endif // PRB_H
