/*

The MIT License (MIT)

Copyright (c) 2018 Tim Warburton

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "e3sm.h"

int main(int argc, char **argv){

  // start up MPI
  MPI_Init(&argc, &argv);

  if(argc!=2){
    printf("usage: ./e3smMain setupfile\n");
    exit(-1);
  }

  // if argv > 2 then should load input data from argv
  setupAide options(argv[1]);
  
  // set up mesh stuff
  string fileName;
  int N, Nz, Nstencil, Nfields;

  options.getArgs("HORIZONTAL POLYNOMIAL DEGREE", N);
  options.getArgs("VERTICAL RESOLUTION", Nz);
  options.getArgs("VERTICAL STENCIL", Nstencil);
  options.getArgs("NUMBER FIELDS", Nfields);

  int Nvgeo = 5;
  int Nq = N+1;

  int RXID = 0, SXID = 0, RYID = 0, SYID = 0, TZID = 0;
  
  char deviceConfig[BUFSIZ];

  int device_id, platform_id;
  options.getArgs("DEVICE NUMBER" ,device_id);

  if(options.compareArgs("THREAD MODEL", "CUDA")){
    sprintf(deviceConfig, "mode: 'CUDA', device_id: %d",device_id);
  }
  else if(options.compareArgs("THREAD MODEL", "HIP")){
    sprintf(deviceConfig, "mode: 'HIP', device_id: %d",device_id);
  }
  else if(options.compareArgs("THREAD MODEL", "OpenCL")){
    int plat;
    options.getArgs("PLATFORM NUMBER", plat);
    sprintf(deviceConfig, "mode: 'OpenCL', device_id: %d, platform_id: %d", device_id, plat);
  }
  else if(options.compareArgs("THREAD MODEL", "OpenMP")){
    sprintf(deviceConfig, "mode: 'OpenMP' ");
  }
  else{
    sprintf(deviceConfig, "mode: 'Serial' ");
  }

  // get and configure device
  occa::device device;
  device.setup(deviceConfig);

  occa::properties props;

  props["defines"].asObject();
  props["includes"].asArray();
  props["header"].asArray();
  props["flags"].asObject();


  props["defines/p_Nfields"] = Nfields;
  props["defines/p_Nq"] = Nq;
  props["defines/p_Nz"] = Nz;
  props["defines/p_Nstencil"] = Nstencil;
  props["defines/p_Nvgeo"] = Nvgeo;

  props["defines/dfloat"] = "double";
  props["defines/dlong"] = "int";

  props["defines/p_RXID"] = RXID;
  props["defines/p_SXID"] = SXID;
  props["defines/p_RYID"] = RYID;
  props["defines/p_SYID"] = SYID;
  props["defines/p_TZID"] = TZID;
  
  // build kernel
  occa::kernel kernel = device.buildKernel("okl/E3SM.okl", "E3SM_divFlux", props); 
  
  // close down MPI
  MPI_Finalize();

  exit(0);
  return 0;
}
