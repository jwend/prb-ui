#include <QFile>
#include <QTextStream>

#include "prb.h"
#include "ui_prb.h"

#include <mpi.h>
#include "src/configuration.h"
#include "prasterblaster-pio.h"
#include <algorithm>
#include <string>
#include <climits>
#include <iostream>
#include <stdexcept>

//! [0]
//#include <QtWidgets>
#include <QFileDialog>
#include <QMessageBox>

#include "qgispreviewdialog.h"

using librasterblaster::Configuration;
using librasterblaster::prasterblasterpio;
using librasterblaster::RasterChunk;
using namespace std;

//librasterblaster::Configuration conf;

Prb::Prb(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Prb)
{
    ui->setupUi(this);

    ui->resamplerComboBox->addItem("MIN");
    ui->resamplerComboBox->addItem("MAX");
    ui->resamplerComboBox->addItem("NEAREST");
    ui->resamplerComboBox->addItem("MEAN");

    ui->srsComboBox->addItem("moll");
    ui->srsComboBox->setEditText("moll");

    ui->elipsoidAComboBox->addItem("6370997");
    ui->elipsoidAComboBox->setEditText("6370997");

    ui->elipsoidBComboBox->addItem("6370997");
    ui->elipsoidBComboBox->setEditText("6370997");

    srsValues.insert("moll", 0);

}

Prb::~Prb()
{
    delete ui;
}

void  Prb::on_inputFileNameButton_clicked(){

    QString fileName = QFileDialog::getOpenFileName(this, tr("Input File"));
    ui->inputFileNameLineEdit->setText(fileName);
}

void  Prb::on_outputFileNameButton_clicked(){

    QString fileName = QFileDialog::getSaveFileName(this, tr("Output File"), "", tr(""));
    ui->outputFileNameLineEdit->setText(fileName);

}


void  Prb::on_reprojectButton_clicked(){

    //./prasterblasterpio --t_srs +proj=moll -n 21600 tests/testdata/veg_geographic_1deg.tif  tests/testoutput/veg_geographic_1deg.tif
    int argc;
    char **argv;
    int rank = 0;
    int process_count = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &process_count);
    RasterChunk *in_chunk, *out_chunk;

    // Initialize Configuration object
    //Configuration conf(argc, argv);

    Configuration conf;
    int validInput = 1;
    QString validationMessage;

    //printf("*** %s ***%i \n", partitionSizeStr.toStdString().c_str(), conf.partition_size);

    // Validate partition size
    QString partitionSize = ui->partitionSizeLineEdit->text();
    validationMessage = validatePartitionSize(partitionSize);
    if(validationMessage.length()>0){
        validInput = 0;
        popupMessageBox(validationMessage);
    }
    else{
        conf.partition_size =  partitionSize.toInt();
    }

    // Validate input file
    validationMessage = validateInputFileName(ui->inputFileNameLineEdit->text());
    if(validationMessage.length()>0){
        validInput = 0;
        popupMessageBox(validationMessage);
    }
    else{
        conf.input_filename = ui->inputFileNameLineEdit->text().toStdString();
    }

    // Validate output file
    validationMessage = validateOutputFileName(ui->outputFileNameLineEdit->text());
    if(validationMessage.length()>0){
        validInput = 0;
        popupMessageBox(validationMessage);
    }
    else{
        conf.output_filename =  ui->outputFileNameLineEdit->text().toStdString();
    }


    // Validate Resampler
    string resamplerStr = ui->resamplerComboBox->currentText().toStdString();
    std::transform(resamplerStr.begin(), resamplerStr.end(), resamplerStr.begin(), ::tolower);
    if(!resamplerStr.compare("min")){
        conf.resampler = librasterblaster::MIN;
    }
    else if (!resamplerStr.compare("max")){
        conf.resampler = librasterblaster::MAX;
    }
    else if (!resamplerStr.compare("nearest")){
        conf.resampler = librasterblaster::NEAREST;
    }
    else if (!resamplerStr.compare("mean")){
        conf.resampler = librasterblaster::MEAN;
    }
    else {
        validationMessage = "Resmapler value must be one of MIN, MAX, NEAREST, or MEAN.";
        validInput = 0;
        popupMessageBox(validationMessage);

    }

    // Validate SRS
    string srsStr;
    validationMessage = validateSrsValue(ui->srsComboBox->currentText());
    if(validationMessage.length()>0){
        validInput = 0;
        popupMessageBox(validationMessage);

    }
    else{
        srsStr = ui->srsComboBox->currentText().toStdString();
   }

    // Validate Elipsoid A value
    string aStr;
    validationMessage = validateElipsoidValue(ui->elipsoidAComboBox->currentText(), "A");
    if(validationMessage.length()>0){
        validInput = 0;
        popupMessageBox(validationMessage);

    }
    else{
        aStr = ui->elipsoidAComboBox->currentText().toStdString();
   }

   // Validate Elipsoid B value
   string bStr;
   validationMessage = validateElipsoidValue(ui->elipsoidBComboBox->currentText(), "B");
    if(validationMessage.length()>0){
        validInput = 0;
        popupMessageBox(validationMessage);

    }
    else{
        bStr = ui->elipsoidBComboBox->currentText().toStdString();
    }

    // Build projection string from srs and elipsoid values
    string projStr = "+proj=" + srsStr + " +a=" + aStr + " +b=" + bStr;
    conf.output_srs = projStr;


    // Validate Fill Value
    QString fillValue = ui->fillValueLineEdit->text();
    validationMessage = validateFillValue(fillValue);
    if(validationMessage.length()>0){
        validInput = 0;
        popupMessageBox(validationMessage);
    }
    else{
        conf.fillvalue=  fillValue.toStdString();
    }


    //cout<<"validInput = :"<<validInput;
    //cout.flush();
    //cerr.flush();
    if(validInput){


        //MPI_Barrier(MPI_COMM_WORLD);
        // local variables for MPI_Send
        char str[1000];
        int n;
        //long m;
        //send the partition size;
        n=conf.partition_size;
        for(int i=1; i<process_count; i++){
            MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        // send the input file to other processes
        strcpy(str, conf.input_filename.c_str());
        for(int i=1; i<process_count; i++){
            MPI_Send(str, 1000, MPI_CHAR, i, 1, MPI_COMM_WORLD);
        }
        // send the output file to other processes
        strcpy(str, conf.output_filename.c_str());
        for(int i=1; i<process_count; i++){
            MPI_Send(str, 1000, MPI_CHAR, i, 2, MPI_COMM_WORLD);
        }
        //send the resampler;
        //m=(long)conf.resampler;
        for(int i=1; i<process_count; i++){
            MPI_Send(&(conf.resampler), sizeof(enum librasterblaster::RESAMPLER), MPI_BYTE, i, 3, MPI_COMM_WORLD);
        }
        // send the output srs string
        strcpy(str, conf.output_srs.c_str());
        for(int i=1; i<process_count; i++){
            MPI_Send(str, 1000, MPI_CHAR, i, 4, MPI_COMM_WORLD);
        }
        // send the fillvalue
        strcpy(str, conf.fillvalue.c_str());
        for(int i=1; i<process_count; i++){
            MPI_Send(str, 1000, MPI_CHAR, i, 5, MPI_COMM_WORLD);
        }

        int ret =  prasterblasterpio(conf, ui->outputTextEdit);
        //MPI_Finalize();
    }

    //return ret;

}

void Prb::on_inputPreviewButton_clicked(){


    struct stat buffer;
    if(stat(ui->inputFileNameLineEdit->text().toStdString().c_str(),&buffer)==0){
        QgisPreviewDialog *d = new QgisPreviewDialog(QString(ui->inputFileNameLineEdit->text().toStdString().c_str()));
        d->setWindowTitle(QString(ui->inputFileNameLineEdit->text().toStdString().c_str()));
        d->show();
    }
    else{
        popupMessageBox("File does not exist.");
    }

}

void Prb::on_outputPreviewButton_clicked(){

    struct stat buffer;
    if(stat(ui->outputFileNameLineEdit->text().toStdString().c_str(),&buffer)==0){
        QgisPreviewDialog *d = new QgisPreviewDialog(QString(ui->outputFileNameLineEdit->text().toStdString().c_str()));
        d->setWindowTitle(QString(ui->outputFileNameLineEdit->text().toStdString().c_str()));
        d->show();
    }
    else{
        popupMessageBox("File does not exist.");
    }

}


void Prb::popupMessageBox(QString message){
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
}

QString Prb::validatePartitionSize(QString partitionSize){

    QString message;
    bool validConversion;
    partitionSize.toInt(&validConversion);
    if(!validConversion){
        message.append("Could not convert Partition Size value to integer.");
    }
    return message;
}


QString Prb::validateInputFileName(QString fileName){

    QString message;
    bool exists = QFile::exists(fileName);
    if(!exists){
        message.append("Input File does not exist.");
    }
    return message;
}

QString Prb::validateOutputFileName(QString fileName){

    QString message;
    bool canCreate;
    QFile file(fileName);

    bool exists = QFile::exists(fileName);

    canCreate = file.open(QIODevice::ReadWrite);
    // cleanup after testing for open, only remove if a new file was created but didn't exist before this method
    if(!exists && canCreate){
        file.remove();
    }
    if(!canCreate){
        message.append("Can't open output file for writing.");
    }

    return message;
}

QString Prb::validateSrsValue(QString srs){
   QString message;
   bool contains = srsValues.contains(srs);
   if(!contains){
       message = "SRS value is invalid or not in current list.";
   }
   return message;
}


QString Prb::validateElipsoidValue(QString elipsoid, QString aOrB){

    QString message;
    int i;
    bool validConversion;

    i = elipsoid.toInt(&validConversion);
    if(!validConversion){
        message.append("Could not convert Elipsoid " + aOrB + " value to integer.");
        return message;
    }

    if(i<6000000 || i>7000000){
        message.append("Value for Elipsoid " + aOrB + " must be between 6000000 and 7000000.");
        return message;
    }

    return message;

}

QString Prb::validateFillValue(QString fillValue){

    QString message;
    bool validConversion;
    fillValue.toInt(&validConversion);
    if(!validConversion){
        message.append("Could not convert Fill Value to integer.");
    }
    return message;
}


