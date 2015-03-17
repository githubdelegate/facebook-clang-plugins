/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <memory>
#include <stdlib.h>

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>

#include "FileServices.h"
#include "FileUtils.h"

namespace ASTPluginLib {

struct PluginASTOptionsBase {
  std::string inputFile;
  std::string outputFile;

  /* Will contain the current directory if PREPEND_CURRENT_DIR was specified.
   * The intention is to make file paths in the AST absolute if needed.
   */
  std::string basePath;

  /* Configure a second pass on file paths to make them relative to the repo root. */
  std::string repoRoot;
  /* Whether file paths not under the repo root should be kept or blanked. */
  bool keepExternalPaths = false;

  /* Deduplication service: whether certain files should be visited once. */
  std::unique_ptr<FileServices::DeduplicationService> deduplicationService;

  /* Translation service: whether certain copied source files should be translated back to the original name. */
  std::unique_ptr<FileServices::TranslationService> translationService;

  typedef std::unordered_map<std::string, std::string> argmap_t;

  static argmap_t makeMap(const std::vector<std::string> &args);

protected:
  static const std::string envPrefix;

  static bool loadString(const argmap_t &map, const char *key, std::string &val);

  static bool loadBool(const argmap_t &map, const char *key, bool &val);

public:
  void loadValuesFromEnvAndMap(const argmap_t map);

  void setOutputFile(const std::string &path);

  // This should be called last.
  // It will also finalize the output file in case of a pattern "%.bla".
  void setInputFile(const std::string &path);

  std::string normalizeSourcePath(std::string path) const;

};

template <class PluginASTOptions = PluginASTOptionsBase>
class SimplePluginASTActionBase : public clang::PluginASTAction {
protected:
  std::unique_ptr<PluginASTOptions> Options;

  virtual bool ParseArgs(const clang::CompilerInstance &CI,
                         const std::vector<std::string> &args_) {
    std::vector<std::string> args = args_;
    Options = std::unique_ptr<PluginASTOptions>(new PluginASTOptions());
    if (args.size() > 0) {
      Options->setOutputFile(args[0]);
      args.erase(args.begin());
    }
    Options->loadValuesFromEnvAndMap(PluginASTOptions::makeMap(args));
    return true;
  }

};

template <
  class T,
  class PluginASTOptions = PluginASTOptionsBase,
  bool Binary = 0,
  bool RemoveFileOnSignal = 1,
  bool UseTemporary = 1,
  bool CreateMissingDirectories = 0
>
class SimplePluginASTAction : public SimplePluginASTActionBase<PluginASTOptions> {
  typedef SimplePluginASTActionBase<PluginASTOptions> Parent;

protected:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InputFile) {
    Parent::Options->setInputFile(InputFile);

    llvm::raw_fd_ostream *OS =
      CI.createOutputFile(Parent::Options->outputFile,
                          Binary,
                          RemoveFileOnSignal,
                          "",
                          "",
                          UseTemporary,
                          CreateMissingDirectories);

    if (!OS) {
      return nullptr;
    }

    return std::unique_ptr<
    clang::ASTConsumer>(new T(CI, std::move(Parent::Options), *OS));
  }
};

template <
  class T,
  class PluginASTOptions = PluginASTOptionsBase
>
class NoOpenSimplePluginASTAction : public SimplePluginASTActionBase<PluginASTOptions> {
  typedef SimplePluginASTActionBase<PluginASTOptions> Parent;

protected:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef inputFile) {
    Parent::Options->setInputFile(inputFile);

    std::string outputFile = Parent::Options->outputFile;
    return std::unique_ptr<clang::ASTConsumer>(new T(CI, std::move(Parent::Options), outputFile));
  }
};

}
