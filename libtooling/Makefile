# Copyright (c) 2014, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.

.PHONY: clean all test all_ast_samples

LEVEL=..

HEADERS+=SimplePluginASTAction.h

# ASTExporter
HEADERS+=atdlib/ATDWriter.h
OBJS+=ASTExporter.o

# Json
PLUGINS+=JsonASTExporter
EXTS+=.json

# Yojson
PLUGINS+=YojsonASTExporter
EXTS+=.yjson

all: build/FacebookClangPlugin.dylib

# hook to optional external sources
include $(LEVEL)/Makefile.extra
ifneq "$(EXTRA_DIR)" ""
include $(EXTRA_DIR)/Makefile.include
endif

include $(LEVEL)/Makefile.common

CLANG_FRONTEND=$(CLANG) -fsyntax-only -Xclang -load -Xclang $(shell pwd)/build/FacebookClangPlugin.dylib

build/FacebookClangPlugin.dylib: $(OBJS:%=build/%) $(HEADERS)
	@mkdir -p build
	$(CXX) $(LDFLAGS) -o $@ $(OBJS:%=build/%) -lz -lpthread -lm

TEST_DIRS=tests
ifneq "$(EXTRA_DIR)" ""
TEST_DIRS+=$(EXTRA_DIR)/tests
endif

OUT_TEST_FILES=${TEST_DIRS:%=%/*/*.out}

# To make sharing of test files easier, each source file should be
# found either in 'tests' or '$(EXTRA_DIR)/tests'. A plugin will only
# use the source files for which a .exp file exists in the
# corresponding subdirectory.
ifneq "$(EXTRA_DIR)" ""
EXPFILES_FORMULA=tests/$$P/*.exp "$(EXTRA_DIR)"/tests/$$P/*.exp
SRCFILE_FORMULA=$$(ls {tests,"$(EXTRA_DIR)"}/$$(basename $$TEST) 2>/dev/null || true)
FILTERFILE_FORMULA=$$(ls {tests,"$(EXTRA_DIR)"}/$${P}/filter.sh 2>/dev/null || true)
else
EXPFILES_FORMULA=tests/$$P/*.exp
SRCFILE_FORMULA=tests/$$(basename $$TEST)
FILTERFILE_FORMULA=tests/$${P}/filter.sh
endif

test: build/FacebookClangPlugin.dylib
	@for P in $(PLUGINS); do                                                        \
	   echo "-- $$P --";                                                            \
	   for EXPFILE in $(EXPFILES_FORMULA); do                                       \
	     TEST=$${EXPFILE%.exp};                                                     \
	     $(RUNTEST) "$$TEST" $(FILTERFILE_FORMULA)                                  \
	       $(CLANG_FRONTEND) -Xclang -plugin -Xclang $$P -c $(SRCFILE_FORMULA);     \
	   done;                                                                        \
	done
	@if [ ! $$KEEP_TEST_OUTPUTS ]; then rm -f $(OUT_TEST_FILES); fi

record-test-outputs:
	@make DEBUG=1 KEEP_TEST_OUTPUTS=1 test || true
	@for F in $(OUT_TEST_FILES); do cp $$F $${F%.out}.exp; done
	@rm -f $(OUT_TEST_FILES)

clean:
	@rm -rf build/* $(OUT_TEST_FILES)

# -- AST samples for specific end-to-end tests --

REGULAR_SOURCES=$(wildcard tests/*.m) $(wildcard tests/*.c)
AST_SAMPLE_FILES=ASTExporter.cpp $(REGULAR_SOURCES:tests/%=%)
all_ast_samples: $(AST_SAMPLE_FILES:%=build/ast_samples/%.json.gz) $(AST_SAMPLE_FILES:%=build/ast_samples/%.yjson.gz)

# dump samples files in Yojson using ASTExporter.cpp
YJ_DUMPER_ARGS=-Xclang -plugin -Xclang YojsonASTExporter -Xclang -plugin-arg-YojsonASTExporter -Xclang

build/ast_samples/%.cpp.yjson: %.cpp build/FacebookClangPlugin.dylib
	@mkdir -p build/ast_samples
	@$(CLANG_FRONTEND) $(CFLAGS) -Wno-ignored-qualifiers -I. $(YJ_DUMPER_ARGS) $@ -c $<

build/ast_samples/%.c.yjson: tests/%.c build/FacebookClangPlugin.dylib
	@mkdir -p build/ast_samples
	@$(CLANG_FRONTEND) $(YJ_DUMPER_ARGS) $@ -c $<

build/ast_samples/%.m.yjson: tests/%.m build/FacebookClangPlugin.dylib
	@mkdir -p build/ast_samples
	@$(CLANG_FRONTEND) $(YJ_DUMPER_ARGS) $@ -c $<

# dump sample files in Yojson using ASTExporter.cpp
J_DUMPER_ARGS=-Xclang -plugin -Xclang JsonASTExporter -Xclang -plugin-arg-JsonASTExporter -Xclang

build/ast_samples/%.cpp.json: %.cpp build/FacebookClangPlugin.dylib
	@mkdir -p build/ast_samples
	@$(CLANG_FRONTEND) $(CFLAGS) -Wno-ignored-qualifiers -I. $(J_DUMPER_ARGS) $@ -c $<

build/ast_samples/%.c.json: tests/%.c build/FacebookClangPlugin.dylib
	@mkdir -p build/ast_samples
	@$(CLANG_FRONTEND) $(J_DUMPER_ARGS) $@ -c $<

build/ast_samples/%.m.json: tests/%.m build/FacebookClangPlugin.dylib
	@mkdir -p build/ast_samples
	@$(CLANG_FRONTEND) $(J_DUMPER_ARGS) $@ -c $<

build/ast_samples/%.gz: build/ast_samples/%
	@gzip -k $<

# generate a preprocessed version of ASTExporter.cpp where ATD directives can be read
# we do not include the C/C++ headers to avoid parsing issues with the 'traditional' cpp (needed for precisely expanding our /// comments)
build/ASTExporter.cpp.p: ASTExporter.cpp
	@mkdir -p build
	@cat $< | grep -v '#include *["<][^.]*\(\.h\)\?[">]' | cpp -w -P -I$(CLANG_PREFIX)/include > build/ASTExporter.cpp.p