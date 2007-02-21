/*
 *  - godori <ghcstop>, www.aesop-embedded.org
 *
 *    => Created. Jan, 2001
 *
 */
#include "gfxdev.h"


// for font load

extern Engfont gulim_96_10_eng;

static const struct english_fonttable {
    const char *name;
    Engfont *font;
} english_fontaliastable[] = {
	{ "default",	&gulim_96_10_eng	},
	{ "gulim_96_10_eng",	&gulim_96_10_eng	},
	{ 0,		0		}
};

extern Hanfont gulim_96_10_han;

static const struct hangul_fonttable {
    const char *name;
    Hanfont *font;
} hangul_fontaliastable[] = {
	{ "default",	&gulim_96_10_han	},
	{ "gulim_96_10_han",	&gulim_96_10_han	},
	{ 0,		0 	}
};

static Engfont *default_Efont = 0;
static Hanfont *default_Hfont = 0;

int gfx_init_context(PSD psd)
{
    if(default_Efont == 0 && (default_Efont = gfx_eng_load_font("default")) == 0)
    {
       printf("gfx_init_context(): eng font load error\n");
       return -1;
    }

    if(default_Hfont == 0 && (default_Hfont = gfx_han_load_font("default")) == 0)
    {
       printf("gfx_init_context(): eng font load error\n");
       return -1;
    }

    psd->hfont = default_Hfont;
    psd->efont = default_Efont;
    psd->fg = RGB2PIXEL(192, 192, 192);
    psd->bg = RGB2PIXEL(64, 64, 64);

    psd->curx = 0;
    psd->cury = 0;

    psd->mtext = gfx_mixed_text;
    psd->htext = gfx_han_text;
    psd->etext = gfx_eng_text;
    psd->hputc = gfx_han_putc;
    psd->eputc = gfx_eng_putc;

    printf("init draw text context\n");

    return 0;
}


Engfont *gfx_eng_load_font(char *fontname)
{
    const struct english_fonttable *ft;

    for (ft = english_fontaliastable; ft->name != 0; ++ft)
    {
	    if( strcmp(ft->name, fontname ) == 0)
	    return ft->font;
    }

    printf("input_eng_font_name error: %s\n", fontname);
    return 0;
}

void gfx_eng_free_font(Engfont *font)
{
}

Hanfont *gfx_han_load_font(char *fontname)
{
    const struct hangul_fonttable *ft;

    for (ft = hangul_fontaliastable; ft->name != 0; ++ft)
    {
	if( strcmp(ft->name, fontname ) == 0)
	    return ft->font;
    }

    printf("input_han_font_name error: %s\n", fontname);
    return 0;
}

void gfx_han_free_font(Hanfont *font)
{
}
