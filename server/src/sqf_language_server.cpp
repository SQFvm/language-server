#include "sqf_language_server.h"

#include <runtime/runtime.h>
#include <runtime/d_string.h>
#include <runtime/d_code.h>
#include <operators/ops.h>
#include <runtime/util.h>

#include <parser/config/config_parser.hpp>
#include <parser/sqf/sqf_parser.hpp>
#include <parser/preprocessor/default.h>
#include <fileio/default.h>

#include <sqc/sqc_parser.h>
#include <fstream>

#include <thread>

void sqf_language_server::scan_documents_recursive_at(std::string directory)
{
    std::filesystem::recursive_directory_iterator dir_end;
    std::vector<std::filesystem::path> files;
    for (std::filesystem::recursive_directory_iterator it(directory, std::filesystem::directory_options::skip_permission_denied); it != dir_end; it++)
    {
        auto path = it->path();
        if (it->is_directory() || !path.has_extension())
        {
            continue;
        }
        else
        {
            files.push_back(path);
        }
    }

    // Lazy "Max-Files-per-Thread"
    size_t files_per_thread = 0;
    size_t thread_count;
    do
    {
        files_per_thread += 20;
        thread_count = files.size() / files_per_thread;
    } while (thread_count > 12);
    {
        std::stringstream sstream;
        sstream << "Launching " << thread_count << " worker threads to handle parsing the whole project.";
        window_logMessage(lsp::data::message_type::Log, sstream.str());
    }
    std::vector<std::thread> threads;
    auto start = files.begin();
    // First round, Globals might not be known here correctly
    for (size_t i = 0; i < thread_count + 1; i++)
    {
        const auto localstart = start;
        start += i == thread_count ? files.size() % files_per_thread : files_per_thread;
        const auto localend = start;
        threads.emplace_back([this, localstart, localend, i]()
            {
                size_t files = localend - localstart;
                for (auto it = localstart; it < localend; ++it)
                {
                    auto path = *it;
                    auto uri = sanitize_to_uri(path.string());
                    auto fpath = sanitize_to_string(uri);
                    if (path.extension() == ".sqf")
                    {
                        text_documents[fpath] = { *this, sqfvm, fpath, text_document::document_type::SQF };
                        std::stringstream sstream;
                        sstream << "[WORKER-" << i << "][" << std::setw(3) << (it - localstart) << "/" << std::setw(3) << files << "] Analyzing " << fpath << " ... ";
                        window_logMessage(lsp::data::message_type::Log, sstream.str());
                        text_documents[fpath].analyze(*this, sqfvm, {});
                    }
                    else
                    {
                        std::stringstream sstream;
                        sstream << "[WORKER-" << i << "][" << std::setw(3) << (it - localstart) << "/" << std::setw(3) << files << "] Skipping " << fpath << ".";
                        window_logMessage(lsp::data::message_type::Log, sstream.str());
                    }
                }
            });
    }
    for (size_t i = 0; i < threads.size(); i++)
    {
        if (threads[i].joinable())
        {
            threads[i].join();
        }
    }
}

void sqf_language_server::after_initialize(const lsp::data::initialize_params& params)
{
    // Prepare sqfvm
    sqfvm.fileio(std::make_unique<sqf::fileio::impl_default>(logger));
    sqfvm.parser_config(std::make_unique<sqf::parser::config::parser>(logger));
    sqfvm.parser_preprocessor(std::make_unique<sqf::parser::preprocessor::impl_default>(logger));
    sqfvm.parser_sqf(std::make_unique<sqf::parser::sqf::parser>(logger));
    sqf::operators::ops(sqfvm);

    // Setup Pathing & Parse every file inside the workspace
    if (params.workspaceFolders.has_value())
    {
        for (auto workspaceFolder : params.workspaceFolders.value())
        {
            auto workspacePath = sanitize_to_string(workspaceFolder.uri);
            m_workspace_folders.push_back(workspacePath);
            if (!std::filesystem::exists(workspacePath))
            {
                std::stringstream sstream;
                sstream << "Cannot analyze workspace folder " << workspacePath << " as it is not existing.";
                window_logMessage(lsp::data::message_type::Error, sstream.str());
                continue;
            }
            {
                std::stringstream sstream;
                sstream << "Mapping " << workspacePath << " onto '/'";
                window_logMessage(lsp::data::message_type::Log, sstream.str());
                sqfvm.fileio().add_mapping(workspacePath, "/");
            }
            std::filesystem::recursive_directory_iterator dir_end;
            size_t sqf_files_total = 0;
            // Read-In all $PBOPREFIX$ files
            for (std::filesystem::recursive_directory_iterator it(workspacePath, std::filesystem::directory_options::skip_permission_denied); it != dir_end; it++)
            {
                auto path = it->path();
                if (!it->is_directory() && !path.has_extension() && path.filename() == "$PBOPREFIX$")
                {
                    // ToDo: If modified, remove existing mapping and reparse as whole. (or message the user that a reload is required)
                    // ToDo: If created, add mapping and reparse as whole. (or message the user that a reload is required)
                    auto pboprefix_path = path.parent_path().string();
                    auto pboprefix_contents_o = sqfvm.fileio().read_file_from_disk(path.string());
                    if (pboprefix_contents_o.has_value())
                    {
                        auto pboprefix_contents = pboprefix_contents_o.value();
                        add_mapping_to_sqf_vm(pboprefix_path, pboprefix_contents);
                    }
                    else
                    {
                        std::stringstream sstream;
                        sstream << "Failed to read " << pboprefix_path << ". Skipping.";
                        window_logMessage(lsp::data::message_type::Error, sstream.str());
                    }
                }
                else if (path.has_extension() && path.extension() == ".sqf")
                {
                    sqf_files_total++;
                }
            }
            // Scan twice to "fix" undeclared gloabls
            scan_documents_recursive_at(workspacePath);
            scan_documents_recursive_at(workspacePath);
        }
    }
    window_logMessage(lsp::data::message_type::Log, "SQF-VM Language Server is ready.");
}

void sqf_language_server::on_workspace_didChangeConfiguration(const lsp::data::did_change_configuration_params& params)
{
    if (params.settings.has_value())
    {
        m_sqc_support = (*params.settings)["sqfVmLanguageServer"]["ls"]["sqcSupport"];
        if (m_sqc_support)
        {
            window_logMessage(lsp::data::message_type::Log, "SQC Auto-Compilation support enabled.");
        }

        logger.setEnabled(loglevel::verbose, (*params.settings)["sqfVmLanguageServer"]["ls"]["logLevel"]["verbose"]);
        logger.setEnabled(loglevel::trace, (*params.settings)["sqfVmLanguageServer"]["ls"]["logLevel"]["trace"]);


        if (m_read_config) { return; }
        m_read_config = true;
        auto res = (*params.settings)["sqfVmLanguageServer"]["ls"]["additionalMappings"];
        if (res.is_array())
        {
            for (auto el : res.items())
            {
                if (el.value().is_string())
                {
                    add_mapping_to_sqf_vm(el.value().get<std::string>(), el.key());
                }
            }
        }
    }
}

void sqf_language_server::on_textDocument_didChange(const lsp::data::did_change_text_document_params& params)
{
    auto& doc = get_or_create(params.textDocument.uri);
    auto path = std::filesystem::path(sanitize_to_string(params.textDocument.uri)).lexically_normal();
    if (path.extension().string() == ".sqc")
    {
        if (!sqc_support()) { return; }
        {
            std::stringstream sstream;
            sstream << "Compiling file '" << path.string() << "'." << std::endl;
            window_logMessage(lsp::data::message_type::Info, sstream.str());
        }
        doc.diagnostics.diagnostics.clear();
        auto preprocessed = sqfvm.parser_preprocessor().preprocess(sqfvm, params.contentChanges.front().text, { path.string(), {} });
        if (preprocessed.has_value())
        {
            sqf::sqc::parser sqcParser(logger);
            auto set = sqcParser.parse(sqfvm, preprocessed.value(), { path.string(), {} });

            if (set.has_value())
            {
                path.replace_extension(".sqf");
                std::ofstream out_file(path, std::ios_base::trunc);
                if (out_file.good())
                {
                    auto dcode = std::make_shared<sqf::types::d_code>(*set);
                    auto str = dcode->to_string_sqf();
                    if (str.length() > 2)
                    {
                        std::string_view view(str.data() + 1, str.length() - 2);
                        view = sqf::runtime::util::trim(view, " \t\r\n");
                        out_file << view;
                    }
                }
                else
                {
                    std::stringstream sstream;
                    sstream << "Failed to open file '" << path.string() << "' for writing." << std::endl;
                    window_logMessage(lsp::data::message_type::Error, sstream.str());
                }
            }
            else
            {
                std::stringstream sstream;
                sstream << "Failed to parse file '" << path.string() << "'." << std::endl;
                window_logMessage(lsp::data::message_type::Error, sstream.str());
            }
        }
        else
        {
            std::stringstream sstream;
            sstream << "Failed to preprocess file '" << path.string() << "'." << std::endl;
            window_logMessage(lsp::data::message_type::Error, sstream.str());
        }
        textDocument_publishDiagnostics(doc.diagnostics);
    }
    else
    {
        doc.analyze(*this, sqfvm, params.contentChanges.front().text);
    }
}

std::optional<std::vector<lsp::data::folding_range>> sqf_language_server::on_textDocument_foldingRange(const lsp::data::folding_range_params& params)
{
    auto& doc = get_or_create(params.textDocument.uri);
    return doc.foldings();
}

std::optional<lsp::data::completion_list> sqf_language_server::on_textDocument_completion(const lsp::data::completion_params& params)
{
    
    // Get navigation token
    auto& doc = get_or_create(params.textDocument.uri);
    auto nav_o = doc.navigate(params.position.line, params.position.character);
    if (!nav_o.has_value()) { /* ToDo: Return default completion_list instead of empty results */ return {}; }

    // ToDo: Handle the different astnode tokens for extended completion (eg. for `addAction [@p1, @p2, @p3, @p4, ...]` the different parameters inside of the list)

    // ToDo: Return slide of default completion_list instead of empty results
    return {};
}


text_document& sqf_language_server::get_or_create(lsp::data::uri uri)
{
    using namespace std::string_view_literals;
    auto fpath = sanitize_to_string(uri);
    std::filesystem::path path(fpath);

    // Check if file already exists
    auto findRes = text_documents.find(fpath);
    if (findRes != text_documents.end())
    {
        // Only perform analysis again on the text provided by params
        return findRes->second;
    }
    else
    {
        auto ext = path.extension();
        
        // Create file at fpath only if file is an actual sqf file and perform analysis.
        text_documents[fpath] = {
            *this,
            sqfvm,
            fpath,
            ext == ".sqc" ? text_document::document_type::SQC
            : text_document::document_type::SQF };
        return text_documents[fpath];
    }
}

void sqf_language_server::add_mapping_to_sqf_vm(std::string phys, std::string virt)
{
    using namespace std::string_view_literals;
    {
        std::replace(phys.begin(), phys.end(), '\\', '/');
        phys = std::string(sqf::runtime::util::trim(phys, " \t\r\n"));
    }
    {
        virt = virt[0] != '/' ? "/" + virt : virt;
        std::replace(virt.begin(), virt.end(), '\\', '/');
        virt = std::string(sqf::runtime::util::trim(virt, " \t\r\n"));
    }

    std::string msg;
    msg.reserve(
        "Mapping "sv.length() +
        phys.length() +
        " onto '"sv.length() +
        virt.length() +
        "'"sv.length()
    );
    msg.append("Mapping "sv);
    msg.append(phys);
    msg.append(" onto '");
    msg.append(virt);
    msg.append("'"sv);
    window_logMessage(lsp::data::message_type::Log, msg);
    sqfvm.fileio().add_mapping(phys, virt);
}
