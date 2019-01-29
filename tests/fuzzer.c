#include <stdint.h>
#include <stddef.h>
#include <config.h>
#include <glib.h>
#include <GdiPlusFlat.h>

static ULONG_PTR gdiplusToken = 0;

void InitLibgdiplus () {
    GdiplusStartupInput gdiplusStartupInput;
    gdiplusStartupInput.GdiplusVersion = 1;
    gdiplusStartupInput.DebugEventCallback = NULL;
    gdiplusStartupInput.SuppressBackgroundThread = FALSE;
    gdiplusStartupInput.SuppressExternalCodecs = FALSE;
    GdiplusStartup (&gdiplusToken, &gdiplusStartupInput, NULL);
}

#if defined (HAVE_LIBFUZZER)

int LLVMFuzzerTestOneInput (const uint8_t *data, size_t size) {

    if (gdiplusToken == 0)
        InitLibgdiplus ();

    GdipLoadBitmapFromMemoryStream (data, size);

    return 0;
}

#else

int main (int argc, char *argv[]) {

    if (gdiplusToken == 0)
        InitLibgdiplus ();

    if (argc != 2)
        return 1;

    WCHAR* file = g_utf8_to_utf16 (argv[1], -1, NULL, NULL, NULL);
    GpImage* image = NULL;

    GdipLoadImageFromFile (file, &image);

    GdipDisposeImage (image);

    GdiplusShutdown (gdiplusToken);

    return 0;
}

#endif
