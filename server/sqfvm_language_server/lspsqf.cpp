#include "lspsqf.hpp"
#include "migration/migrator.hpp"
#include "sqfanalyzer.hpp"
#include "repositories/t_files.hpp"
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
            for (auto& analyzer : m_analyzers)
            {
                if (analyzer->handles(file_path))
                {
                    auto last_write_time = iter->last_write_time();
                    uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(last_write_time.time_since_epoch()).count();
                    file f = file::get(m_db, file_path);
                    f.state = f.timestamp == timestamp ? file::estate::same : file::estate::differs;
                    file::set(m_db, f);
                }
            }
        }
    }

    // Handle all unset (Not-Found) files
    std::vector<file> files;
    file::all(m_db, files, file::unset);
    for (auto& f : files)
    {
        // ToDo: Handle removed stuff
    }

    // Remove all unset from database
    files.clear();
    file::drop_all(m_db, file::unset);

    // Handle all differs (changed) files
    file::all(m_db, files, file::differs);
    for (auto& f : files)
    {
        // ToDo: Handle changed stuff
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

sqfvm::lsp::lssqf::lssqf()
{
    m_analyzers.push_back(std::make_shared<sqfanalyzer>());
}
