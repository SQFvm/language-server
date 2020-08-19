#include "languageserver.h"

class sqf_language_server : public lsp::server
{
	// Inherited via server
	virtual lsp::data::responses::initialize_result on_initialize(const lsp::data::requests::initialize_params& params) override
	{
		lsp::data::responses::initialize_result res;
		res.serverInfo = {};
		res.serverInfo->name = "SQF-VM Language Server";
		res.serverInfo->version = "0.1.0";
		res.capabilities.colorProvider = {};
		res.capabilities.colorProvider->documentSelector = {};
		res.capabilities.colorProvider->documentSelector->language = "sqf";
		res.capabilities.textDocumentSync = {};
		res.capabilities.textDocumentSync->change = lsp::data::text_document_sync_kind::Full;
		res.capabilities.textDocumentSync->openClose = true;
		return res;
	}
};

int main(int argc, char** argv)
{
#ifdef _DEBUG
	_CrtDbgReport(_CRT_ASSERT, "", 0, "", "Waiting for vs.");
#endif // _DEBUG

	sqf_language_server ls;
	ls.listen();
}