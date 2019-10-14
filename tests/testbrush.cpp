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

static void test_getBrushType ()
{
    GpStatus status;
    GpSolidFill *brush;
    GpBrushType brushType;

    GdipCreateSolidFill (1, &brush);

    // Negative tests.
    status = GdipGetBrushType (NULL, &brushType);
    assertEqualInt (status, InvalidParameter);

    status = GdipGetBrushType ((GpBrush *) brush, NULL);
    assertEqualInt (status, InvalidParameter);

    GdipDeleteBrush ((GpBrush *) brush);
}

static void test_clone ()
{
    GpStatus status;
    GpSolidFill *brush;
    GpBrush *clone;

    GdipCreateSolidFill (1, &brush);

    // Negative tests.
    status = GdipCloneBrush (NULL, &clone);
    assertEqualInt (status, InvalidParameter);

    status = GdipCloneBrush ((GpBrush *) brush, NULL);
    assertEqualInt (status, InvalidParameter);

    GdipDeleteBrush ((GpBrush *) brush);
}

static void test_delete ()
{
    GpStatus status;
    GpSolidFill *brush;

    GdipCreateSolidFill (1, &brush);

    status = GdipDeleteBrush ((GpBrush *) brush);
    assertEqualInt (status, Ok);

    // Negative tests.
    status = GdipDeleteBrush (NULL);
    assertEqualInt (status, InvalidParameter);
}

LIBGDIPLUS_TEST (testbrush, test_getBrushType)
LIBGDIPLUS_TEST (testbrush, test_clone)
LIBGDIPLUS_TEST (testbrush, test_delete)
