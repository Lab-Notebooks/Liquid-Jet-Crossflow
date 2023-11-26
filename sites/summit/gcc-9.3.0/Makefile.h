# FLASH makefile definitions for Summit (GCC compiler)
ifdef OLCF_GCC_ROOT
    PE_ENV=gnu
else
    $(error Cannot determine compiler module. \
            Load the gcc, pgi, or xl module---e.g. "module load gcc" \
            ---or specify a specific makefile with the setup argument---e.g. "-makefile=gcc")
endif
#----------------------------------------------------------------------------
# Set the HDF5/MPI library paths -- these need to be updated for your system
#----------------------------------------------------------------------------

MPI_PATH   = ${OLCF_OMPI_ROOT}
AMREX_PATH = $(AMREX$(NDIM)D_HOME)
HDF5_PATH  = ${OLCF_HDF5_ROOT}
HYPRE_PATH = ${OLCF_HYPRE_ROOT}
CUDA_PATH  = ${OLCF_CUDA_ROOT}
MAGMA_PATH = ${OLCF_MAGMA_ROOT}

ZLIB_PATH  =

PAPI_PATH  = ${OLCF_PAPI_ROOT}
PAPI_FLAGS =

NCMPI_PATH = ${OLCF_PARALLEL_NETCDF_ROOT}
MPE_PATH   =

ifdef OLCF_ESSL_ROOT
    HYPRE_VERSION = essl

    ESSL_PATH     = ${OLCF_ESSL_ROOT}
    CFLAGS_ESSL   = -I${ESSL_PATH}/include
    FFLAGS_ESSL   = -I${ESSL_PATH}/include
    LIB_ESSL      = -L${ESSL_PATH}/lib64 -lessl

    LAPACK_PATH   = ${ESSL_PATH}
    CFLAGS_LAPACK = ${CFLAGS_ESSL}
    FFLAGS_LAPACK = ${FFLAGS_ESSL}
    LIB_LAPACK    = ${LIB_ESSL}
else
    HYPRE_VERSION = default

    LAPACK_PATH   =
    CFLAGS_LAPACK =
    FFLAGS_LAPACK =
    LIB_LAPACK    =
endif

ifdef OLCF_NETLIB_LAPACK_ROOT
    LAPACK_PATH    = ${OLCF_NETLIB_LAPACK_ROOT}
    CFLAGS_LAPACK += -I${LAPACK_PATH}/include
    FFLAGS_LAPACK += -I${LAPACK_PATH}/include
    LIB_LAPACK    += -L${LAPACK_PATH}/lib64 -llapack -lblas
endif

#----------------------------------------------------------------------------
# Compiler and linker commands
#
#   Use the MPICH wrappers around the compilers -- these will automatically
#   load the proper libraries and include files.  Version of MPICH prior
#   to 1.2.2 (?) do not recognize .F90 as a valid Fortran file extension.
#   You need to edit mpif90 and add .F90 to the test of filename extensions,
#   or upgrade your MPICH.
#----------------------------------------------------------------------------

FCOMP   = mpifort
CCOMP   = mpicc
CPPCOMP = mpicxx -std=c++11
CUCOMP  = nvcc
LINK    = mpifort

# pre-processor flag
MDEFS =
PP    = -D

#----------------------------------------------------------------------------
# Compilation flags
#
#  Three sets of compilation/linking flags are defined: one for optimized
#  code, one for testing, and one for debugging.  The default is to use the
#  _OPT version.  Specifying -debug to setup will pick the _DEBUG version,
#  these should enable bounds checking.  Specifying _TEST is used for
#  flash_test, and is set for quick code generation, and (sometimes)
#  profiling.  The Makefile generated by setup will assign the generic token
#  (ex. FFLAGS) to the proper set of flags (ex. FFLAGS_OPT).
#----------------------------------------------------------------------------

OPENMP = -fopenmp

FFLAGS_AMREX = -I${AMREX_PATH}/include
FFLAGS_HDF5  = -I${HDF5_PATH}/include ${MDEFS}${PP}H5_USE_18_API
FFLAGS_NCMPI = -I${NCMPI_PATH}/include
FFLAGS_HYPRE = -I${HYPRE_PATH}/include ${FFLAGS_LAPACK}
FFLAGS_CUDA  = -I${CUDA_PATH}/include
FFLAGS_MAGMA = -I${MAGMA_PATH}/include
FFLAGS_OACC  = -fopenacc
FFLAGS_UNIFYFS = -I${UNIFYFS_ROOT}

OPT_FLAGS    = -g -Ofast -funroll-loops -fprefetch-loop-arrays
TEST_FLAGS   = -g -O3
DEBUG_FLAGS  = -g -Og

FFLAGS_OPT   = -c ${OPT_FLAGS}
FFLAGS_TEST  = -c ${TEST_FLAGS}
FFLAGS_DEBUG = -c ${DEBUG_FLAGS} -fcheck=bounds,do,mem,pointer -ffpe-trap=invalid,zero,overflow -fbacktrace

F90FLAGS     = -fdefault-real-8 -fdefault-double-8 -fimplicit-none -ffree-line-length-none -Wno-argument-mismatch -cpp
f90FLAGS     = ${F90FLAGS}
F77FLAGS     = -fdefault-real-8 -fdefault-double-8 -fimplicit-none -Wno-argument-mismatch -cpp
f77FLAGS     = ${F77FLAGS}

FFLAGS_AMREX = -I${AMREX_PATH}/include
CFLAGS_HDF5  = -I$(HDF5_PATH)/include ${PP}H5_USE_18_API
CFLAGS_NCMPI = -I$(NCMPI_PATH)/include
CFLAGS_HYPRE = -I${HYPRE_PATH}/include ${CFLAGS_LAPACK}
CFLAGS_CUDA  = -I${CUDA_PATH}/include
CFLAGS_MAGMA = -I${MAGMA_PATH}/include
CFLAGS_OACC  = -fopenacc
CFLAGS_UNIFYFS = -I${UNIFYFS_ROOT}

CFLAGS_OPT   = -c ${OPT_FLAGS}
CFLAGS_TEST  = -c ${TEST_FLAGS}
CFLAGS_DEBUG = -c ${DEBUG_FLAGS}
CFLAGS_AMREX = -I${AMREX_PATH}/include

CU_FLAGS     = -c -g -O2 -m64 -gencode arch=compute_70,code=sm_70

.SUFFIXES: .o .c .f .F .h .fh .F90 .f90 .cu

#----------------------------------------------------------------------------
# Linker flags
#----------------------------------------------------------------------------
LFLAGS_OPT   = ${OPT_FLAGS} -o
LFLAGS_TEST  = ${TEST_FLAGS} -o
LFLAGS_DEBUG = ${DEBUG_FLAGS} -o

#----------------------------------------------------------------------------
# Library specific linking
#
#  If a FLASH module has a 'LIBRARY xxx' line in its Config file, we need to
#  create a macro in this Makefile.h for LIB_xxx, which will be added to the
#  link line when FLASH is built.  This allows us to switch between different
#  (incompatible) libraries.  We also create a _OPT, _DEBUG, and _TEST
#  library macro to add any performance-minded libraries (like fast math),
#  depending on how FLASH was setup.
#----------------------------------------------------------------------------

LIB_MPI =

LIB_STDCXX = -lstdc++
LIB_AMREX = -L${AMREX_PATH}/lib -lamrex -lstdc++
LIB_BITTREE = 
LIB_HDF5  = -L${HDF5_PATH}/lib -lhdf5_fortran -lhdf5
LIB_NCMPI = -L${NCMPI_PATH}/lib -lpnetcdf
LIB_MATH  = ${LIB_ESSL}
LIB_HYPRE = -L${HYPRE_PATH}/lib -lHYPRE ${LIB_LAPACK}
LIB_CUDA  = -L${CUDA_PATH}/lib64 -lcusparse -lcusolver -lcublas -lcudart -lcuda
LIB_MAGMA = -L$(MAGMA_PATH)/lib -lmagma -Wl,-rpath,$(MAGMA_PATH)/lib
LIB_OACC  =
LIB_UNIFYFS = -L${UNIFYFS_ROOT} -lunifyfs_mpi_gotcha

LIB_OPT   =
LIB_DEBUG =
LIB_TEST  =

#----------------------------------------------------------------------------
# Additional machine-dependent object files
#
#  Add any machine specific files here -- they will be compiled and linked
#  when FLASH is built.
#----------------------------------------------------------------------------

MACHOBJ =

#----------------------------------------------------------------------------
# Additional commands
#----------------------------------------------------------------------------

MV = mv -f
AR = ar -r
RM = rm -f
CD = cd
RL = ranlib
ECHO = echo
AWK = awk
CAT = cat
