//
// Created by marco.silipo on 28.08.2023.
//
#include "sqfvm_analyzer.hpp"

void sqfvm::language_server::analysis::sqfvm_analyzer::analyze() {
    auto path_info = m_runtime->fileio().get_info(m_file.path, {m_file.path, {}, {}});
    if (!path_info.has_value()) {
        report_diagnostic({
                                  .file_fk = m_file.id_pk,
                                  .severity = database::tables::t_diagnostic::severity_level::error,
                                  .message = "Failed to get path info for file: " + m_file.path,
                          });
        return;
    }
    auto &preprocessor = m_runtime->parser_preprocessor();
    auto casted = dynamic_cast<sqf::parser::preprocessor::impl_default *>(&preprocessor);
    if (casted == nullptr) {
        report_diagnostic({
                                  .file_fk = m_file.id_pk,
                                  .severity = database::tables::t_diagnostic::severity_level::error,
                                  .message = "Failed to cast preprocessor to default implementation. "
                                             "Please report this issue and rollback to a previous version of the language server.",
                          });
        return;
    }
    casted->macro_resolved([&](
            auto orig_start,
            auto orig_end,
            auto pp_start,
            auto pp_end,
            auto &runtime,
            auto &local_fileinfo,
            auto &original_fileinfo,
            auto &m,
            auto &param_map) {
        m_offset_pairs.push_back({
                                         .raw = orig_start,
                                         .preprocessed_offset = pp_start,
                                         .kind = offset_pair::kind::start,
                                 });
        m_offset_pairs.push_back({
                                         .raw = orig_end,
                                         .preprocessed_offset = pp_end,
                                         .kind = offset_pair::kind::end,
                                 });
        macro_resolved(orig_start, orig_end, pp_start, pp_end, runtime, local_fileinfo, original_fileinfo, m,
                       param_map);
    });
    casted->file_included([&](auto & included_fileinfo, auto & source_fileinfo) {
       file_included(included_fileinfo, source_fileinfo);
    });

    auto preprocessed_opt = preprocessor.preprocess(*m_runtime, m_text, {m_file.path, {}, {}});
    if (!preprocessed_opt.has_value()) {
        // Preprocessor already reported the error
        return;
    }
    m_preprocessed_text = preprocessed_opt.value();

    analyze(*m_runtime);
}
