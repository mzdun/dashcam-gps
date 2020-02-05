#pragma once
// @PROJECT_NAME@ @PROJECT_VERSION@@PROJECT_VERSION_STABILITY@

#define MGPS_VERSION_STR "@PROJECT_VERSION_MAJOR@.@PROJECT_VERSION_MINOR@.@PROJECT_VERSION_PATCH@"
#define MGPS_VERSION_STR_SHORT "@PROJECT_VERSION_MAJOR@.@PROJECT_VERSION_MINOR@"
#define MGPS_VERSION_STABILITY "@PROJECT_VERSION_STABILITY@"
#define MGPS_PROJECT_NAME "@PROJECT_NAME@"

#define MGPS_VERSION_MAJOR @PROJECT_VERSION_MAJOR@
#define MGPS_VERSION_MINOR @PROJECT_VERSION_MINOR@
#define MGPS_VERSION_PATCH @PROJECT_VERSION_PATCH@

#ifndef RC_INVOKED
namespace mgps {
	struct version {
		static constexpr char string[] = "@PROJECT_VERSION_MAJOR@.@PROJECT_VERSION_MINOR@.@PROJECT_VERSION_PATCH@";
		static constexpr char string_short[] = "@PROJECT_VERSION_MAJOR@.@PROJECT_VERSION_MINOR@";
		static constexpr char stability[] = "@PROJECT_VERSION_STABILITY@"; // or "-beta", "-rc3", "", ...
		static constexpr char string_ui[] = "@PROJECT_VERSION@@PROJECT_VERSION_STABILITY@";

		static constexpr unsigned major = @PROJECT_VERSION_MAJOR@;
		static constexpr unsigned minor = @PROJECT_VERSION_MINOR@;
		static constexpr unsigned patch = @PROJECT_VERSION_PATCH@;
	};
}
#endif
