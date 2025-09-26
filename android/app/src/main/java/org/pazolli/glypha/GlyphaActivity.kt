package org.pazolli.glypha

import android.content.Context
import android.os.Bundle
import android.view.View
import com.google.androidgamesdk.GameActivity
import android.util.Log
import android.widget.Toast
import androidx.annotation.Keep
import androidx.core.content.edit
import org.pazolli.glypha.MainActivity.Companion.PREFS_NAME

class GlyphaActivity : GameActivity() {
    companion object {
        const val KEY_PLAYER_NAME = "playerName"
        const val KEY_TOUCH_ENABLED = "touchEnabled"
        const val KEY_REWRITE_PREFS = "rewritePrefs"
        private const val TAG = "GlyphaActivity"
        init {
            System.loadLibrary("glypha")
        }
    }

    private external fun nativeInitGameSettings(playerName: String, touchEnabled: Boolean)

    @Keep
    fun showSoundMutedToast() {
        runOnUiThread {
            Toast.makeText(this, "Sound is muted", Toast.LENGTH_SHORT).show()
        }
    }

    @Keep
    fun showSoundActiveToast() {
        runOnUiThread {
            Toast.makeText(this, "Sound is active", Toast.LENGTH_SHORT).show()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val sharedPrefs = getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
        if (intent.hasExtra(KEY_PLAYER_NAME) && intent.hasExtra(KEY_TOUCH_ENABLED)) {
            val playerName = intent.getStringExtra(KEY_PLAYER_NAME) ?: "Player"
            val touchEnabled = intent.getBooleanExtra(KEY_TOUCH_ENABLED, true) // Default if not found
            Log.d(TAG, "Player Name from Intent: $playerName")
            Log.d(TAG, "Touch Enabled from Intent: $touchEnabled")
            if (intent.hasExtra(KEY_REWRITE_PREFS) && intent.getBooleanExtra(KEY_REWRITE_PREFS, false)) {
                sharedPrefs.edit {
                    putString(MainActivity.Companion.KEY_PLAYER_NAME, playerName)
                    putBoolean(MainActivity.Companion.KEY_TOUCH_ENABLED, touchEnabled)
                }
            }
            nativeInitGameSettings(playerName, touchEnabled)
        }
        else {
            val playerName = sharedPrefs.getString(MainActivity.Companion.KEY_PLAYER_NAME, "Player") ?: "Player"
            val touchEnabled = sharedPrefs.getBoolean(MainActivity.Companion.KEY_TOUCH_ENABLED, true)
            Log.d(TAG, "Player Name from preferences: $playerName")
            Log.d(TAG, "Touch Enabled from preferences: $touchEnabled")
            nativeInitGameSettings(playerName, touchEnabled)
        }
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            hideSystemUi()
        }
    }

    private fun hideSystemUi() {
        val decorView = window.decorView
        decorView.systemUiVisibility = (View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_FULLSCREEN)
    }
}