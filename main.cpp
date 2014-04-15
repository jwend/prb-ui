
#include "prb.h"

#include <QApplication>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <mpi.h>
#include <src/configuration.h>
#include <prasterblaster-pio.h>



int main(int argc, char *argv[])
{
    int rtn;
    int rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0){
        //QApplication::setStyle("plastique");
        //QApplication::setDesktopSettingsAware(true);
        //QApplication::setEffectEnabled(true);

        QString myPluginsDir = "/usr/lib/qgis/plugins";
        QgsProviderRegistry * preg = QgsProviderRegistry::instance(myPluginsDir);
        QString pluglist=preg->pluginList();
        //printf("plugins: %s\n",pluglist.toStdString().c_str());
        //fflush(stdout);

        QgsApplication a(argc, argv, TRUE);
        //QApplication a(argc, argv);
        Prb w;
        w.show();
        rtn = a.exec();
    }
    else{
        // wait
        int runCnt = 0;
        while (true){

            int rank;
            MPI_Comm_rank(MPI_COMM_WORLD, &rank);
            printf("************ Run count = %i for rank %i\n", ++runCnt, rank);
            fflush(stdout);

            librasterblaster::Configuration conf;
            // local variables for MPI_Recv
            MPI_Status status;
            char str[1000];
            int n;
            long m;
            //receive the partition size
            MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            conf.partition_size = n;
            // receive the input file
            MPI_Recv(str, 1000, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
            conf.input_filename = str;
            // receive the output file
            MPI_Recv(str, 1000, MPI_CHAR, 0, 2, MPI_COMM_WORLD, &status);
            conf.output_filename = str;
            //receive the resampler;
            MPI_Recv(&(conf.resampler), sizeof(enum librasterblaster::RESAMPLER), MPI_BYTE, 0, 3, MPI_COMM_WORLD, &status);
            // receive the output srs
            MPI_Recv(str, 1000, MPI_CHAR, 0, 4, MPI_COMM_WORLD, &status);
            conf.output_srs = str;
            //receive the fillvalue
            MPI_Recv(str, 1000, MPI_CHAR, 0, 5, MPI_COMM_WORLD, &status);
            conf.fillvalue = str;


            // reporting

            printf("process %i received partition size %i\n", rank, conf.partition_size);
            printf("process %i received input file %s\n", rank, conf.input_filename.c_str());
            printf("process %i received output file file %s\n", rank, conf.output_filename.c_str());
            //printf("process %i received resampler %l\n", rank, conf.resampler);

            if(conf.resampler == librasterblaster::MIN){
                printf("process %i received resampler MIN\n", rank);
            }
            else if(conf.resampler == librasterblaster::MAX){
                printf("process %i received resampler MAX\n", rank);
            }
            else if(conf.resampler == librasterblaster::NEAREST){
                printf("process %i received resampler NEAREST\n", rank);
            }
            else  if(conf.resampler == librasterblaster::MEAN){
                printf("process %i received resampler MEAN\n", rank);
            }
            else {
                 printf("process %i received resampler NONE OF THE ABOVE\n", rank);
            }
            printf("process %i received output srs %s\n", rank, conf.output_srs.c_str());
            printf("process %i received fillvalue file %s\n", rank, conf.fillvalue.c_str());
            fflush(stdout);
            int ret =  librasterblaster::prasterblasterpio(conf, NULL);


        }






    }



    MPI_Finalize();
    return rtn;
}
