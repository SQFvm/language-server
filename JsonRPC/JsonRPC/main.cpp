#include "jsonrpc.h"

int main(int argc, char** argv)
{
	jsonrpc rpc(std::cin, std::cout, jsonrpc::detach, jsonrpc::skip);
	rpc.register_method("initialize", [](jsonrpc& rpc, const jsonrpc::rpcmessage& msg) { });
	rpc.register_method("initialized", [](jsonrpc& rpc, const jsonrpc::rpcmessage& msg) { });
	rpc.register_method("workspace/didChangeConfiguration", [](jsonrpc& rpc, const jsonrpc::rpcmessage& msg) { });
}