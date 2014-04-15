Install instuctions for prasterblaster GUI desktop application (PrbUi2) 
for Ubuntu 12.04 LTS (precise) or later, x86_64.

Prepare Machine
  Prepare machine for building from source by running:
```
sudo apt-get install build-essential gdb
```

Install QT 4
 
  First install dependencies:
```
sudo apt-get build-dep qt4-qmake
``` 
 Then go to:
http://qt-project.org/downloads
  and download:
Qt libraries 4.8.5 for Linux/X11.
  Extract the file and follow the instructions in the INSTALL file.
  The default installation commands are:
```
./configure
make
sudo make install
```
  Add the following to your shell startup file, e.g., .bashrc:
```
PATH=/usr/local/Trolltech/Qt-4.8.5/bin:$PATH
export PATH
```

Install QGis Stable

  Download and install QGis Stable by following the instructions here:
http://www.qgis.org/en/site/forusers/download.html

  For Ubuntu 12.04 LTS (precise) the instructions are:
  First, add the following to your /etc/apt/sources.list file:
```
deb         http://qgis.org/debian precise main
deb-src http://qgis.org/debian precise main
```
  Then run:
```
gpg --keyserver keyserver.ubuntu.com --recv 47765B75
gpg --export --armor 47765B75 | sudo apt-key add -
sudo apt-get update
sudo apt-get install qgis python-qgis libqgis-dev
```

Install Open MPI.

  Run:
```
sudo apt-get install openmpi-dev
sudo apt-get install openmpi-bin
```

Install GDAL and Proj

  Run the buildgdal.sh script from the distribution directory:
```
./buildgdal.sh
```

Compile the PrbUi2 executable:

  From the distribution directory run:
```
qmake
make
```
  Add the following to your shell startup file, i.e., .bashrc:  
```
export LD_LIBRARY_PATH=/home/jwendel/prbui2/gdal/lib:$LD_LIBRARY_PATH
```
  Run the PrbUi2 executable with the following:
```
./PrbUi2
```
To use more than one processor on a machine run:
mpirun -n <number of processors to use> PrbUi2
For example, on a four cpu machine try:
```
mpirun -n 4 PrbUi2
```

