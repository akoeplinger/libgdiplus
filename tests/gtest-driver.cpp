#include <gtest/gtest.h>
#include "testhelpers.h"

ULONG_PTR globalGdiplusToken = 0;

void StartupGdiplus ()
{
	if (globalGdiplusToken != 0)
		return;

	GdiplusStartupInput gdiplusStartupInput;
	gdiplusStartupInput.GdiplusVersion = 1;
	gdiplusStartupInput.DebugEventCallback = NULL;
	gdiplusStartupInput.SuppressBackgroundThread = FALSE;
	gdiplusStartupInput.SuppressExternalCodecs = FALSE;

	GdiplusStartup (&globalGdiplusToken, &gdiplusStartupInput, NULL);
}

void ShutdownGdiplus ()
{
	if (globalGdiplusToken != 0) {
		GdiplusShutdown (globalGdiplusToken);
		globalGdiplusToken = 0;
	}
}

int main (int argc, char **argv)
{
	::testing::InitGoogleTest (&argc, argv);

	StartupGdiplus ();
	int result = RUN_ALL_TESTS ();
	ShutdownGdiplus ();

	deleteFile ("temp_asset.bmp");
	deleteFile ("temp_asset.emf");
	deleteFile ("temp_asset.gif");
	deleteFile ("temp_asset.ico");
	deleteFile ("temp_asset.jpeg");
	deleteFile ("temp_asset.png");
	deleteFile ("temp_asset.tif");
	deleteFile ("temp_asset.wmf");

	return result;
}
