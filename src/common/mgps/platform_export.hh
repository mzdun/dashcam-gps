#pragma once

#if defined(WIN32)
#define MGPS_EXPORT_EXP __declspec(dllexport)
#define MGPS_EXPORT_IMP __declspec(dllimport)
#else
#define MGPS_EXPORT_EXP __attribute__((visibility("default")))
#define MGPS_EXPORT_IMP
#endif
