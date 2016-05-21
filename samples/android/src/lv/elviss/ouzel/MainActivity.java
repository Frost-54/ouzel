// Copyright (C) 2016 Elviss Strazdins
// This file is part of the Ouzel engine.

package lv.elviss.ouzel;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.widget.Toast;

public class MainActivity extends Activity
{
	private GLSurfaceView glSurfaceView;

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		ActivityManager activityManager 
			= (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
		ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();

		final boolean supportsEs2 = 
			configurationInfo.reqGlEsVersion >= 0x20000 || isProbablyEmulator();

		if (supportsEs2)
		{
			glSurfaceView = new GLSurfaceView(this);

			if (isProbablyEmulator())
			{
				// Avoids crashes on startup with some emulator images.
				glSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
			}

			glSurfaceView.setEGLContextClientVersion(2);
			glSurfaceView.setRenderer(new RendererWrapper());
			setContentView(glSurfaceView);
		}
		else
		{
			// Should never be seen in production, since the manifest filters
			// unsupported devices.
			Toast.makeText(this, "This device does not support OpenGL ES 2.0.",
					Toast.LENGTH_LONG).show();
			return;
		}
	}

	private boolean isProbablyEmulator()
	{
		return Build.VERSION.SDK_INT >= Build.VERSION_CODES.GINGERBREAD
				&& (Build.FINGERPRINT.startsWith("generic")
						|| Build.FINGERPRINT.startsWith("unknown")
						|| Build.MODEL.contains("google_sdk")
						|| Build.MODEL.contains("Emulator")
						|| Build.MODEL.contains("Android SDK built for x86"));
	}

	@Override
	protected void onPause()
	{
		super.onPause();

		if (glSurfaceView != null)
		{
			glSurfaceView.onPause();
		}
	}

	@Override
	protected void onResume()
	{
		super.onResume();

		if (glSurfaceView != null)
		{
			glSurfaceView.onResume();
		}
	}
}
