///
/// Copyright 0000 <Nobody>
/// @file
/// @author David Matthew Mattli <dmattli@usgs.gov>
///
/// @section LICENSE
///
/// This software is in the public domain, furnished "as is", without
/// technical support, and with no warranty, express or implied, as to
/// its usefulness for any purpose.
///
/// @section DESCRIPTION
///
/// This file demonstrates how to use librasterblaster to implement parallel
/// raster reprojection. This implementation uses a MPI I/O to write to a tiff
/// file in parallel.
///
///

#include <vector>

#include "src/configuration.h"
#include "src/quadtree.h"
#include "src/reprojection_tools.h"

#include "sptw.h"
#include "src/utils.h"


//#include <QtWidgets>
#include <QTextEdit>
#include <QString>


using librasterblaster::Area;
using librasterblaster::PartitionBySize;
using librasterblaster::RasterChunk;
using librasterblaster::Configuration;
using librasterblaster::PRB_ERROR;
using librasterblaster::PRB_BADARG;
using librasterblaster::PRB_NOERROR;

using sptw::PTIFF;
using sptw::write_subrow;
using sptw::write_rasterchunk;
using sptw::open_raster;
using sptw::SPTW_ERROR;

void MyErrorHandler(CPLErr eErrClass, int err_no, const char *msg) {
        return;
}
/*! \page prasterblasterpio

\htmlonly
USAGE:
\endhtmlonly

\verbatim
prasterblasterpio [--t_srs target_srs] [--s_srs source_srs] 
                  [-r resampling_method] [-n partition_size]
                  [--dstnodata no_data_value]
                  source_file destination_file

\endverbatim

\section prasterblasterpio_description DESCRIPTION

<p> 

The prasterblasterpio demo program implements parallel raster reprojection and
demonstrates the use of librasterblaster. The implementation can be found in
prasterblaster-pio.cc.

</p>
 */
namespace librasterblaster {

/** Main function for the prasterblasterpio program */
PRB_ERROR prasterblasterpio(Configuration conf, QTextEdit *textEdit) {
  RasterChunk *in_chunk, *out_chunk;
  double start_time, end_time;

  start_time = MPI_Wtime();

  int rank = 0;
  int process_count = 1;
  char buffer[1024];
  QString cur;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &process_count);

  // Replace CPLErrorHandler
  CPLPushErrorHandler(MyErrorHandler);

  GDALAllRegister();

  
  // jdw, not needed, file name validation done in GUI 
  if (conf.input_filename == "" || conf.output_filename == "") {
    fprintf(stderr, "Specify an input and output filename\n");
    return PRB_BADARG;
  }


  // Open the input raster
  //
  // The input raster is only read so we can use the serial i/o provided by the
  // GDAL library.
  GDALDataset *input_raster =
      static_cast<GDALDataset*>(GDALOpen(conf.input_filename.c_str(),
                                         GA_ReadOnly));
 
  // jdw, file existence validated by GUI, input raster may be null if file is not a valid raster file? 
  if (input_raster == NULL) {
    fprintf(stderr, "Error opening input raster!\n");
    return PRB_IOERROR;
  }

  // If we are the process with rank 0 we are responsible for the creation of
  // the output raster.
  if (rank == 0) {
    OGRSpatialReference sr;
    char *wkt;
    sr.SetFromUserInput(input_raster->GetProjectionRef());
    sr.exportToPrettyWkt(&wkt);



    printf("***** Beginning reprojection *****\n");
    sprintf(buffer, "***** Beginning reprojection *****\n");
    cur = textEdit->toPlainText();
    textEdit->setPlainText(cur + buffer);




    printf("Input File: %s\nOutput File: %s\n",
           conf.input_filename.c_str(), conf.output_filename.c_str());

    sprintf(buffer, "Input File: %s\nOutput File: %s\n",
            conf.input_filename.c_str(), conf.output_filename.c_str());

    cur = textEdit->toPlainText();
    textEdit->setPlainText(cur + buffer);



    printf("Input Projection: %s\nOutput Projection: %s\n",
           wkt, conf.output_srs.c_str());
    sprintf(buffer,"Input Projection: %s\nOutput Projection: %s\n",
            wkt, conf.output_srs.c_str());
    cur = textEdit->toPlainText();
    textEdit->setPlainText(cur + buffer);

    printf("Process Count: %d\n", process_count);
    sprintf(buffer,"Process Count: %d\n", process_count);
    cur = textEdit->toPlainText();
    textEdit->setPlainText(cur + buffer);

    OGRFree(wkt);

    // Now we have to create the output raster
    printf("Creating output raster...");
    sprintf(buffer,"Creating output raster...");
    cur = textEdit->toPlainText();
    textEdit->setPlainText(cur + buffer);

    double gt[6];
    input_raster->GetGeoTransform(gt);
    PRB_ERROR err = librasterblaster::CreateOutputRaster(input_raster,
                                                         conf.output_filename,
                                                         gt[1],
                                                         conf.output_srs);
    // jdw, shouldn't happen as output file name supplied by user is tested for write access by GUI validation. 
    if (err != PRB_NOERROR) {
      fprintf(stderr, "Error creating raster!: %d\n", err);
      return PRB_IOERROR;
    }

    printf(" Output raster created.\n");
    sprintf(buffer," Output raster created.\n");
    cur = textEdit->toPlainText();
    textEdit->setPlainText(cur + buffer);

  }

  // Wait for rank 0 to finish creating the file
  MPI_Barrier(MPI_COMM_WORLD);

  // Now open the new output file as a ProjectedRaster object. This object will
  // only be used to read metadata. It will _not_ be used to write to the output
  // file.
  PTIFF* output_raster = open_raster(conf.output_filename);
  GDALDataset *gdal_output_raster =
      static_cast<GDALDataset*>(GDALOpen(conf.output_filename.c_str(),
                                         GA_ReadOnly));

  if (output_raster == NULL) {
    fprintf(stderr, "Could not open output raster\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
    return PRB_IOERROR;
  }

  // Now we will partition the output raster space. We will use a
  // maximum_height of 1 because we want single row or smaller
  // partitions to work with sptw.
  vector<Area> partitions = PartitionBySize(rank,
                                            process_count,
                                            output_raster->y_size,
                                            output_raster->x_size,
                                            conf.partition_size);
  if (rank == 0) {
    printf("Typical process has %lu partitions with base size: %d\n",
           static_cast<unsigned long>(partitions.size()),
           conf.partition_size);
    sprintf(buffer,"Typical process has %lu partitions with base size: %d\n",
            static_cast<unsigned long>(partitions.size()),
            conf.partition_size);
    cur = textEdit->toPlainText();
    textEdit->setPlainText(cur + buffer);

  }
  double read_start, read_total;
  double write_start, write_total;
  double resample_start, resample_total;

  read_total = write_total = resample_total = 0.0;

  int percent_done = 0;
  int last_percent_done = -1;
  if(textEdit != NULL){
    printf("Percent complete:\n");
    sprintf(buffer,"Percent complete:\n");
    cur = textEdit->toPlainText();
    textEdit->setPlainText(cur + buffer);
  }

  // Now we loop through the returned partitions
  for (size_t i = 0; i < partitions.size(); ++i) {

      percent_done = (int) ((double)i+1)/((double)partitions.size()) * 100.0;
      if(percent_done != last_percent_done){
          last_percent_done = percent_done;
          if(textEdit != NULL){
            printf("% d", percent_done);
            sprintf(buffer,"%d ", percent_done);
            cur = textEdit->toPlainText();
            textEdit->setPlainText(cur + buffer);
          }
          fflush(stdout);

      }

    // Swap y-axis of partition
    double t = partitions.at(i).lr.y;
    partitions.at(i).lr.y = partitions.at(i).ul.y;
    partitions.at(i).ul.y = t;

    // Now we use the ProjectedRaster object we created for the input file to
    // create a RasterChunk that has the pixel values read into it.
    in_chunk = RasterChunk::CreateRasterChunk(input_raster,
                                              gdal_output_raster,
                                              partitions.at(i));

    read_start = MPI_Wtime();
    PRB_ERROR chunk_err = RasterChunk::ReadRasterChunk(input_raster, in_chunk);
    if (chunk_err != PRB_NOERROR) {
      fprintf(stderr, "Error reading input chunk!\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
      return PRB_IOERROR;
    }
    read_total += MPI_Wtime() - read_start;

    // We want a RasterChunk for the output area but we area going to generate
    // the pixel values not read them from the file so we use
    // CreateRasterChunk
    out_chunk = RasterChunk::CreateRasterChunk(gdal_output_raster,
                                               partitions.at(i));
    if (out_chunk == NULL) {
      fprintf(stderr, "Error allocating output chunk! %f %f %f %f'n",
              partitions[i].ul.x,
              partitions[i].ul.y,
              partitions[i].lr.x,
              partitions[i].lr.y);
      return PRB_BADARG;
    }

    // Now we call ReprojectChunk with the RasterChunk pair and the desired
    // resampler. ReprojectChunk performs the reprojection/resampling and fills
    // the output RasterChunk with the new values.
    resample_start = MPI_Wtime();
    bool ret = ReprojectChunk(in_chunk,
                              out_chunk,
                              conf.fillvalue,
                              conf.resampler);
    if (ret == false) {
            fprintf(stderr, "Error reprojecting chunk!\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
    }
    resample_total += MPI_Wtime() - resample_start;

    write_start = MPI_Wtime();
    SPTW_ERROR err;
    err = sptw::write_rasterchunk(output_raster,
                                  out_chunk);
    write_total += MPI_Wtime() - write_start;
    if (err != sptw::SP_None) {
      fprintf(stderr, "Rank %d: Error writing chunk!\n", rank);
      MPI_Abort(MPI_COMM_WORLD, 1);
    }


    MPI_Barrier(MPI_COMM_WORLD);
    delete in_chunk;
    delete out_chunk;
  }
  printf("\n");
  if(textEdit != NULL){
    sprintf(buffer,"\n");
    cur = textEdit->toPlainText();
    textEdit->setPlainText(cur + buffer);
  }
  // Clean up
  close_raster(output_raster);
  output_raster = NULL;
  delete gdal_output_raster;
  delete input_raster;

  // Report runtimes
  end_time = MPI_Wtime();
  double runtimes[4] = { end_time - start_time,
                         read_total,
                         write_total,
                         resample_total};
  std::vector<double> process_runtimes(process_count*4);
  MPI_Gather(runtimes,
             4,
             MPI_DOUBLE,
             &(process_runtimes[0]),
             4,
             MPI_DOUBLE,
             0,
             MPI_COMM_WORLD);

  if (rank == 0) {
    printf("Performance:\nRank  Total Runtime  Read  I/O  Write I/O  Resampling\n");
    sprintf(buffer,"Performance:\nRank  Total Runtime  Read  I/O  Write I/O  Resampling\n");
    cur = textEdit->toPlainText();
    textEdit->setPlainText(cur + buffer);

    for (unsigned int i = 0; i < process_runtimes.size(); i+=4) {
      printf("%5u| %15.4fs| %7.4fs| %8.4fs| %11.4fs\n",
             i/4,
             process_runtimes.at(i),
             process_runtimes.at(i+1),
             process_runtimes.at(i+2),
             process_runtimes.at(i+3));
      if(textEdit != NULL){
        sprintf(buffer,"%5u| %15.4fs| %7.4fs| %8.4fs| %11.4fs\n",
                  i/4,
                  process_runtimes.at(i),
                  process_runtimes.at(i+1),
                  process_runtimes.at(i+2),
                  process_runtimes.at(i+3));
        cur = textEdit->toPlainText();
        textEdit->setPlainText(cur + buffer);
      }


    }
    printf("\n");
    sprintf(buffer,"\n");
    cur = textEdit->toPlainText();
    textEdit->setPlainText(cur + buffer);


  }
  return PRB_NOERROR;
}

}
