package com.midnightbits.mgps

import android.Manifest
import android.app.Activity
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import android.os.Environment
import android.util.Log
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.core.os.EnvironmentCompat
import androidx.fragment.app.Fragment
import java.io.File
import java.io.InputStream
import java.util.*
import kotlin.collections.ArrayList

class Utils {

    /* returns external storage paths (directory of external memory card) as array of Strings */
    companion object {
        const val PERMISSION_GRANTED = 0
        const val PERMISSION_DENIED = -1
        const val PERMISSION_CANCELED = -99
        const val CONTINUE_CHECKING = -100

        val LOG_TAG: String = "mGPS"

        fun getExternalStorageDirectories(context: Context?): Array<File?>? {
            val results: MutableList<String> = ArrayList()
            val externalDirs = context?.getExternalFilesDirs(null)
            if (externalDirs != null) {
                for (file in externalDirs) {
                    if (file == null) //solved NPE on some Lollipop devices
                        continue
                    val path = file.path.split("/Android").toTypedArray()[0]
                    val addPath = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                        Environment.isExternalStorageRemovable(file) ||
                        Environment.isExternalStorageEmulated(file)
                    } else {
                        Environment.MEDIA_MOUNTED.equals(EnvironmentCompat.getStorageState(file))
                    }
                    if (addPath) {
                        results.add(path)
                    }
                }
            }

            if (results.isEmpty()) { //Method 2 for all versions
                // better variation of: http://stackoverflow.com/a/40123073/5002496
                var output = ""
                try {
                    val process =
                        ProcessBuilder().command("mount | grep /dev/block/vold")
                            .redirectErrorStream(true).start()
                    process.waitFor()
                    val istream: InputStream = process.inputStream
                    val buffer = ByteArray(1024)
                    while (istream.read(buffer) != -1) {
                        output = output + String(buffer)
                    }
                    istream.close()
                } catch (e: Exception) {
                    e.printStackTrace()
                }
                if (!output.trim { it <= ' ' }.isEmpty()) {
                    val devicePoints = output.split("\n").toTypedArray()
                    for (voldPoint in devicePoints) {
                        results.add(voldPoint.split(" ").toTypedArray()[2])
                    }
                }
            }
            //Below few lines is to remove paths which may not be external memory card, like OTG (feel free to comment them out)
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                var i = 0
                while (i < results.size) {
                    val dirname = results[i].toLowerCase(Locale.ROOT)
                    if (!dirname.matches(Regex(".*[0-9a-f]{4}[-][0-9a-f]{4}"))
                        && !dirname.matches(Regex("/storage/emulated/[0-9]+"))) {
                        Log.d(LOG_TAG, results[i] + " might not be extSDcard")
                        results.removeAt(i--)
                    }
                    i++
                }
            } else {
                var i = 0
                while (i < results.size) {
                    val dirname = results[i].toLowerCase(Locale.ROOT)
                    if (!dirname.contains("ext") && !dirname.contains("sdcard")) {
                        Log.d(LOG_TAG, results[i] + " might not be extSDcard")
                        results.removeAt(i--)
                    }
                    i++
                }
            }
            val storageDirectories =
                arrayOfNulls<File>(results.size)
            for (i in results.indices) storageDirectories[i] = File(results[i])
            return storageDirectories
        }

        private val EXTERNAL_PERMS = arrayOf<String>(Manifest.permission.READ_EXTERNAL_STORAGE)
        private const val EXTERNAL_REQUEST = 138

        private fun checkDcimPermission(context: Context): Boolean {
            if (Build.VERSION.SDK_INT >= 23) {
                if (ContextCompat.checkSelfPermission(context, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                    return false
                }
            }
            return true
        }

        fun checkOrRequestDcimPermission(fragment : Fragment): Boolean {
            if (!checkDcimPermission(fragment.context!!)) {
                fragment.requestPermissions(EXTERNAL_PERMS, EXTERNAL_REQUEST)
                return false
            }
            return true
        }

        fun checkOrRequestDcimPermission(activity : Activity): Boolean {
            if (!checkDcimPermission(activity)) {
                ActivityCompat.requestPermissions(activity, EXTERNAL_PERMS, EXTERNAL_REQUEST);
                return false
            }
            return true
        }


        fun onRequestPermissionsResult(
            requestCode: Int,
            permissions: Array<out String>,
            grantResults: IntArray
        ): Int {
            if (requestCode != EXTERNAL_REQUEST)
                return CONTINUE_CHECKING

            var permissionIndex: Int = 0
            for (per in permissions) {
                if (per == Manifest.permission.READ_EXTERNAL_STORAGE)
                    break
                ++permissionIndex
            }
            if (permissionIndex >= grantResults.size) {
                return PERMISSION_CANCELED
            }
            if (grantResults[permissionIndex] == PackageManager.PERMISSION_GRANTED)
                return PERMISSION_GRANTED

            return PERMISSION_DENIED
        }

        fun dcimFolders(ctx: Context?):Array<String>? {
            val dirs = getExternalStorageDirectories(ctx)
            if (dirs == null) {
                Log.d(LOG_TAG, "No directories found!")
                return null
            }

            val results: MutableList<String> = ArrayList()
            for (dir in dirs) {
                val dcim = File(dir, "DCIM")
                if (!dcim.isDirectory)
                    continue

                results.add(dcim.canonicalPath)
            }
            return results.toTypedArray()
        }
    }
}