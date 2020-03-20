#pragma once

#include <mgps/platform_export.hh>

#if defined(MGPS_STATIC) && MGPS_STATIC
#define MGPS_EXPORT
#else
#if defined(MGPS_COMPILED) && MGPS_COMPILED
#define MGPS_EXPORT MGPS_EXPORT_EXP
#else
#define MGPS_EXPORT MGPS_EXPORT_IMP
#endif
#endif // MGPS_STATIC
