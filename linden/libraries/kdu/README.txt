
Unpack the Kakadu library source into the "source" directory, 
e.g. "kdu/source/coresys" then build the library using the
included compiling instructions. 

After compiling Kakadu, binary files will be created in a 
subdirectory, for example a "linden\libraries\kdu\lib_x86" 
folder will be created for the x86 release and debug libs on 
Windows.

CMake and the default build system are NOT used to build KDU.
You need to acquire the KDU source yourself in order to add
it here. 

For more on how CMake works with the iw_kdu_loader project,
read the iw_kdu_loader section in "indra/cmake/InWorldz.cmake".
