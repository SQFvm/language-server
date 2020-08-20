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

class sqf_language_server : public lsp::server
{
	lsp::data::initialize_params client;
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

	virtual void on_textDocument_didOpen(const lsp::data::did_open_text_document_params& params) override
	{

	}
	virtual void on_textDocument_didChange(const lsp::data::did_change_text_document_params& params) override
	{

	}
	virtual std::optional<std::vector<lsp::data::folding_range>> on_textDocument_foldingRange(const lsp::data::folding_range_params& params) override
	{
		return {};
	}

public:
	class QueueLogger : public Logger {
	public:
		QueueLogger() : Logger() {}
		std::queue<std::string> infos;
		std::queue<std::string> warnings;
		std::queue<std::string> errors;
		std::queue<std::string> other;

		virtual void log(loglevel level, std::string_view message) override
		{
			std::stringstream sstream;
			switch (level)
			{
			case loglevel::fatal:
			case loglevel::error:
				errors.push(std::string(message));
				break;
			case loglevel::warning:
				warnings.push(std::string(message));
				break;
			case loglevel::info:
				errors.push(std::string(message));
				break;
			case loglevel::verbose:
			case loglevel::trace:
			default:
				other.push(std::string(message));
				break;
			}

		}
	};
	QueueLogger logger;
	sqf::runtime::runtime sqfvm;
	sqf_language_server() : logger(), sqfvm(logger, {}) {}
};

int main(int argc, char** argv)
{
#ifdef _DEBUG
	_CrtDbgReport(_CRT_ASSERT, "", 0, "", "Waiting for vs.");
#endif // _DEBUG

	sqf_language_server ls;
	ls.listen();
}