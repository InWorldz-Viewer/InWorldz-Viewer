Unpack the Kakadu library source into the "source" directory, 
then build the library. 

Binary files should be created in folders in this directory, 
for example a "linden\libraries\kdu\lib_x86" folder will be 
created for the x86 release and debug libs.

CMake and the default build system are NOT used to build KDU.
You need to acquire the KDU source yourself in order to add
it here. 

For more on how CMake works with the iw_kdu_loader project,
check out the implementation in "indra/cmake".

-- McCabe 
