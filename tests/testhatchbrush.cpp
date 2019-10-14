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

static void test_clone ()
{
    GpStatus status;
    GpBrush *brush;
    GpBrushType brushType;
    GpBrush *clone;

    GpHatchStyle hatchStyle;
    ARGB foregroundColor;
    ARGB backgroundColor;

    GdipCreateHatchBrush (HatchStyle10Percent, 0x00000001, 0x00000002, (GpHatch **)&brush);

    status = GdipCloneBrush (brush, &clone);
    assertEqualInt (status, Ok);
    assertTrue (clone);
    assertTrue (brush != clone);

    GdipGetBrushType (clone, &brushType);
    assertEqualInt (brushType, BrushTypeHatchFill);

    GdipGetHatchStyle ((GpHatch *) clone, &hatchStyle);
    assertEqualInt (hatchStyle, HatchStyle10Percent);

    GdipGetHatchForegroundColor ((GpHatch *) clone, &foregroundColor);
    assertEqualInt (foregroundColor, 0x00000001);

    GdipGetHatchBackgroundColor ((GpHatch *) clone, &backgroundColor);
    assertEqualInt (backgroundColor, 0x00000002);

    GdipDeleteBrush (brush);
    GdipDeleteBrush (clone);
}

static void test_delete ()
{
    GpStatus status;
    GpHatch *brush;

    GdipCreateHatchBrush (HatchStyle05Percent, 0x00000001, 0x00000002, &brush);

    status = GdipDeleteBrush ((GpBrush *) brush);
    assertEqualInt (status, Ok);
}

static void test_createHatchBrush ()
{
    GpStatus status;
    GpHatch *brush;
    GpBrushType brushType;

    status = GdipCreateHatchBrush (HatchStyleMin, 0x00000001, 0x00000002, &brush);
    assertEqualInt (status, Ok);
    assertTrue (brush) << "Expected the brush to be initialized.";

    status = GdipGetBrushType ((GpBrush *) brush, &brushType);
    assertEqualInt (status, Ok);
    assertEqualInt (brushType, BrushTypeHatchFill);

    GdipDeleteBrush ((GpBrush *) brush);

    status = GdipCreateHatchBrush (HatchStyleMax, 0x00000001, 0x00000002, &brush);
    assertEqualInt (status, Ok);
    assertTrue (brush) << "Expected the brush to be initialized.";

    GdipDeleteBrush ((GpBrush *)brush);

    // Negative tests.
    brush = (GpHatch *) 0xCC;
    status = GdipCreateHatchBrush (HatchStyle05Percent, 0x00000001, 0x00000002, NULL);
    assertEqualInt (status, InvalidParameter);
    assertEqual (brush, (GpHatch *) 0xCC);

    brush = (GpHatch *) 0xCC;
    status = GdipCreateHatchBrush ((HatchStyle)(HatchStyleMin - 1), 0x00000001, 0x00000002, &brush);
    assertEqualInt (status, InvalidParameter);
    assertEqual (brush, (GpHatch *) 0xCC);

    brush = (GpHatch *) 0xCC;
    status = GdipCreateHatchBrush ((HatchStyle)(HatchStyleMax + 1), 0x00000001, 0x00000002, &brush);
    assertEqualInt (status, InvalidParameter);
    assertEqual (brush, (GpHatch *) 0xCC);
}

static void test_getHatchStyle ()
{
    GpStatus status;
    GpHatch *brush;
    GpHatchStyle hatchStyle;

    GdipCreateHatchBrush (HatchStyle05Percent, 0x00000001, 0x00000002, &brush);

    status = GdipGetHatchStyle (brush, &hatchStyle);
    assertEqualInt (status, Ok);
    assertEqualInt (hatchStyle, HatchStyle05Percent);

    // Negative tests.
    status = GdipGetHatchStyle (NULL, &hatchStyle);
    assertEqualInt (status, InvalidParameter);

    status = GdipGetHatchStyle (brush, NULL);
    assertEqualInt (status, InvalidParameter);

    GdipDeleteBrush ((GpBrush *) brush);
}

static void test_getForegroundColor ()
{
    GpStatus status;
    GpHatch *brush;
    ARGB foregroundColor;

    GdipCreateHatchBrush (HatchStyle05Percent, 0x00000001, 0x00000002, &brush);

    status = GdipGetHatchForegroundColor (brush, &foregroundColor);
    assertEqualInt (status, Ok);
    assertEqualInt (foregroundColor, 0x00000001);

    // Negative tests.
    status = GdipGetHatchForegroundColor (NULL, &foregroundColor);
    assertEqualInt (status, InvalidParameter);

    status = GdipGetHatchForegroundColor (brush, NULL);
    assertEqualInt (status, InvalidParameter);

    GdipDeleteBrush ((GpBrush *) brush);
}

static void test_getBackgroundColor ()
{
    GpStatus status;
    GpHatch *brush;
    ARGB backgroundColor;

    GdipCreateHatchBrush (HatchStyle05Percent, 0x00000001, 0x00000002, &brush);

    status = GdipGetHatchBackgroundColor (brush, &backgroundColor);
    assertEqualInt (status, Ok);
    assertEqualInt (backgroundColor, 0x00000002);

    // Negative tests.
    status = GdipGetHatchBackgroundColor (NULL, &backgroundColor);
    assertEqualInt (status, InvalidParameter);

    status = GdipGetHatchBackgroundColor (brush, NULL);
    assertEqualInt (status, InvalidParameter);

    GdipDeleteBrush ((GpBrush *) brush);
}

LIBGDIPLUS_TEST (testhatchbrush, test_clone)
LIBGDIPLUS_TEST (testhatchbrush, test_delete)
LIBGDIPLUS_TEST (testhatchbrush, test_createHatchBrush)
LIBGDIPLUS_TEST (testhatchbrush, test_getHatchStyle)
LIBGDIPLUS_TEST (testhatchbrush, test_getForegroundColor)
LIBGDIPLUS_TEST (testhatchbrush, test_getBackgroundColor)

