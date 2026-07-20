set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv)

set(CMAKE_C_COMPILER riscv-none-elf-gcc)
set(CMAKE_CXX_COMPILER riscv-none-elf-g++)
set(CMAKE_ASM_COMPILER riscv-none-elf-gcc)

set(C906_MARCH "rv64gc_xtheadba_xtheadbb_xtheadbs_xtheadcmo_xtheadcondmov_xtheadfmemidx_xtheadmac_xtheadmemidx_xtheadmempair_xtheadsync_zicsr_zifencei")

set(CMAKE_C_FLAGS "-mcpu=thead-c906 -march=${C906_MARCH} -mno-fence-tso -mabi=lp64d -mcmodel=medany -DCONFIG_64BIT -msmall-data-limit=0 -D__riscv_xtheadc -fno-strict-aliasing -O2 -g -ffunction-sections -fdata-sections" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=gnu++20" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "-mcpu=thead-c906 -march=${C906_MARCH} -mabi=lp64d -DCONFIG_64BIT" CACHE STRING "" FORCE)

set(CMAKE_EXE_LINKER_FLAGS "-mcpu=thead-c906 -march=${C906_MARCH} -mabi=lp64d -nostartfiles -Wl,--gc-sections -static --specs=nosys.specs -Wl,-u,_start" CACHE STRING "" FORCE)

# Ensure the find commands only look for target libraries
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
