/*
 * Copyright (c) 2004 Ximian
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
 * and associated documentation files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial 
 * portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT 
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Author:
 *          Jordi Mas i Hernandez <jordi@ximian.com>, 2004
 */

#include "gdip.h"
#include "gdip_win32.h"
#include <math.h>
#include <glib.h>
#include <freetype/tttables.h>


/* Generic fonts */
static GpFontFamily* familySerif = NULL;
static GpFontFamily* familySansSerif = NULL;
static GpFontFamily* familyMonospace = NULL;
static int ref_familySerif = 0;
static int ref_familySansSerif = 0;
static int ref_familyMonospace = 0;


/* Family and collections font functions */

GpStatus
GdipNewInstalledFontCollection (GpFontCollection **font_collection)
{	
	
	FcObjectSet *os = FcObjectSetBuild (FC_FAMILY, FC_FOUNDRY, 0);
	FcPattern *pat = FcPatternCreate ();
	FcValue val;
	FcFontSet *col;
	GpFontCollection *result;
	    
	if (!font_collection)
		return InvalidParameter;
    
	/* Only Scalable fonts for now */
	val.type = FcTypeBool;
	val.u.b = FcTrue;
	FcPatternAdd (pat, FC_SCALABLE, val, TRUE);
	FcObjectSetAdd (os, FC_SCALABLE);

	col =  FcFontList (0, pat, os);
	FcPatternDestroy (pat);
	FcObjectSetDestroy (os);
    
	result = (GpFontCollection *) GdipAlloc (sizeof (GpFontCollection));
	result->fontset = col;
	result->config = NULL;
	*font_collection = result;

	return Ok;
}

GpStatus
GdipNewPrivateFontCollection (GpFontCollection **font_collection)
{
	GpFontCollection *result;

	if (!font_collection)
		return InvalidParameter;

	result = (GpFontCollection *) GdipAlloc (sizeof (GpFontCollection));
	result->fontset = NULL;
	result->config = FcConfigCreate ();
    
	*font_collection = result;
	return Ok;
}

GpStatus
GdipDeletePrivateFontCollection (GpFontCollection **font_collection)
{
	if (!font_collection)
		return InvalidParameter;

	if (*font_collection) {
		FcFontSetDestroy ((*font_collection)->fontset);
		FcConfigDestroy ((*font_collection)->config);
		GdipFree ((void *)font_collection);
	}

	return Ok;
}

GpStatus
GdipPrivateAddFontFile (GpFontCollection *font_collection,  GDIPCONST WCHAR *filename)
{
	unsigned char *file;
	
	if (!font_collection || !filename)
		return InvalidParameter;
    
	file = (unsigned char*) g_utf16_to_utf8 ((const gunichar2 *)filename, -1,
						 NULL, NULL, NULL);
	
	FcConfigAppFontAddFile (font_collection->config, file);
    
	g_free (file);
	return Ok;
}


GpStatus 
GdipDeleteFontFamily (GpFontFamily *fontFamily)
{
	bool delete = TRUE;
	
	if (!fontFamily)
		return Ok;
		
	if (fontFamily == familySerif) {
		ref_familySerif--;
		if (ref_familySerif)
			delete = FALSE;
	}
	
	if (fontFamily == familySansSerif) {
		ref_familySansSerif--;
		if (ref_familySansSerif)
			delete = FALSE;
	}	
	
	if (fontFamily == familyMonospace) {
		ref_familyMonospace--;
		if (ref_familyMonospace)
			delete = FALSE;
	}
	
	
	if (delete) {
		FcPatternDestroy (fontFamily->pattern);
		GdipFree (fontFamily);
	}
	
	return Ok;
}

void
gdip_createPrivateFontSet (GpFontCollection *font_collection)
{
	FcObjectSet *os = FcObjectSetBuild (FC_FAMILY, FC_FOUNDRY, 0);
	FcPattern *pat = FcPatternCreate ();
	FcFontSet *col =  FcFontList (font_collection->config, pat, os);
    
	if (font_collection->fontset)
		FcFontSetDestroy (font_collection->fontset);

	FcPatternDestroy (pat);
	FcObjectSetDestroy (os);

	font_collection->fontset = col;
}

GpStatus
GdipGetFontCollectionFamilyCount (GpFontCollection *font_collection, int *numFound)
{
	if (!font_collection  || !numFound)
		return InvalidParameter;

	if (font_collection->config)
		gdip_createPrivateFontSet (font_collection);

	if (font_collection->fontset)
		*numFound = font_collection->fontset->nfont;
	else
		*numFound = 0;

	return Ok;
}

GpStatus
GdipGetFontCollectionFamilyList (GpFontCollection *font_collection, int num_sought, GpFontFamily **gpfamilies, int *num_found)
{
	GpFontFamily **gpfam = gpfamilies;
	FcPattern **pattern =  font_collection->fontset->fonts;
	int i;
	
	if (!font_collection || !gpfamilies || !num_found)
		return InvalidParameter;

	if (font_collection->config)
		gdip_createPrivateFontSet (font_collection);
		

	for (i = 0; i < font_collection->fontset->nfont; gpfam++, pattern++, i++) {
		*gpfam = (GpFontFamily *) GdipAlloc (sizeof (GpFontFamily));
		(*gpfam)->pattern = *pattern;
		(*gpfam)->allocated = FALSE;
	}

	*num_found = font_collection->fontset->nfont;
	return Ok;  
}

GpStatus
GdipCreateFontFamilyFromName (GDIPCONST WCHAR *name, GpFontCollection *font_collection, GpFontFamily **fontFamily)
{
	glong items_read = 0;
	glong items_written = 0;
	unsigned char *string;
	FcPattern **gpfam;
	FcChar8 *str;
	int i;
	
	if (!name || !fontFamily)
		return InvalidParameter;

	string = (unsigned char*)g_utf16_to_utf8 ((const gunichar2 *)name, -1,
						  &items_read, &items_written, NULL);

	if (!font_collection) {
		FcChar8 *str;
		FcPattern *pat = FcPatternCreate ();
		FcResult rlt;
		FcResult r;
		
		/* find the family we want */
		FcValue val;
                                   
		val.type = FcTypeString;
		val.u.s = string;
		FcPatternAdd (pat, FC_FAMILY, val, TRUE);

		FcConfigSubstitute (0, pat, FcMatchPattern);
		FcDefaultSubstitute (pat);                  
		
		*fontFamily = (GpFontFamily *) GdipAlloc (sizeof (GpFontFamily));
		(*fontFamily)->pattern =  FcFontMatch (0, pat, &rlt);
		(*fontFamily)->allocated = TRUE;

		r = FcPatternGetString ((*fontFamily)->pattern, FC_FAMILY, 0, &str);
		g_free (string);
		FcPatternDestroy (pat);
		return Ok;
	}

	gpfam = font_collection->fontset->fonts;
    
	for (i=0; i < font_collection->fontset->nfont; gpfam++, i++){
		FcResult r = FcPatternGetString (*gpfam, FC_FAMILY, 0, &str);

		if (strcmp (string, str)==0) {
			*fontFamily = (GpFontFamily *) GdipAlloc (sizeof (GpFontFamily));
			(*fontFamily)->pattern = *gpfam;
			(*fontFamily)->allocated = FALSE;
			g_free (string);
			return Ok;
		}
	}

	g_free (string);
	return FontFamilyNotFound;
}


GpStatus
GdipGetFamilyName (GDIPCONST GpFontFamily *family, WCHAR name[LF_FACESIZE], int language)
{                
	FcChar8 *fc_str;
	glong items_read = 0;
	glong items_written = 0;
	FcResult r;
	gunichar2 *string;
	
	if (!family)
		return InvalidParameter;

	r = FcPatternGetString (family->pattern, FC_FAMILY, 0, &fc_str);

	string =  g_utf8_to_utf16 ((const gchar *)fc_str, -1, &items_read, &items_written,NULL);

	if (items_written>= (LF_FACESIZE-1))
		items_written= (LF_FACESIZE-1);

	memcpy (name, string, items_written * sizeof (WCHAR));
	name [1+items_written*sizeof (WCHAR)]=0;

	g_free (string);

	return Ok;
}


GpStatus
GdipGetGenericFontFamilySansSerif (GpFontFamily **nativeFamily)
{
	const WCHAR MSSansSerif[] = {'M','S',' ','S','a','n','s',' ', 'S','e','r','i','f', 0};
	
	if (!familySansSerif) 
		GdipCreateFontFamilyFromName (MSSansSerif, NULL, &familySansSerif);    
	
	ref_familySansSerif++;
	*nativeFamily = familySansSerif;    
	return Ok;
}

GpStatus
GdipGetGenericFontFamilySerif (GpFontFamily **nativeFamily)
{
	const WCHAR Serif[] = {'S','e','r','i','f', 0};
	
	if (!familySerif)
		GdipCreateFontFamilyFromName (Serif, NULL, &familySerif);
	
	ref_familySerif++;	
	*nativeFamily = familySerif;    
	return Ok;
}

GpStatus
GdipGetGenericFontFamilyMonospace (GpFontFamily **nativeFamily)
{
	const WCHAR Serif[] = {'S','e','r','i','f', 0};
	
	if (!familyMonospace)
		GdipCreateFontFamilyFromName (Serif, NULL, &familyMonospace);    
		
	ref_familyMonospace++;
	*nativeFamily = familyMonospace;    
	return Ok;
}

GpStatus
GdipGetEmHeight (GDIPCONST GpFontFamily *family, GpFontStyle style, short *EmHeight)
{
	short rslt = 0;
	GpFont *font = NULL;

	if (!family || !EmHeight)
		return InvalidParameter;

	GdipCreateFont (family, 0.0f, style, UnitPoint, &font);

	if (font) {
		FT_Face	face;

		face = cairo_ft_font_face(font->cairofnt);

		TT_VertHeader *pVert = FT_Get_Sfnt_Table (face, ft_sfnt_vhea);
		if (pVert)
			rslt = pVert->yMax_Extent;
		else if (face)
			rslt = face->units_per_EM;
		else
			rslt = 0;
		GdipDeleteFont (font);
	}

	*EmHeight = rslt;
	return Ok;
}

GpStatus
GdipGetCellAscent (GDIPCONST GpFontFamily *family, GpFontStyle style, short *CellAscent)
{
	short rslt = 0;
	GpFont *font = NULL;

	if (!family || !CellAscent)
		return InvalidParameter;

	GdipCreateFont (family, 0.0f, style, UnitPoint, &font);

	if (font){
                FT_Face face;

                face = cairo_ft_font_face(font->cairofnt);

		TT_HoriHeader *pHori = FT_Get_Sfnt_Table (face, ft_sfnt_hhea);

		if (pHori)
			rslt = pHori->Ascender;

		GdipDeleteFont (font);
	}

	*CellAscent = rslt;
	return Ok;         
}

GpStatus
GdipGetCellDescent (GDIPCONST GpFontFamily *family, GpFontStyle style, short *CellDescent)
{
	short rslt = 0;
	GpFont *font = NULL;

	if (!family || !CellDescent)
		return InvalidParameter;

	*CellDescent = 0;

	GdipCreateFont (family, 0.0f, style, UnitPoint, &font);

	if (font){
                FT_Face face;

                face = cairo_ft_font_face(font->cairofnt);

		TT_HoriHeader *pHori = FT_Get_Sfnt_Table (face, ft_sfnt_hhea);

		if (pHori)
			rslt = -pHori->Descender;

		GdipDeleteFont (font);
	}

	*CellDescent = rslt;
	return Ok;         
}

GpStatus
GdipGetLineSpacing (GDIPCONST GpFontFamily *family, GpFontStyle style, short *LineSpacing)
{
	short rslt = 0;
	GpFont *font = NULL;

	if (!family || !LineSpacing)
		return InvalidParameter;

	GdipCreateFont (family, 0.0f, style, UnitPoint, &font);

	if (font){
                FT_Face face;

                face = cairo_ft_font_face(font->cairofnt);

		TT_HoriHeader *pHori = FT_Get_Sfnt_Table (face, ft_sfnt_hhea);
		if (pHori)
			rslt = pHori->Ascender + (-pHori->Descender) + pHori->Line_Gap;
		else if (face)
			rslt = face->height;
		else
			rslt = 0;
		GdipDeleteFont (font);
	}

	*LineSpacing = rslt;
	return Ok;
}

GpStatus
GdipIsStyleAvailable (GDIPCONST GpFontFamily *family, int style, BOOL *IsStyleAvailable)
{
	if (!family || !IsStyleAvailable)
		return InvalidParameter;

	*IsStyleAvailable = TRUE;
	return Ok;    
}



/* Font functions */

int
gdip_font_create (const unsigned char *family, int fcslant, int fcweight, GpFont *result)
{
	cairo_font_t *font = NULL;
	FcPattern * pat = NULL;
	FT_Library ft_library;
	FT_Error error;

	pat = FcPatternCreate ();
	if (pat == NULL || result == NULL) {
		return 0;
	}

	FcPatternAddString (pat, FC_FAMILY, family);
	FcPatternAddInteger (pat, FC_SLANT, fcslant);
	FcPatternAddInteger (pat, FC_WEIGHT, fcweight);

	error = FT_Init_FreeType (&ft_library);
	if (error) {
		FcPatternDestroy (pat);
		return 0;
	}

	font = cairo_ft_font_create (ft_library, pat);
	if (font == NULL) {
		FT_Done_FreeType(ft_library);
		FcPatternDestroy (pat);
		return 0;
	}

	result->cairofnt = font;
	result->ft_library = ft_library;

	FT_Set_Char_Size (cairo_ft_font_face(font),
			  DOUBLE_TO_26_6 (1.0),
			  DOUBLE_TO_26_6 (1.0),
			  0, 0);

	FcPatternDestroy (pat);
	return 1;
}

void
gdip_font_drawunderline (GpGraphics *graphics, GpBrush *brush, float x, float y, float width)
{
        float pos, size;
        cairo_font_extents_t extents;

        cairo_current_font_extents (graphics->ct, &extents);
        pos = 0.5 + ((extents.ascent + extents.descent) *0.1);
        size = 0.5 + ((extents.ascent + extents.descent) *0.05);

        GdipFillRectangle (graphics, brush, x, y +pos, width, size);     
}

void
gdip_font_drawstrikeout (GpGraphics *graphics, GpBrush *brush, float x, float y, float width)
{
        float pos, size;
        cairo_font_extents_t extents;

        cairo_current_font_extents (graphics->ct, &extents);
        pos = 0.5 + ((extents.ascent + extents.descent) *0.5);
        size = 0.5 + ((extents.ascent + extents.descent) *0.05);

        GdipFillRectangle (graphics, brush, x, y -pos, width, size);
}

GpStatus
GdipCreateFont (GDIPCONST GpFontFamily* family, float emSize, GpFontStyle style, Unit unit,  GpFont **font)
{
	FcChar8* str;
	FcResult r;
	GpFont *result;
	int slant = 0;
	int weight = FC_WEIGHT_LIGHT;
	
	if (!family || !font)
		return InvalidParameter;

	r = FcPatternGetString (family->pattern, FC_FAMILY, 0, &str);

	result = (GpFont *) GdipAlloc (sizeof (GpFont));

	gdip_unitConversion (unit, UnitPixel, emSize, &result->sizeInPixels);

        if ((style & FontStyleBold) == FontStyleBold)
                weight = FC_WEIGHT_BOLD;

        if ((style & FontStyleItalic) == FontStyleItalic)        
                slant = FC_SLANT_ITALIC;
        
	if (!gdip_font_create (str, slant, weight, result)) {
		return InvalidParameter;	// FIXME -  wrong return code
	}
        result->style = style;
	cairo_font_reference ((cairo_font_t *)result->cairofnt);
	result->wineHfont=CreateWineFont(str, style, emSize, unit);
	*font=result;
        
	return Ok;
}

GpStatus
GdipGetHfont(GpFont* font, void **Hfont)
{
	if (font) {
		*Hfont=font->wineHfont;
		return(Ok);
	}
	return InvalidParameter;
}

GpStatus
GdipDeleteFont (GpFont* font)
{
	if (font) {
		cairo_font_destroy ((cairo_font_t *)font->cairofnt);
		if (font->ft_library) {
			FT_Done_FreeType(font->ft_library);
		}
		if (font->wineHfont) {
			DeleteWineFont(font->wineHfont);
		}
		GdipFree ((void *)font);
                return Ok;
	}
        return InvalidParameter;
}

static GpStatus
CreateFontFromHDCorHfont(void *hdcIn, void *hfont, GpFont **font, LOGFONTA *lf)
{
	FcResult	r;
	GpFont		*result;
	int		slant	= 0;
	int		weight	= FC_WEIGHT_LIGHT;
	int		style	= 0;
	TEXTMETRICA	tm;
	unsigned char	FaceName[33];
	void		*hdc;
	void		*oldFont;
	
	if (!font) {
		return InvalidParameter;
	}

	if (!hdcIn) {
		hdc=GetDC_pfn(NULL);
		oldFont=SelectObject_pfn(hdc, hfont);
	} else {
		hdc=hdcIn;
		oldFont=0;	/* Make the compiler happy, we don't care about oldFont in this path */
	}
	SetMapMode_pfn(hdc, 1);
	if (GetTextMetrics_pfn(hdc, &tm)==0 || GetTextFace_pfn(hdc, sizeof(FaceName), FaceName)==0) {
		ReleaseDC_pfn(NULL, hdc);
		return InvalidParameter;
	}
	if (!hdcIn) {
		SelectObject_pfn(hdc, oldFont);
		ReleaseDC_pfn(NULL, hdc);
	}

	result = (GpFont *) GdipAlloc (sizeof (GpFont));

	if (tm.tmHeight<0) {
	   result->sizeInPixels=tm.tmHeight*-1;
	} else {
	   result->sizeInPixels=tm.tmHeight;
	}

	if (tm.tmStruckOut!=0) {
		style |= FontStyleStrikeout;
	}

	if (tm.tmUnderlined!=0) {
		style |= FontStyleUnderline;
	}

	if (tm.tmItalic!=0) {
		style |= FontStyleItalic;
		slant = FC_SLANT_ITALIC;
	}

	if (tm.tmWeight>400) {
		style |= FontStyleBold;
		weight = FC_WEIGHT_BOLD;
	}

	if (!gdip_font_create (FaceName, slant, weight, result)) {
		return InvalidParameter;	// FIXME - pick right return code
	}
	
	result->style = style;
	cairo_font_reference ((cairo_font_t *)result->cairofnt);
	result->wineHfont=CreateWineFont(FaceName, style, result->sizeInPixels, UnitPixel);

	/* Assign our results */
	*font=result;
	if (lf) {
		lf->lfHeight=result->sizeInPixels;
		lf->lfItalic=tm.tmItalic;
		lf->lfStrikeOut=tm.tmStruckOut;
		lf->lfUnderline=tm.tmUnderlined;
		lf->lfWeight=tm.tmWeight;
		strcpy(lf->lfFaceName, FaceName);
	}

	return Ok;
}

GpStatus
GdipCreateFontFromDC(void *hdc, GpFont **font)
{
	return(CreateFontFromHDCorHfont(hdc, NULL, font, NULL));
}

GpStatus
GdipCreateFontFromHfont(void *hfont, GpFont **font, LOGFONTA *lf)
{
	return(CreateFontFromHDCorHfont(NULL, hfont, font, lf));
}

GpStatus
GdipGetLogFontA(GpFont *font, GpGraphics *graphics, LOGFONTA *lf)
{
	void		*hdc;
	void		*oldFont;
	TEXTMETRICA	tm;
	unsigned char	FaceName[33];

	if (!font || !lf) {
		return(InvalidParameter);
	}

	hdc=GetDC_pfn(NULL);
	oldFont=SelectObject_pfn(hdc, font->wineHfont);

	if (GetTextMetrics_pfn(hdc, &tm)==0 || GetTextFace_pfn(hdc, sizeof(FaceName), FaceName)==0) {
		SelectObject_pfn(hdc, oldFont);
		ReleaseDC_pfn(NULL, hdc);
		return(Win32Error);
	}

	/* We assign what we know and default the rest */
	lf->lfHeight=font->sizeInPixels;
	lf->lfWidth=tm.tmAveCharWidth;
	lf->lfEscapement=0;
	lf->lfOrientation=0;
	lf->lfWeight=tm.tmWeight;
	lf->lfItalic=tm.tmItalic;
	lf->lfUnderline=tm.tmUnderlined;
	lf->lfStrikeOut=tm.tmStruckOut;
	lf->lfCharSet=tm.tmCharSet;
	lf->lfOutPrecision=0;		/* 0 = OUT_DEFAULT_PRECIS */
	lf->lfClipPrecision=0;		/* 0 = CLIP_DEFAULT_PRECIS */
	lf->lfQuality=4;		/* 4 = ANTIALIASED_QUALITY */
	lf->lfPitchAndFamily=tm.tmPitchAndFamily;
	strcpy(lf->lfFaceName, FaceName);

	/* Clean up */
	SelectObject_pfn(hdc, oldFont);
	ReleaseDC_pfn(NULL, hdc);

	return(Ok);
}

GpStatus
GdipPrivateAddMemoryFont(GpFontCollection *fontCollection, GDIPCONST void *memory, int length)
{
	char	*fontfile;
	FILE	*f;

	fontfile=tempnam(NULL, NULL);
	if (!fontfile) {
		return(OutOfMemory);
	}

	f=fopen(fontfile, "wb");
	if (!f) {
		free(fontfile);
		return(GenericError);
	}

	if (fwrite(memory, 1, length, f)!=length) {
		fclose(f);
		free(fontfile);
		return(GenericError);
	}
	fclose(f);

	FcConfigAppFontAddFile(fontCollection->config, fontfile);

	/* FIXME - May we delete our temporary font file or does 
	   FcConfigAppFontAddFile just reference our file?  */

	free(fontfile);
	return(Ok);
}
