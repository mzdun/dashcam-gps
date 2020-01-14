#pragma once

#define JNI_PRIMITIVES(X)   \
	X(jbyte, Byte, "B")     \
	X(jchar, Char, "C")     \
	X(jdouble, Double, "D") \
	X(jfloat, Float, "F")   \
	X(jint, Int, "I")       \
	X(jlong, Long, "J")     \
	X(jshort, Short, "S")   \
	X(jboolean, Boolean, "Z")
