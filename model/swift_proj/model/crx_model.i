%module crx_model

%include <std_string.i>

%include "../src/run.h"


%{
  typedef int MPI_Comm;
  #include "../src/run.h"
%}

typedef int MPI_Comm;
