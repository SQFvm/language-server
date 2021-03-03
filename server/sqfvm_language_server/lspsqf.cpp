#include "lspsqf.hpp"
#include "migration/migrator.hpp"
#include "sqfanalyzer.hpp"
#include "repositories/t_files.hpp"

#include "fileio/default.h"
#include "parser/sqf/sqf_parser.hpp"
#include "parser/preprocessor/default.h"
#include "parser/config/config_parser.hpp"
#include "operators/ops.h"


#include <string_view>
#include <unordered_map>
#include <vector>
#include <sstream>

using namespace std::string_view_literals;
using namespace sqfvm::lsp::repositories;

void sqfvm::lsp::lssqf::after_initialize(const::lsp::data::initialize_params& params)
{
    std::filesystem::path uri(params.rootUri.has_value() ? params.rootUri.value().full() : "./"sv);
    uri = std::filesystem::absolute(uri);
    m_folder = uri / ".vscode"sv / "sqfvm-lsp";
    m_db_path = m_folder / "sqlite3.db";

    // Handle SQLite3 database
    sqlite::result dbresult;
    if ((dbresult = m_db.open(m_db_path)) != sqlite::result::OK)
    {
        std::stringstream sstream;
        sstream << "Fatal error encountered while opening SQLite3 database file at '" << m_db_path << "': "
            << "[0x" << std::hex << static_cast<int>(dbresult) << "] " << m_db.last_error();
        window_logMessage(::lsp::data::message_type::Error, sstream.str());
        kill();
        return;
    }
    dbresult = sqfvm::lsp::migrator::migrate(m_db);
    if (dbresult != sqlite::result::OK)
    {
        std::stringstream sstream;
        sstream << "Fatal error encountered while migrating to the latest SQL schema: "
            << "[0x" << std::hex << static_cast<int>(dbresult) << "] " << m_db.last_error();
        window_logMessage(::lsp::data::message_type::Error, sstream.str());
        kill();
        return;
    }

    // Reset state of all files
    file::set_all_state(m_db, file::unset);

    // Mark all files according to their state
    for (auto& workspaceFolder : params.workspaceFolders.value())
    {
        std::filesystem::path workspace_path(workspaceFolder.uri.full());

        // Iterate over all files recursive
        std::filesystem::recursive_directory_iterator iter(workspace_path, std::filesystem::directory_options::skip_permission_denied);
        std::filesystem::recursive_directory_iterator iter_end;
        for (; iter != iter_end; iter++)
        {
            auto& file_path = iter->path();
            if (m_analyzer_factory.has(file_path.extension().string()))
            {
                auto last_write_time = iter->last_write_time();
                uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(last_write_time.time_since_epoch()).count();
                file f = file::get(m_db, file_path);
                f.state = f.timestamp == timestamp ? file::estate::same : file::estate::differs;
                file::set(m_db, f);
            }
        }
    }

    // Delete all unset (Not-Found) files.
    // Thanks to cascade-delete, old analytics results will be deleted too.
    std::vector<file> files;
    file::drop_all(m_db, file::unset);

    // Handle all differs (changed) files
    file::all(m_db, files, file::differs);
    for (auto& f : files)
    {
        auto analyzer = m_analyzer_factory.get(f.filepath.extension().string());
        if (analyzer.has_value())
        {
            auto document_ = sqf::fileio::disabled::read_file_from_disk(f.filepath.string());
            if (document_.has_value())  
            {
                // Drop File (to also remove all references via cascade)
                file::drop(m_db, f);
                
                // Analyze actual file
                analyzer.value()->analyze(m_runtime, *document_, f.filepath);

                // Push analytics results IF available
                file::set(m_db, f);
            }
            else
            {
                std::stringstream sstream;
                sstream << "Could not load file '" << f.filepath << "'. Did not update analytics results.";
                window_logMessage(::lsp::data::message_type::Warning, sstream.str());
            }
        }
        else
        {
            std::stringstream sstream;
            sstream << "Cannot find analyzer for '" << f.filepath << "'. Did not update analytics results.";
            window_logMessage(::lsp::data::message_type::Warning, sstream.str());
        }
    }

    // Calculate hints etc.
    // ToDo: Handle sending things to lsp client
}

void sqfvm::lsp::lssqf::on_workspace_didChangeConfiguration(const::lsp::data::did_change_configuration_params& params)
{
}

void sqfvm::lsp::lssqf::on_textDocument_didChange(const::lsp::data::did_change_text_document_params& params)
{
}

std::optional<std::vector<::lsp::data::folding_range>> sqfvm::lsp::lssqf::on_textDocument_foldingRange(const::lsp::data::folding_range_params& params)
{
    return std::optional<std::vector<::lsp::data::folding_range>>();
}

std::optional<::lsp::data::completion_list> sqfvm::lsp::lssqf::on_textDocument_completion(const::lsp::data::completion_params& params)
{
    return std::optional<::lsp::data::completion_list>();
}

sqfvm::lsp::lssqf::lssqf() : m_logger(), m_runtime(m_logger, {})
{
    // Setup SQF-VM
    m_runtime.fileio(std::make_unique<sqf::fileio::impl_default>(m_logger));
    m_runtime.parser_config(std::make_unique<sqf::parser::config::parser>(m_logger));
    m_runtime.parser_sqf(std::make_unique<sqf::parser::sqf::parser>(m_logger));
    m_runtime.parser_preprocessor(std::make_unique<sqf::parser::preprocessor::impl_default>(m_logger));
    sqf::operators::ops(m_runtime);

    // Setup analyzers
    m_analyzer_factory.set(".sqf", []() -> std::unique_ptr<analyzer> { return std::make_unique<sqfanalyzer>(); });
}
