#if defined(USE_WINDOWS_GDIPLUS)
#include <Windows.h>
#include <GdiPlus.h>

#pragma comment(lib, "gdiplus")
#else
#include <GdiPlusFlat.h>
#endif

#if defined(USE_WINDOWS_GDIPLUS)
using namespace Gdiplus;
using namespace DllExports;
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "testhelpers.h"

static void verifyArrowCap (GpAdjustableArrowCap *cap, REAL expectedHeight, REAL expectedWidth, BOOL expectedIsFilled)
{
	assertTrue (cap) << "Expected cap to be initialized.";

	GpStatus status;
	REAL height;
	REAL width;
	BOOL isFilled;
	REAL middleInset;
	CustomLineCapType capType;
	LineCap baseCap;
	REAL baseInset;
	LineCap startCap;
	LineCap endCap;
	LineJoin strokeJoin;
	REAL widthScale;

	status = GdipGetAdjustableArrowCapHeight (cap, &height);
	assertEqualInt (status, Ok);
	if (isnan (expectedHeight))
		assertNaN (height);
	else
		assertEqualFloat (height, expectedHeight);

	status = GdipGetAdjustableArrowCapWidth (cap, &width);
	assertEqualInt (status, Ok);
	if (isnan (expectedWidth))
		assertNaN (width);
	else
		assertEqualFloat (width, expectedWidth);

	status = GdipGetAdjustableArrowCapFillState (cap, &isFilled);
	assertEqualInt (status, Ok);
	assertEqual (isFilled, expectedIsFilled);

	status = GdipGetAdjustableArrowCapMiddleInset (cap, &middleInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (middleInset, 0);

	status = GdipGetCustomLineCapType ((GpCustomLineCap *) cap, &capType);
	assertEqualInt (status, Ok);
	assertEqualInt (capType, CustomLineCapTypeAdjustableArrow);

	status = GdipGetCustomLineCapBaseCap ((GpCustomLineCap *) cap, &baseCap);
	assertEqualInt (status, Ok);
	assertEqualInt (baseCap, LineCapTriangle);

	status = GdipGetCustomLineCapBaseInset ((GpCustomLineCap *) cap, &baseInset);
	assertEqualInt (status, Ok);
	REAL expectedInset = (expectedWidth == 0 ? 0 : (expectedHeight / expectedWidth));
	if (isnan (expectedInset))
		assertNaN (expectedInset);
	else
		assertEqualFloat (baseInset, expectedInset);

	status = GdipGetCustomLineCapStrokeCaps ((GpCustomLineCap *) cap, &startCap, &endCap);
	assertEqualInt (status, Ok);
	assertEqualInt (startCap, LineCapFlat);
	assertEqualInt (endCap, LineCapFlat);

	status = GdipGetCustomLineCapStrokeJoin ((GpCustomLineCap *) cap, &strokeJoin);
	assertEqualInt (status, Ok);
	assertEqualInt (strokeJoin, LineJoinMiter);

	status = GdipGetCustomLineCapWidthScale ((GpCustomLineCap *) cap, &widthScale);
	assertEqualInt (status, Ok);
	assertEqualFloat (widthScale, 1);
}

static void test_createAdjustableArrowCap ()
{
	GpStatus status;
	GpAdjustableArrowCap *cap;

	status = GdipCreateAdjustableArrowCap (10, 11, TRUE, &cap);
	assertEqualInt (status, Ok);
	verifyArrowCap (cap, 10, 11, TRUE);
	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);

	status = GdipCreateAdjustableArrowCap (0, 0, FALSE, &cap);
	assertEqualInt (status, Ok);
	verifyArrowCap (cap, 0, 0, FALSE);
	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);

	status = GdipCreateAdjustableArrowCap (2, 0, FALSE, &cap);
	assertEqualInt (status, Ok);
	verifyArrowCap (cap, 2, 0, FALSE);
	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);

	status = GdipCreateAdjustableArrowCap (0, 2, FALSE, &cap);
	assertEqualInt (status, Ok);
	verifyArrowCap (cap, 0, 2, FALSE);
	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);

	status = GdipCreateAdjustableArrowCap (-1, -2, FALSE, &cap);
	assertEqualInt (status, Ok);
	verifyArrowCap (cap, -1, -2, FALSE);
	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);

	status = GdipCreateAdjustableArrowCap (NAN, -2, FALSE, &cap);
	assertEqualInt (status, Ok);
	verifyArrowCap (cap, NAN, -2, FALSE);
	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);

	status = GdipCreateAdjustableArrowCap (1, NAN, FALSE, &cap);
	assertEqualInt (status, Ok);
	verifyArrowCap (cap, 1, NAN, FALSE);
	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);

	status = GdipCreateAdjustableArrowCap (-INFINITY, 1, FALSE, &cap);
	assertEqualInt (status, Ok);
	verifyArrowCap (cap, -INFINITY, 1, FALSE);
	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);

	status = GdipCreateAdjustableArrowCap (1, INFINITY, FALSE, &cap);
	assertEqualInt (status, Ok);
	verifyArrowCap (cap, 1, INFINITY, FALSE);
	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);

	// Negative tests.
	status = GdipCreateAdjustableArrowCap (10, 11, TRUE, NULL);
	assertEqualInt (status, InvalidParameter);
}

static void test_cloneAdjustableArrowCap ()
{
	GpStatus status;
	GpAdjustableArrowCap *cap;
	GpCustomLineCap *clonedCap;

	GdipCreateAdjustableArrowCap (10, 11, TRUE, &cap);

	status = GdipCloneCustomLineCap ((GpCustomLineCap *) cap, &clonedCap);
	assertEqualInt (status, Ok);
	assertTrue (clonedCap);
	assertTrue (clonedCap != cap);
	verifyArrowCap ((GpAdjustableArrowCap *) clonedCap, 10, 11, TRUE);
	GdipDeleteCustomLineCap ((GpCustomLineCap *) clonedCap);

	// Negative tests.
	status = GdipCloneCustomLineCap (NULL, &clonedCap);
	assertEqualInt (status, InvalidParameter);

	// This causes a null pointer reference in GDI+.
#if !defined(_WIN32)
	status = GdipCloneCustomLineCap (cap, NULL);
	assertEqualInt (status, InvalidParameter);
#endif

	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);
}

static void test_deleteAdjustableArrowCap ()
{
	GpStatus status;
	GpAdjustableArrowCap *cap;

	GdipCreateAdjustableArrowCap (10, 11, TRUE, &cap);

	status = GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);
	assertEqualInt (status, Ok);

	// Negative tests.
	status = GdipDeleteCustomLineCap (NULL);
	assertEqualInt (status, InvalidParameter);
}

static void test_setAdjustableArrowCapHeight ()
{
	GpStatus status;
	GpAdjustableArrowCap *cap;
	REAL height;
	REAL baseInset;
	LineCap baseCap;

	GdipCreateAdjustableArrowCap (150, 3, TRUE, &cap);

	// Positive value.
	GdipSetCustomLineCapBaseInset ((GpCustomLineCap *) cap, 120);
	GdipSetCustomLineCapBaseCap ((GpCustomLineCap *) cap, LineCapRound);
	status = GdipSetAdjustableArrowCapHeight (cap, 30);
	assertEqualInt (status, Ok);

	status = GdipGetAdjustableArrowCapHeight (cap, &height);
	assertEqualInt (status, Ok);
	assertEqualFloat (height, 30);

	status = GdipGetCustomLineCapBaseInset ((GpCustomLineCap *) cap, &baseInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (baseInset, 10);

	// Same value.
	GdipSetCustomLineCapBaseInset ((GpCustomLineCap *) cap, 120);
	GdipSetCustomLineCapBaseCap ((GpCustomLineCap *) cap, LineCapRound);
	status = GdipSetAdjustableArrowCapHeight (cap, 30);
	assertEqualInt (status, Ok);

	status = GdipGetAdjustableArrowCapHeight (cap, &height);
	assertEqualInt (status, Ok);
	assertEqualFloat (height, 30);

	status = GdipGetCustomLineCapBaseInset ((GpCustomLineCap *) cap, &baseInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (baseInset, 120);

	status = GdipGetCustomLineCapBaseCap ((GpCustomLineCap *) cap, &baseCap);
	assertEqualInt (status, Ok);
	assertEqualInt (baseCap, LineCapRound);

	// Zero.
	GdipSetCustomLineCapBaseInset ((GpCustomLineCap *) cap, 120);
	GdipSetCustomLineCapBaseCap ((GpCustomLineCap *) cap, LineCapRound);
	status = GdipSetAdjustableArrowCapHeight (cap, 0);
	assertEqualInt (status, Ok);

	status = GdipGetAdjustableArrowCapHeight (cap, &height);
	assertEqualInt (status, Ok);
	assertEqualFloat (height, 0);

	status = GdipGetCustomLineCapBaseInset ((GpCustomLineCap *) cap, &baseInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (baseInset, 0);

	status = GdipGetCustomLineCapBaseCap ((GpCustomLineCap *) cap, &baseCap);
	assertEqualInt (status, Ok);
	assertEqualInt (baseCap, LineCapTriangle);

	// Negative value.
	GdipSetCustomLineCapBaseInset ((GpCustomLineCap *) cap, 120);
	GdipSetCustomLineCapBaseCap ((GpCustomLineCap *) cap, LineCapRound);
	status = GdipSetAdjustableArrowCapHeight (cap, -30);
	assertEqualInt (status, Ok);

	status = GdipGetAdjustableArrowCapHeight (cap, &height);
	assertEqualInt (status, Ok);
	assertEqualFloat (height, -30);

	status = GdipGetCustomLineCapBaseInset ((GpCustomLineCap *) cap, &baseInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (baseInset, -10);

	status = GdipGetCustomLineCapBaseCap ((GpCustomLineCap *) cap, &baseCap);
	assertEqualInt (status, Ok);
	assertEqualInt (baseCap, LineCapTriangle);

	// Negative tests.
	status = GdipSetAdjustableArrowCapHeight (NULL, 30);
	assertEqualInt (status, InvalidParameter);

	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);
}

static void test_getAdjustableArrowCapHeight ()
{
	GpStatus status;
	GpAdjustableArrowCap *cap;
	REAL height;

	GdipCreateAdjustableArrowCap (10, 11, TRUE, &cap);

	// Negative tests.
	status = GdipGetAdjustableArrowCapHeight (NULL, &height);
	assertEqualInt (status, InvalidParameter);

	status = GdipGetAdjustableArrowCapHeight (cap, NULL);
	assertEqualInt (status, InvalidParameter);

	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);
}

static void test_setAdjustableArrowCapWidth ()
{
	GpStatus status;
	GpAdjustableArrowCap *cap;
	REAL width;
	REAL baseInset;
	LineCap baseCap;

	GdipCreateAdjustableArrowCap (150, 3, TRUE, &cap);

	// Positive value.
	GdipSetCustomLineCapBaseCap ((GpCustomLineCap *) cap, LineCapRound);
	GdipSetCustomLineCapBaseInset ((GpCustomLineCap *) cap, 120);
	status = GdipSetAdjustableArrowCapWidth (cap, 30);
	assertEqualInt (status, Ok);

	status = GdipGetAdjustableArrowCapWidth (cap, &width);
	assertEqualInt (status, Ok);
	assertEqualFloat (width, 30);

	status = GdipGetCustomLineCapBaseInset ((GpCustomLineCap *) cap, &baseInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (baseInset, 5);

	status = GdipGetCustomLineCapBaseCap ((GpCustomLineCap *) cap, &baseCap);
	assertEqualInt (status, Ok);
	assertEqualInt (baseCap, LineCapTriangle);

	// Same value.
	GdipSetCustomLineCapBaseCap ((GpCustomLineCap *) cap, LineCapRound);
	GdipSetCustomLineCapBaseInset ((GpCustomLineCap *) cap, 120);
	status = GdipSetAdjustableArrowCapWidth (cap, 30);
	assertEqualInt (status, Ok);

	status = GdipGetAdjustableArrowCapWidth (cap, &width);
	assertEqualInt (status, Ok);
	assertEqualFloat (width, 30);

	status = GdipGetCustomLineCapBaseInset ((GpCustomLineCap *) cap, &baseInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (baseInset, 120);

	status = GdipGetCustomLineCapBaseCap ((GpCustomLineCap *) cap, &baseCap);
	assertEqualInt (status, Ok);
	assertEqualInt (baseCap, LineCapRound);

	// Zero.
	GdipSetCustomLineCapBaseCap ((GpCustomLineCap *) cap, LineCapRound);
	GdipSetCustomLineCapBaseInset ((GpCustomLineCap *) cap, 120);
	status = GdipSetAdjustableArrowCapWidth (cap, 0);
	assertEqualInt (status, Ok);

	status = GdipGetAdjustableArrowCapWidth (cap, &width);
	assertEqualInt (status, Ok);
	assertEqualFloat (width, 0);

	status = GdipGetCustomLineCapBaseInset ((GpCustomLineCap *) cap, &baseInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (baseInset, 0);

	status = GdipGetCustomLineCapBaseCap ((GpCustomLineCap *) cap, &baseCap);
	assertEqualInt (status, Ok);
	assertEqualInt (baseCap, LineCapTriangle);

	// Negative value.
	GdipSetCustomLineCapBaseInset ((GpCustomLineCap *) cap, 120);
	status = GdipSetAdjustableArrowCapWidth (cap, -30);
	assertEqualInt (status, Ok);

	status = GdipGetAdjustableArrowCapWidth (cap, &width);
	assertEqualInt (status, Ok);
	assertEqualFloat (width, -30);

	status = GdipGetCustomLineCapBaseInset ((GpCustomLineCap *) cap, &baseInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (baseInset, -5);

	// Negative tests.
	status = GdipSetAdjustableArrowCapWidth (NULL, 30);
	assertEqualInt (status, InvalidParameter);

	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);
}

static void test_getAdjustableArrowCapWidth ()
{
	GpStatus status;
	GpAdjustableArrowCap *cap;
	REAL width;

	GdipCreateAdjustableArrowCap (10, 11, TRUE, &cap);

	// Negative tests.
	status = GdipGetAdjustableArrowCapWidth (NULL, &width);
	assertEqualInt (status, InvalidParameter);

	status = GdipGetAdjustableArrowCapWidth (cap, NULL);
	assertEqualInt (status, InvalidParameter);

	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);
}

static void test_setAdjustableArrowCapMiddleInset ()
{
	GpStatus status;
	GpAdjustableArrowCap *cap;
	REAL middleInset;
	REAL baseInset;
	LineCap baseCap;

	GdipCreateAdjustableArrowCap (150, 3, TRUE, &cap);

	// Positive value.
	GdipSetCustomLineCapBaseInset ((GpCustomLineCap *) cap, 120);
	GdipSetCustomLineCapBaseCap ((GpCustomLineCap *) cap, LineCapRound);
	status = GdipSetAdjustableArrowCapMiddleInset (cap, 30);
	assertEqualInt (status, Ok);

	status = GdipGetAdjustableArrowCapMiddleInset (cap, &middleInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (middleInset, 30);

	status = GdipGetCustomLineCapBaseInset ((GpCustomLineCap *) cap, &baseInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (baseInset, 50);

	status = GdipGetCustomLineCapBaseCap ((GpCustomLineCap *) cap, &baseCap);
	assertEqualInt (status, Ok);
	assertEqualInt (baseCap, LineCapTriangle);

	// Same value.
	GdipSetCustomLineCapBaseInset ((GpCustomLineCap *) cap, 120);
	GdipSetCustomLineCapBaseCap ((GpCustomLineCap *) cap, LineCapRound);
	status = GdipSetAdjustableArrowCapMiddleInset (cap, 30);
	assertEqualInt (status, Ok);

	status = GdipGetAdjustableArrowCapMiddleInset (cap, &middleInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (middleInset, 30);

	status = GdipGetCustomLineCapBaseInset ((GpCustomLineCap *) cap, &baseInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (baseInset, 120);

	status = GdipGetCustomLineCapBaseCap ((GpCustomLineCap *) cap, &baseCap);
	assertEqualInt (status, Ok);
	assertEqualInt (baseCap, LineCapRound);

	// Zero.
	GdipSetCustomLineCapBaseInset ((GpCustomLineCap *) cap, 120);
	GdipSetCustomLineCapBaseCap ((GpCustomLineCap *) cap, LineCapRound);
	status = GdipSetAdjustableArrowCapMiddleInset (cap, 0);
	assertEqualInt (status, Ok);

	status = GdipGetAdjustableArrowCapMiddleInset (cap, &middleInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (middleInset, 0);

	status = GdipGetCustomLineCapBaseInset ((GpCustomLineCap *) cap, &baseInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (baseInset, 50);

	status = GdipGetCustomLineCapBaseCap ((GpCustomLineCap *) cap, &baseCap);
	assertEqualInt (status, Ok);
	assertEqualInt (baseCap, LineCapTriangle);

	// Negative value.
	GdipSetCustomLineCapBaseInset ((GpCustomLineCap *) cap, 120);
	GdipSetCustomLineCapBaseCap ((GpCustomLineCap *) cap, LineCapRound);
	status = GdipSetAdjustableArrowCapMiddleInset (cap, -30);
	assertEqualInt (status, Ok);

	status = GdipGetAdjustableArrowCapMiddleInset (cap, &middleInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (middleInset, -30);

	status = GdipGetCustomLineCapBaseInset ((GpCustomLineCap *) cap, &baseInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (baseInset, 50);

	status = GdipGetCustomLineCapBaseCap ((GpCustomLineCap *) cap, &baseCap);
	assertEqualInt (status, Ok);
	assertEqualInt (baseCap, LineCapTriangle);

	// Negative tests.
	status = GdipSetAdjustableArrowCapMiddleInset (NULL, 30);
	assertEqualInt (status, InvalidParameter);

	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);
}

static void test_getAdjustableArrowCapMiddleInset ()
{
	GpStatus status;
	GpAdjustableArrowCap *cap;
	REAL middleInset;

	GdipCreateAdjustableArrowCap (10, 11, TRUE, &cap);

	// Negative tests.
	status = GdipGetAdjustableArrowCapMiddleInset (NULL, &middleInset);
	assertEqualInt (status, InvalidParameter);

	status = GdipGetAdjustableArrowCapMiddleInset (cap, NULL);
	assertEqualInt (status, InvalidParameter);

	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);
}

static void test_setAdjustableArrowCapFillState ()
{
	GpStatus status;
	GpAdjustableArrowCap *cap;
	BOOL isFilled;
	REAL baseInset;
	LineCap baseCap;

	GdipCreateAdjustableArrowCap (10, 11, TRUE, &cap);

	// FALSE
	GdipSetCustomLineCapBaseInset ((GpCustomLineCap *) cap, 120);
	GdipSetCustomLineCapBaseCap ((GpCustomLineCap *) cap, LineCapRound);
	status = GdipSetAdjustableArrowCapFillState (cap, FALSE);
	assertEqualInt (status, Ok);

	status = GdipGetAdjustableArrowCapFillState (cap, &isFilled);
	assertEqualInt (status, Ok);
	assertFalse (isFilled);

	// TRUE
	GdipSetCustomLineCapBaseInset ((GpCustomLineCap *) cap, 120);
	GdipSetCustomLineCapBaseCap ((GpCustomLineCap *) cap, LineCapRound);
	status = GdipSetAdjustableArrowCapFillState (cap, TRUE);
	assertEqualInt (status, Ok);

	status = GdipGetAdjustableArrowCapFillState (cap, &isFilled);
	assertEqualInt (status, Ok);
	assertTrue (isFilled);

	// Same value
	GdipSetCustomLineCapBaseInset ((GpCustomLineCap *) cap, 120);
	GdipSetCustomLineCapBaseCap ((GpCustomLineCap *) cap, LineCapRound);
	status = GdipSetAdjustableArrowCapFillState (cap, TRUE);
	assertEqualInt (status, Ok);

	status = GdipGetAdjustableArrowCapFillState (cap, &isFilled);
	assertEqualInt (status, Ok);
	assertTrue (isFilled);

	status = GdipGetCustomLineCapBaseInset ((GpCustomLineCap *) cap, &baseInset);
	assertEqualInt (status, Ok);
	assertEqualFloat (baseInset, 120);

	status = GdipGetCustomLineCapBaseCap ((GpCustomLineCap *) cap, &baseCap);
	assertEqualInt (status, Ok);
	assertEqualInt (baseCap, LineCapRound);

	// Negative tests.
	status = GdipSetAdjustableArrowCapFillState (NULL, 30);
	assertEqualInt (status, InvalidParameter);

	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);
}

static void test_getAdjustableArrowCapFillState ()
{
	GpStatus status;
	GpAdjustableArrowCap *cap;
	BOOL isFilled;

	GdipCreateAdjustableArrowCap (10, 11, TRUE, &cap);

	// Negative tests.
	status = GdipGetAdjustableArrowCapFillState (NULL, &isFilled);
	assertEqualInt (status, InvalidParameter);

	status = GdipGetAdjustableArrowCapFillState (cap, NULL);
	assertEqualInt (status, InvalidParameter);

	GdipDeleteCustomLineCap ((GpCustomLineCap *) cap);
}

LIBGDIPLUS_TEST (testadjustablearrowcap, test_createAdjustableArrowCap)
LIBGDIPLUS_TEST (testadjustablearrowcap, test_cloneAdjustableArrowCap)
LIBGDIPLUS_TEST (testadjustablearrowcap, test_deleteAdjustableArrowCap)
LIBGDIPLUS_TEST (testadjustablearrowcap, test_setAdjustableArrowCapHeight)
LIBGDIPLUS_TEST (testadjustablearrowcap, test_getAdjustableArrowCapHeight)
LIBGDIPLUS_TEST (testadjustablearrowcap, test_setAdjustableArrowCapWidth)
LIBGDIPLUS_TEST (testadjustablearrowcap, test_getAdjustableArrowCapWidth)
LIBGDIPLUS_TEST (testadjustablearrowcap, test_setAdjustableArrowCapMiddleInset)
LIBGDIPLUS_TEST (testadjustablearrowcap, test_getAdjustableArrowCapMiddleInset)
LIBGDIPLUS_TEST (testadjustablearrowcap, test_setAdjustableArrowCapFillState)
LIBGDIPLUS_TEST (testadjustablearrowcap, test_getAdjustableArrowCapFillState)
