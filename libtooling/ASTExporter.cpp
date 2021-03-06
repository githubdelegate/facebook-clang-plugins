/**
 * Copyright (c) 2014, Facebook, Inc.
 * Copyright (c) 2003-2014 University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * This file is distributed under the University of Illinois Open Source License.
 * See LLVM-LICENSE for details.
 *
 */

/**
 * Clang frontend plugin to export an AST of clang into Json and Yojson (and ultimately Biniou)
 * while conforming to the inlined ATD specifications.
 */

#include "ASTExporter.h"

//===----------------------------------------------------------------------===//
// ASTExporter Plugin Main
//===----------------------------------------------------------------------===//

namespace {

  using namespace ASTLib;
  using namespace ASTPluginLib;

  template <class ATDWriter=YojsonWriter>
  class ExporterASTConsumer : public ASTConsumer {
  private:
    ASTExporterOptions Options;
    raw_ostream &OS;

  public:
    ExporterASTConsumer(const CompilerInstance &CI,
                        std::unique_ptr<ASTExporterOptions> &&Opts,
                        raw_ostream &OS)
    : Options(std::move(*Opts)), OS(OS)
    { }

    virtual void HandleTranslationUnit(ASTContext &Context) {
      TranslationUnitDecl *D = Context.getTranslationUnitDecl();
      ASTExporter<ATDWriter> P(OS, Context, Options);
      P.dumpDecl(D);
    }
  };

}

typedef ASTPluginLib::SimplePluginASTAction<ExporterASTConsumer<JsonWriter>, ASTExporterOptions> JsonExporterASTAction;
typedef ASTPluginLib::SimplePluginASTAction<ExporterASTConsumer<YojsonWriter>, ASTExporterOptions> YojsonExporterASTAction;

static FrontendPluginRegistry::Add<JsonExporterASTAction>
X("JsonASTExporter", "Export the AST of source files into ATD-specified Json data");

static FrontendPluginRegistry::Add<YojsonExporterASTAction>
Y("YojsonASTExporter", "Export the AST of source files into ATD-specified Yojson data");
