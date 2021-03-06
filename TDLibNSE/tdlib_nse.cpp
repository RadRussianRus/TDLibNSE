#include "tdlib_nse.h"

namespace tdlibnse {
namespace {

bool receive_thread_running = false;

struct threadParams {
	double timeout;
	sciter::value callback;
};

void receiveThread(threadParams params) {
	int delay = 0;
	const auto startChars = aux::utf2w("{\"tdlibnse\":\"receive_thread_started\"}");
	delay = params.callback.call(sciter::string(startChars.c_str(), startChars.length())).get(0);

	while (delay >= 0 && receive_thread_running) {
		if (delay > 0) {
			::Sleep(delay);
		}
		const auto updateChars = aux::utf2w(td_receive(params.timeout));
		delay = params.callback.call(sciter::string(updateChars.c_str(), updateChars.length())).get(0);
	}

	if (receive_thread_running) {
		receive_thread_running = false;
	}

	const auto endChars = aux::utf2w("{\"tdlibnse\":\"receive_thread_stopped\"}");
	params.callback.call(sciter::string(endChars.c_str(), endChars.length()));
}

} // namespace

sciter::value TDLib::startReceiveThread(double timeout, sciter::value callback) {
	receive_thread_running = true;
	threadParams params;
	params.timeout = timeout;
	params.callback = callback;
	sciter::thread(receiveThread, params);
	return sciter::value();
}

sciter::value TDLib::stopReceiveThread() {
	receive_thread_running = false;
	return sciter::value();
}

bool TDLib::receiveThreadRunning() {
	return receive_thread_running;
}

} // namespace tdlibnse

extern "C" {

#ifndef WINDOWS
__attribute__((visibility("default")))
#endif
BOOL SCAPI SciterLibraryInit(ISciterAPI *psapi, SCITER_VALUE *plibobject) {
#ifdef WINDOWS
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
#endif
	_SAPI(psapi);
	static sciter::om::hasset<tdlibnse::TDLib> tdlib_root = new tdlibnse::TDLib();
	*plibobject = sciter::value::wrap_asset(tdlib_root);
	return TRUE;
}

} // extern "C"