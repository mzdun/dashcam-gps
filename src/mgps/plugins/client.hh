#pragma once

#ifdef __GNUC__
#define API_EXPORT extern "C" __attribute__((visibility("default")))
#else
#define API_EXPORT extern "C" __declspec(dllexport)
#endif

namespace mgps {
	struct media_file;
}

API_EXPORT bool mgps_probe(char const* filename);
API_EXPORT bool mgps_load(char const* filename, mgps::media_file* out);
