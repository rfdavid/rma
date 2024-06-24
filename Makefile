#############################################################################
# Autoconf variables

# builddir: where to build the final artifacts
builddir := .
# objectdir: container of the objects
objectdir := objects

# Current position
srcdir := .
VPATH := .
top_srcdir := .

# Installation paths
prefix := /usr/local
exec_prefix := ${prefix}
libdir := ${exec_prefix}/lib

# Subdirectories
subdirs := @subdirs@ 

# Compiler programs & flags
# !!! It assumes that ${top_srcdir} has been defined in the container file !!!
AR := ar
AR_FLAGS := rcsPT
CC := clang
CXX := clang++ -std=gnu++20
CPPFLAGS :=  
EXTRA_CPPFLAGS :=  # extra flags defined by autoconf, similar to AM_CPPFLAGS
SRC_CPPFLAGS := -I./
ALL_CPPFLAGS := ${CPPFLAGS} ${EXTRA_CPPFLAGS} ${SRC_CPPFLAGS}
CFLAGS := -g -fno-limit-debug-info -fno-omit-frame-pointer -Wall -Wno-sign-compare -O0
EXTRA_CFLAGS :=  # extra flags defined by autoconf, similar to AM_CFLAGS
ALL_CFLAGS := ${ALL_CPPFLAGS} ${EXTRA_CFLAGS} ${CFLAGS}
CXXFLAGS := -g -fno-limit-debug-info -fno-omit-frame-pointer -Wall -Wno-sign-compare -Wno-overloaded-virtual -O0
EXTRA_CXXFLAGS :=  # extra flags defined by autoconf, similar to AM_CXXFLAGS
ALL_CXXFLAGS := ${ALL_CPPFLAGS} ${EXTRA_CXXFLAGS} ${CXXFLAGS}

# Linker
LIBS := -ldl -lpthread -latomic  
LDFLAGS := ${LIBS}

#############################################################################
# List of the sources to compile (except the main)

sources := \
	buffered_rewired_memory.cpp \
	configuration.cpp \
	console_arguments.cpp \
	cpu_topology.cpp \
	database.cpp \
	errorhandling.cpp \
	memory_pool.cpp \
	miscellaneous.cpp \
	profiler.cpp \
	rewired_memory.cpp \
	abtree/abtree.cpp \
	abtree/art.cpp \
	abtree/dense_array.cpp \
	abtree/stx-btree.cpp \
	distribution/apma_distributions.cpp \
	distribution/cbytearray.cpp \
	distribution/cbyteview.cpp \
	distribution/distribution.cpp \
	distribution/driver.cpp \
	distribution/factory.cpp \
	distribution/idls_distributions.cpp \
	distribution/random_permutation.cpp \
	distribution/sparse_uniform_distribution.cpp \
	distribution/uniform_distribution.cpp \
	distribution/zipf_distribution.cpp \
	pma/bulk_loading.cpp \
	pma/density_bounds.cpp \
	pma/driver.cpp \
	pma/experiment.cpp \
	pma/external.cpp \
	pma/factory.cpp \
	pma/interface.cpp \
	pma/adaptive/basic/apma_baseline.cpp \
	pma/adaptive/bh07_v2/adaptive_rebalancing.cpp \
	pma/adaptive/bh07_v2/packed_memory_array.cpp \
	pma/adaptive/bh07_v2/predictor.cpp \
	pma/adaptive/bh07_v2/spread_with_rewiring.cpp \
	pma/adaptive/int1/adaptive_rebalancing.cpp \
	pma/adaptive/int1/detector.cpp \
	pma/adaptive/int1/iterator.cpp \
	pma/adaptive/int1/knobs.cpp \
	pma/adaptive/int1/move_detector_info.cpp \
	pma/adaptive/int1/packed_memory_array.cpp \
	pma/adaptive/int1/partition.cpp \
	pma/adaptive/int1/rebalancing_profiler.cpp \
	pma/adaptive/int1/sum.cpp \
	pma/adaptive/int1/static_abtree.cpp \
	pma/adaptive/int1/storage.cpp \
	pma/adaptive/int1/weights.cpp \
	pma/adaptive/int2/adaptive_rebalancing.cpp \
	pma/adaptive/int2/iterator.cpp \
	pma/adaptive/int2/move_detector_info.cpp \
	pma/adaptive/int2/packed_memory_array.cpp \
	pma/adaptive/int2/spread_with_rewiring.cpp \
	pma/adaptive/int2/storage.cpp \
	pma/adaptive/int2/sum.cpp \
	pma/adaptive/int2/weights.cpp \
	pma/adaptive/int3/adaptive_rebalancing.cpp \
	pma/adaptive/int3/iterator.cpp \
	pma/adaptive/int3/move_detector_info.cpp \
	pma/adaptive/int3/packed_memory_array.cpp \
	pma/adaptive/int3/spread_with_rewiring.cpp \
	pma/adaptive/int3/storage.cpp \
	pma/adaptive/int3/sum.cpp \
	pma/adaptive/int3/weights.cpp \
	pma/btree/btreepma_v2.cpp \
	pma/btree/btreepma_v4.cpp \
	pma/btree/btreepmacc5.cpp \
	pma/btree/btreepmacc7.cpp \
	pma/btree/08/iterator.cpp \
	pma/btree/08/packed_memory_array.cpp \
	pma/btree/08/spread_with_rewiring.cpp \
	pma/btree/08/storage.cpp \
	pma/experiments/aging.cpp \
	pma/experiments/bandwidth_idls.cpp \
	pma/experiments/bulk_loading.cpp \
	pma/experiments/idls.cpp \
	pma/experiments/insert_lookup.cpp \
	pma/experiments/range_query.cpp \
	pma/experiments/step_idls.cpp \
	pma/experiments/step_insert_lookup.cpp \
	pma/experiments/step_insert_scan.cpp \
	pma/external/dfr/dfr.cpp \
	pma/external/iejoin/khayyat.cpp \
	pma/external/montes/pma.c \
	pma/external/raizes/pkd_mem_arr.c \
	pma/external/sha/pma.cpp \
	pma/external/drui/pma_index.cpp \
	pma/generic/static_index.cpp \
	pma/sequential/pma_v4.cpp \
	third-party/art/Tree.cpp \
	third-party/sqlite3/sqlite3.c \
	third-party/zipf/genzipf.cpp
	
# Include INRIA sources (if available)
ifneq (,$(findstring -DHAVE_PMA_DFR,${ALL_CPPFLAGS}))
	sources += $(addprefix third-party/pma_dfr_inria/, pma_batch.cpp pma_bender.cpp pma_common.cpp pma_iter.cpp pma_utils.cpp)
endif
	
srcmain := main.cpp
srcaging := aging_generate.cpp

#############################################################################
# The executables to create
artifact_main := pmacomp
artifact_aging := aging_generate

#############################################################################
# Helper variables
makedepend_c = @$(CC) -MM $(ALL_CFLAGS) -MP -MT $@ -MF $(basename $@).d $<
makedepend_cxx = @$(CXX) -MM $(ALL_CXXFLAGS) -MP -MT $@ -MF $(basename $@).d $<
# Library objects
objectdirs := $(patsubst %./, %, $(sort $(addprefix ${objectdir}/, $(dir ${sources} ${srcmain}))))
objects_c := $(addprefix ${objectdir}/, $(patsubst %.c, %.o, $(filter %.c, ${sources})))
objects_cxx := $(addprefix ${objectdir}/, $(patsubst %.cpp, %.o, $(filter %.cpp, ${sources})))
objects := ${objects_c} ${objects_cxx}
# Main object
objmain := $(addprefix ${objectdir}/, $(patsubst %.cpp, %.o, ${srcmain}))
objects_cxx += ${objmain}
# Aging generator object
objaging := $(addprefix ${objectdir}/, $(patsubst %.cpp, %.o, ${srcaging}))
objects_cxx += ${objaging}

# Support for tcmalloc
ifneq (,$(findstring gperftools,${subdirs}))
	tcmalloc := third-party/gperftools-2.7/lib/libtcmalloc_minimal.so
endif

liblink = -L${builddir} -l$(patsubst lib%,%, $(basename ${library})) # recursive
library := libcommon.a

.DEFAULT_GOAL = all
.PHONY: all

all: Makefile ${builddir}/${artifact_main} ${builddir}/${artifact_aging}

#############################################################################
# Artifacts to build
${builddir}/${artifact_main}: ${objmain} ${library} | ${builddir}
	${CXX} $< $(liblink) ${LDFLAGS} -o $@

${builddir}/${artifact_aging}: ${objaging} ${library} | ${builddir}
	${CXX} $< $(liblink) ${LDFLAGS} -o $@

#############################################################################
# Tests (it needs to be refactored!)
testfolder := tests
testsources := $(notdir $(wildcard ${srcdir}/${testfolder}/*.cpp))
testobjdir := ${objectdir}/${testfolder}/
testbindir := $(abspath ${builddir}/${testfolder})
objectdirs += ${testobjdir}
objects_cxx += $(addprefix ${testobjdir}, $(patsubst %.cpp, %.o, ${testsources}))
testartifacts := $(addprefix tests/, $(basename ${testsources}))
testignored := "test_reddragon_impl1" # space separated list

.PHONY: check
check: ${testartifacts}
	success=1; \
	for f in `ls ${testbindir}`; do \
		f="${testbindir}/$$f"; \
		b=`basename $$f`; \
		for igntest in "${testignored}"; do \
			igntest="$$(echo -e "$${igntest}" | tr -d '[:space:]')"; \
			if [ x"$$igntest" == x"$$b" ]; then \
				echo "Test blacklisted and ignored: $$f"; \
				continue 2; \
			fi; \
		done; \
		echo "> Executing $$f ..."; \
		if [ -x "$$f" ]; then \
			$$f; \
			if [ "$$?" -ne 0 ]; then \
				echo "Test $$b failed. Avoiding to execute further tests."; \
				success=0; \
				break; \
			fi; \
		fi; \
	done; \
	if [ $$success -eq 1 ]; then \
		echo "!!! All tests passed !!!"; \
	fi;

${testartifacts}: ${library}
${testartifacts}: % : ${objectdir}/%.o |${testbindir}
	${CXX} $< $(liblink) ${LDFLAGS} -o $@
	
#############################################################################
# Compiling the objects
${library}: ${objects} ${tcmalloc}
	$(AR) $(AR_FLAGS) $@ $?

# Objects from C files
${objects_c} : ${objectdir}/%.o : %.c | ${objectdirs}
	${makedepend_c}
	${CC} -c ${ALL_CFLAGS} $< -o $@

# Objects from C++ files
${objects_cxx}: ${objectdir}/%.o : %.cpp | ${objectdirs}
	${makedepend_cxx}
	$(CXX) -c $(ALL_CXXFLAGS) $< -o $@

# Create the build directories
${builddir} ${testbindir} ${objectdirs}:
	mkdir -pv $@
	
# Wrapped projects
${subdirs}:
	$(MAKE) -C $@
.PHONY: ${subdirs}	

#############################################################################
# Support for tcmalloc	
${tcmalloc}: third-party/gperftools-2.7
	$(MAKE) -C $< install
	
#############################################################################
# Remove everything from the current build
.PHONY: clean
clean:
	rm -rf ${builddir}/${artifact_main}
	rm -rf ${builddir}/${artifact_aging}
	rm -rf ${builddir}/${library}
	rm -rf ${builddir}/${objectdir}
	rm -rf ${testbindir}
	
#############################################################################
# Regenerate the Makefile when the configuration has been changed
$(srcdir)/configure: configure.ac aclocal.m4
	cd '$(srcdir)' && autoconf
	
config.status: configure
	./config.status --recheck
	
Makefile: Makefile.in config.status
	./config.status
	
#############################################################################
# Dependencies to update the translation units if a header has been altered
-include ${objects:.o=.d}
