#include "languageserver.h"


#include <runtime/runtime.h>
#include <operators/ops_config.h>
#include <operators/ops_diag.h>
#include <operators/ops_generic.h>
#include <operators/ops_group.h>
#include <operators/ops_logic.h>
#include <operators/ops_markers.h>
#include <operators/ops_math.h>
#include <operators/ops_namespace.h>
#include <operators/ops_object.h>
#include <operators/ops_sqfvm.h>
#include <operators/ops_string.h>

#include <parser/config/default.h>
#include <parser/sqf/default.h>
#include <parser/preprocessor/default.h>
#include <fileio/default.h>

#include <unordered_map>
#include <filesystem>

class sqf_language_server : public lsp::server
{
public:
	class QueueLogger : public Logger {
	public:
		QueueLogger() : Logger() {}
		std::unordered_map<std::string, lsp::data::publish_diagnostics_params> messages;

		virtual void log(const LogMessageBase& base) override
		{
			auto& message = dynamic_cast<const logmessage::RuntimeLogMessageBase&>(base);
			if (message == nullptr)
			{
				return;
			}

			auto location = message.location();
			auto findRes = messages.find(location.path);
			if (findRes == messages.end())
			{
				lsp::data::publish_diagnostics_params p;
				auto path = std::filesystem::path(location.path).lexically_normal();
				auto str = path.string();
				std::replace(str.begin(), str.end(), '\\', '/');
				p.uri = "file:///" + str;
				messages[location.path] = p;
			}

			lsp::data::publish_diagnostics_params& params = messages[location.path];

			lsp::data::diagnostics msg;
			msg.range.start.line = location.line;
			msg.range.start.character = location.col;
			msg.range.end.line = location.line;
			msg.range.end.character = location.col;


			

			msg.message = message.formatMessage();
			msg.source = "SQF-VM";

			switch (message.getLevel())
			{
			case loglevel::fatal:
			case loglevel::error:
				msg.severity = lsp::data::diagnostic_severity::Error;
				break;
			case loglevel::warning:
				msg.severity = lsp::data::diagnostic_severity::Warning;
				break;
			case loglevel::info:
				msg.severity = lsp::data::diagnostic_severity::Information;
				break;
			case loglevel::verbose:
			case loglevel::trace:
			default:
				msg.severity = lsp::data::diagnostic_severity::Hint;
				break;
			}
			params.diagnostics.push_back(msg);
		}
		void report(sqf_language_server& ls)
		{
			for (auto res : messages)
			{
				if (res.second.diagnostics.empty())
				{
					continue;
				}

				ls.textDocument_publishDiagnostics(res.second);
				res.second.diagnostics.clear();
			}
		}
	};
	class text_document
	{
	private:
		std::string m_path;
		std::string m_contents;
		sqf::parser::sqf::impl_default::astnode m_root_ast;
		std::vector<lsp::data::folding_range> m_foldings;

		void recalculate_ast(sqf::runtime::runtime& sqfvm)
		{
			auto parser = dynamic_cast<sqf::parser::sqf::impl_default&>(sqfvm.parser_sqf());
			bool errflag = false;

			auto preprocessed = sqfvm.parser_preprocessor().preprocess(sqfvm, { m_path, {} });
			if (preprocessed.has_value())
			{
				m_contents = preprocessed.value();
				m_root_ast = parser.get_ast(sqfvm, m_contents, { m_path, {} }, &errflag);
				if (errflag)
				{
					m_root_ast = {};
				}
			}
			else
			{
				m_root_ast = {};
			}
		}

		void recalculate_foldings_recursive(sqf::runtime::runtime& sqfvm, sqf::parser::sqf::impl_default::astnode& current)
		{
			switch (current.kind)
			{
			case sqf::parser::sqf::impl_default::nodetype::ARRAY:
			case sqf::parser::sqf::impl_default::nodetype::CODE:
			{
				// todo: find a way to force vscode into using offsets instead of lines
				lsp::data::folding_range frange;
				frange.startCharacter = current.file_offset;
				frange.startLine = current.line - 1; // lines start at 0
				frange.endCharacter = current.file_offset + current.length;

				// find current nodes, last child in tree and set its line as end.
				sqf::parser::sqf::impl_default::astnode* prev = &current;
				sqf::parser::sqf::impl_default::astnode* node = &current;
				while (node)
				{
					prev = node;
					node = node->children.empty() ? nullptr : &node->children.back();
				}
				frange.endLine = prev->line;
				m_foldings.push_back(frange);
			} break;
			}
			for (auto child : current.children)
			{
				recalculate_foldings_recursive(sqfvm, child);
			}
		}
		void recalculate_foldings(sqf::runtime::runtime& sqfvm)
		{
			m_foldings.clear();
			recalculate_foldings_recursive(sqfvm, m_root_ast);
		}
	public:
		text_document() {}
		text_document(sqf::runtime::runtime& sqfvm, std::string path) : m_path(path)
		{
			reread(sqfvm);
		}

		std::string_view contents() const { return m_contents; }
		void reread(sqf::runtime::runtime& sqfvm)
		{
			recalculate_ast(sqfvm);
			recalculate_foldings(sqfvm);
		}

		std::vector<lsp::data::folding_range>& foldings() { return m_foldings; }
	};
protected:

	// Inherited via server
	virtual lsp::data::initialize_result on_initialize(const lsp::data::initialize_params& params) override
	{
		client = params;
		// Prepare server capabilities
		lsp::data::initialize_result res;
		res.serverInfo = lsp::data::initialize_result::server_info{};
		res.serverInfo->name = "SQF-VM Language Server";
		res.serverInfo->version = "0.1.0";
		res.capabilities.colorProvider = lsp::data::initialize_result::server_capabilities::document_color_registration_options{ };
		res.capabilities.colorProvider->documentSelector = lsp::data::document_filter{ };
		res.capabilities.colorProvider->documentSelector->language = "sqf";
		res.capabilities.textDocumentSync = lsp::data::initialize_result::server_capabilities::text_document_sync_options{};
		res.capabilities.textDocumentSync->change = lsp::data::text_document_sync_kind::Full;
		res.capabilities.textDocumentSync->openClose = true;
		res.capabilities.foldingRangeProvider = lsp::data::initialize_result::server_capabilities::folding_range_registration_options{};
		res.capabilities.foldingRangeProvider->documentSelector = lsp::data::document_filter{ };
		res.capabilities.foldingRangeProvider->documentSelector->language = "sqf";
		res.capabilities.completionProvider = lsp::data::initialize_result::server_capabilities::completion_options{};

		// prepare sqfvm
		sqfvm.fileio(std::make_unique<sqf::fileio::impl_default>());
		sqfvm.parser_config(std::make_unique<sqf::parser::config::impl_default>(logger));
		sqfvm.parser_preprocessor(std::make_unique<sqf::parser::preprocessor::impl_default>(logger));
		sqfvm.parser_sqf(std::make_unique<sqf::parser::sqf::impl_default>(logger));
		sqf::operators::ops_config(sqfvm);
		sqf::operators::ops_diag(sqfvm);
		sqf::operators::ops_generic(sqfvm);
		sqf::operators::ops_group(sqfvm);
		sqf::operators::ops_logic(sqfvm);
		sqf::operators::ops_markers(sqfvm);
		sqf::operators::ops_math(sqfvm);
		sqf::operators::ops_namespace(sqfvm);
		sqf::operators::ops_object(sqfvm);
		sqf::operators::ops_sqfvm(sqfvm);
		sqf::operators::ops_string(sqfvm);

		return res;
	}
	virtual void on_shutdown() override {}

	// After Initialize
	virtual void after_initialize(const lsp::data::initialize_params& params) override
	{
		if (params.workspaceFolders.has_value())
		{
			for (auto workspaceFolder : params.workspaceFolders.value())
			{
				auto uri = workspaceFolder.uri;
				std::string dpath;
				dpath.reserve(uri.host().length() + 1 + uri.path().length());
				dpath.append(uri.host());
				dpath.append("/");
				dpath.append(uri.path());
				std::replace(dpath.begin(), dpath.end(), '\\', '/');
				std::filesystem::path data_path(dpath);
				data_path = data_path.lexically_normal();
				std::filesystem::recursive_directory_iterator dir_start(data_path, std::filesystem::directory_options::skip_permission_denied);
				std::filesystem::recursive_directory_iterator dir_end;

				for (auto it = dir_start; it != dir_end; it++)
				{
					if (it->is_directory())
					{
						continue;
					}
					auto fpath = it->path().string();
					text_documents[fpath] = { sqfvm, fpath };
				}
			}
		}
		logger.report(*this);
	}

	virtual void on_textDocument_didChange(const lsp::data::did_change_text_document_params& params) override
	{
		auto& uri = params.textDocument.uri;
		std::string dpath;
		dpath.reserve(uri.host().length() + 1 + uri.path().length());
		dpath.append(uri.host());
		dpath.append("/");
		dpath.append(uri.path());
		std::replace(dpath.begin(), dpath.end(), '\\', '/');
		std::filesystem::path data_path(dpath);
		data_path = data_path.lexically_normal();
		auto findRes = text_documents.find(data_path.string());
		if (findRes != text_documents.end())
		{
			auto& doc = findRes->second;
			doc.reread(sqfvm);
		}
		else
		{
			auto fpath = data_path.string();
			text_documents[fpath] = { sqfvm, fpath };
		}
		logger.report(*this);
	}
	virtual std::optional<std::vector<lsp::data::folding_range>> on_textDocument_foldingRange(const lsp::data::folding_range_params& params) override
	{
		auto& uri = params.textDocument.uri;
		std::string dpath;
		dpath.reserve(uri.host().length() + 1 + uri.path().length());
		dpath.append(uri.host());
		dpath.append("/");
		dpath.append(uri.path());
		std::replace(dpath.begin(), dpath.end(), '\\', '/');
		std::filesystem::path data_path(dpath);
		data_path = data_path.lexically_normal();
		auto findRes = text_documents.find(data_path.string());
		if (findRes != text_documents.end())
		{
			auto& doc = findRes->second;
			return doc.foldings();
		}
		else
		{
			return {};
		}
	}

public:
	std::unordered_map<std::string, text_document> text_documents;
	lsp::data::initialize_params client;
	QueueLogger logger;
	sqf::runtime::runtime sqfvm;
	sqf_language_server() : logger(), sqfvm(logger, {}) {}
};

int main(int argc, char** argv)
{
#ifdef _DEBUG
	_CrtDbgReport(_CRT_ASSERT, "", 0, "", "Waiting for vs.");
#endif // _DEBUG

	// x39::uri a("aba://aba:aba@aba:aba/aba?aba#aba");
	// x39::uri b("aba://aba:aba@aba:aba?aba#aba");
	// x39::uri c("aba://aba:aba@aba?aba#aba");
	// x39::uri d("aba://aba@aba?aba#aba");
	// x39::uri e("aba://aba?aba#aba");
	// x39::uri f("aba://aba?aba");
	// x39::uri g("aba://aba");
	// x39::uri h("file://D%3A/Git/Sqfvm/vm/tests/");
	// x39::uri i("https://www.google.com/search?rlz=1C1CHBF_deDE910DE910&sxsrf=ALeKk02J_XcmnGpP0UfYPa2S-usVtUnZXw%3A1597937338384&ei=upY-X4TzFpHikgWc7pXwBQ&q=file%3A%2F%2F%2FD%3A%2Fasdasd");
	sqf_language_server ls;
	ls.listen();
}