package com.midnightbits.mgps

import java.lang.StringBuilder
import java.util.*
import kotlin.collections.ArrayList

enum class Clip { Unrecognized, Normal, Emergency, Parking, Other }
enum class Page { Everything, Emergency, Parking }

data class Duration(val milliseconds: Long) {
    fun toMillis(): Long = milliseconds
    fun toSeconds(): Long = milliseconds / 1000
    fun toMinutes(): Long = toSeconds() / 60
    fun toHours(): Long = toMinutes() / 60

    fun toStringEx(): String = milliseconds.toString()
    override fun toString(): String {
        val ms = if (milliseconds < 0) -milliseconds else milliseconds;
        val zero__ : Long = 0
        val s__: Long = (ms + 500) / 1000
        if (s__ == zero__)
            return "0 s";

        val h : Long = s__ /3600
        val m : Long = s__ / 60 - h * 60
        val s : Long = s__ - m * 60

        val sb = StringBuilder()

        if (milliseconds < 0)
            sb.append("-");

        if (h > 0) {
            sb.append(h).append(" h")
            if (m > 0 || s > 0)
                sb.append(' ')
        }

        if (m > 0) {
            sb.append(m).append(" min")
            if (s > 0)
                sb.append(' ')
        }

        if (s > 0)
            sb.append(s).append(" s")

        return sb.toString()
    }
}

data class MediaFile (
    val path : String,
    val date: Date,
    val duration: Duration,
    val clip: Clip) {}

data class MediaClip (val offset: Duration, val duration: Duration, val file : MediaFile) {}

data class GpsPoint(val lat: Double, val lon: Double, val kmph: Long, val duration : Duration) {}

data class GpsSegment(
    val offset: Duration,
    val duration: Duration,
    val distanceInMetres: Long,
    val points: MutableList<GpsPoint>) {}

data class GpsTrace(val offset: Duration, val lines: MutableList<GpsSegment>) {}

data class Trip (
    val start: Date,
    val duration: Duration,
    val playlist: MutableList<MediaClip>,
    val trace: GpsTrace) {}

data class Filter(val id: Int, val page: Page, var trips: MutableList<Trip>) {
    override fun toString(): String = "${page}/${trips.size}"
}

class Library {
    val Filters: MutableList<Filter> = ArrayList()
    private var callback: DirectoryLoadCallback = DummyCallback()

    interface DirectoryLoadCallback { fun onLoadDone(); }
    private class DummyCallback :
        DirectoryLoadCallback { override fun onLoadDone() {} }

    fun setDirectoryLoadCallback(cb: DirectoryLoadCallback?) {
        if (cb != null)
            callback = cb
        else
            callback = DummyCallback()
    }

    fun loadDirectories(dirs: Array<String>) {
        loadDirectories_native(dirs);
        callback.onLoadDone();
    }
    private external fun loadDirectories_native(dirs: Array<String>)

    companion object {
        val self = Library()

        init {
            System.loadLibrary("dashcam-gps-player-native")
        }
    }
}