#include <com/midnightbits/dashcam_gps_player/data/Library.hh>
#include <jni/env.hh>

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /* reserved */) {
	jni::Env::on_load(vm);
	return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNICALL
Java_com_midnightbits_dashcam_1gps_1player_data_Library_loadDirectories_1native(
    JNIEnv* env,
    jobject obj,
    jobjectArray dirs) {
	jni::EnvCall call{env};

	auto lib = com::midnightbits::dashcam_gps_player::data::Library{obj};
	lib.loadDirectories(dirs);
}
