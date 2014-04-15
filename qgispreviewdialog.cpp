#include "qgispreviewdialog.h"
#include "previewwindow.h"


QgisPreviewDialog::QgisPreviewDialog(QString fileName, QWidget *parent) :
    QDialog(parent)
{


       PreviewWindow * previewWindow = new PreviewWindow(); // or your own class
                                                     // inheriting QMainWindow

       previewWindow->addLayer(fileName);
       QHBoxLayout * layout = new QHBoxLayout();
       layout->addWidget(previewWindow);
       setLayout(layout);




}
