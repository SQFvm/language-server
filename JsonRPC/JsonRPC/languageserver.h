#pragma once
#include "jsonrpc.h"

namespace lsp
{
	class server
	{
	public:
		struct client_capabilities
		{
			struct workspace_
			{
				bool applyEdit;
				struct workspaceEdit_
				{
					enum eEesourceOperations
					{
						ro_create = 0b001,
						ro_rename = 0b010,
						ro_delete = 0b100
					};
					enum eFailureHandling
					{
						textOnlyTransactional = 0b001
					};
					bool documentChanges;
					eEesourceOperations resourceOperations;
					eFailureHandling failureHandling;
				} workspaceEdit;
			} workspace;
		};
	private:
		void initialize(jsonrpc& rpc, const jsonrpc::rpcmessage& msg)
		{
		}
	public:
		jsonrpc rpc;
		server() : rpc(std::cin, std::cout, jsonrpc::detach, jsonrpc::skip)
		{
			rpc.register_method("initialize", [&](jsonrpc& rpc, const jsonrpc::rpcmessage& msg) { initialize(rpc, msg); });
		}

		void listen()
		{
			while (true)
			{
				if (!rpc.handle_single_message())
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					continue;
				}
			}
		}
	};
}