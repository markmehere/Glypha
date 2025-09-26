package org.pazolli.glypha

import android.content.Context
import android.content.Intent
import android.content.SharedPreferences
import android.view.View
import android.os.Bundle
import android.view.KeyEvent
import android.widget.Button
import android.widget.EditText
import android.widget.Switch
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.edit
import android.util.Log

class MainActivity : AppCompatActivity() {

    companion object {
        const val PREFS_NAME = "GlyphaPrefs"
        const val KEY_PLAYER_NAME = "playerName"
        const val KEY_TOUCH_ENABLED = "touchEnabled"
    }

    private lateinit var nameField: EditText
    private lateinit var touchSwitch: Switch
    private lateinit var sharedPrefs: SharedPreferences

    fun startGlypha() {
        val orientation = resources.configuration.orientation
        if (orientation == android.content.res.Configuration.ORIENTATION_PORTRAIT) {
            Toast.makeText(this, "Please rotate to landscape to play", Toast.LENGTH_LONG).show()
        } else {
            val playerName = nameField.getText().toString()
            val touchEnabled = touchSwitch.isChecked()
            val intent = Intent(this, GlyphaActivity::class.java)
            intent.putExtra(KEY_PLAYER_NAME, playerName)
            intent.putExtra(KEY_TOUCH_ENABLED, touchEnabled);
            sharedPrefs.edit {
                putString(KEY_PLAYER_NAME, playerName)
                putBoolean(KEY_TOUCH_ENABLED, touchEnabled)
            }
            if (touchEnabled) {
                Log.d("MainActivity", "Touch Enabled sent as true")
                setContentView(R.layout.instructions_main)
                val sLaunchButton = findViewById<Button>(R.id.sLaunchButton)
                sLaunchButton.setOnClickListener {
                    startActivity(intent)
                }
            }
            else {
                Log.d("MainActivity", "Touch Enabled sent as false")
                startActivity(intent)
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        sharedPrefs = getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)

        val initialName = sharedPrefs.getString(KEY_PLAYER_NAME, makeRandomName())
        nameField = findViewById<EditText>(R.id.nameField)
        nameField.setText(initialName)

        val randomizeButton = findViewById<Button>(R.id.randomizeButton)
        randomizeButton.setOnClickListener {
            nameField.setText(makeRandomName())
        }

        val initialTouch = sharedPrefs.getBoolean(KEY_TOUCH_ENABLED, true)
        touchSwitch = findViewById<Switch>(R.id.touchSwitch)
        touchSwitch.setChecked(initialTouch)

        val launchButton = findViewById<Button>(R.id.launchButton)
        launchButton.setOnClickListener {
            startGlypha()
        }
    }

    override fun onKeyDown(keyCode: Int, event: KeyEvent?): Boolean {

        event?.let {
            if (it.repeatCount == 0) {
                when (keyCode) {
                    KeyEvent.KEYCODE_BUTTON_A -> {
                        if (touchSwitch.isChecked()) {
                            touchSwitch.setChecked(false)
                        }
                        else {
                            startGlypha()
                        }
                        return true
                    }
                    KeyEvent.KEYCODE_BUTTON_Y -> {
                        nameField.setText(makeRandomName())
                        return true
                    }
                }
            }
        }
        return super.onKeyDown(keyCode, event)
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