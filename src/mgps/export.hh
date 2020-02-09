#pragma once

#include <mgps/platform_export.hh>

#if defined(MGPS_COMPILED) && MGPS_COMPILED
#define MGPS_EXPORT MGPS_EXPORT_EXP
#else
#define MGPS_EXPORT MGPS_EXPORT_IMP
#endif
