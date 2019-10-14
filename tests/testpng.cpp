#if defined(USE_WINDOWS_GDIPLUS)
#include <Windows.h>
#include <GdiPlus.h>

#pragma comment(lib, "gdiplus.lib")
#else
#include <GdiPlusFlat.h>
#endif

#if defined(USE_WINDOWS_GDIPLUS)
using namespace Gdiplus;
using namespace DllExports;
#endif

#include <stdio.h>
#include <stdlib.h>
#include "testhelpers.h"

CLSID png_clsid = { 0x557cf406, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x0, 0x0, 0xf8, 0x1e, 0xf3, 0x2e } };

static void test_resave ()
{
    GpImage *img;
    WCHAR *unis;
    GpBitmap *bitmap;
    GpStatus status;
    int original_palette_size;
    int reloaded_palette_size;
    ColorPalette *original_palette;
    ColorPalette *reloaded_palette;
    PixelFormat pixel_format;
    ARGB color;

    // PNG resave should preserve the palette transparency. Let's test it
    // by loading a PNG file and its palette, then resaving it and loading
    // it again for comparison.
    unis = createWchar ("test-trns.png");
    status = GdipLoadImageFromFile (unis, &img);
    assertEqualInt(status, Ok);
    freeWchar (unis);

    status = GdipGetImagePaletteSize (img, &original_palette_size);
    assertEqualInt(status, Ok);
    assertEqualInt(original_palette_size, 72);
    original_palette = (ColorPalette *) malloc (original_palette_size);
    GdipGetImagePalette (img, original_palette, original_palette_size);
    assertEqualInt(status, Ok);

    unis = createWchar ("test-trns-resave.png");
    status = GdipSaveImageToFile (img, unis, &png_clsid, NULL);
    assertEqualInt(status, Ok);
    GdipDisposeImage (img);
    status = GdipLoadImageFromFile (unis, &img);
    assertEqualInt(status, Ok);
    freeWchar (unis);

    status = GdipGetImagePaletteSize (img, &reloaded_palette_size);
    assertEqualInt(status, Ok);
    assertEqualInt(reloaded_palette_size, 72);
    assertEqualInt(reloaded_palette_size, original_palette_size);
    reloaded_palette = (ColorPalette *) malloc (reloaded_palette_size);
    GdipGetImagePalette (img, reloaded_palette, reloaded_palette_size);
    assertEqualInt(status, Ok);

    assertEqualInt(memcmp (original_palette, reloaded_palette, original_palette_size), 0);

    GdipDisposeImage (img);
    img = NULL;
    deleteFile ("test-trns-resave.png");
    free (original_palette);
    free (reloaded_palette);

    // Test grayscale image with alpha channel. The image should be converted
    // into 32-bit ARGB format and the alpha channel should be preserved.
    unis = createWchar ("test-gsa.png");
    status = GdipCreateBitmapFromFile (unis, &bitmap);
    assertEqualInt(status, Ok);
    freeWchar (unis);
    status = GdipGetImagePixelFormat (bitmap, &pixel_format);
    assertEqualInt(status, Ok);
    assertEqualInt(pixel_format, PixelFormat32bppARGB);
    status = GdipBitmapGetPixel (bitmap, 0, 0, &color);
    assertEqualInt(status, Ok);
    assertEqualInt(color, 0xffffff);
    status = GdipBitmapGetPixel (bitmap, 1, 7, &color);
    assertEqualInt(status, Ok);
    assertEqualInt(color, 0xe8b3b3b3);
    GdipDisposeImage (bitmap);
}

LIBGDIPLUS_TEST (testpng, test_resave)
