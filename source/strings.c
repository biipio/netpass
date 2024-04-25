#include "strings.h"

LanguageString str_loading = {
	{CFG_LANGUAGE_EN, "Loading     "},
	{CFG_LANGUAGE_DE, "Laden     "},
	{CFG_LANGUAGE_ES, 0},
	{CFG_LANGUAGE_PL, "Ładowanie     "}
};
LanguageString str_libcurl_error = {
	{CFG_LANGUAGE_EN, "libCURL error (%d): %s"},
	{CFG_LANGUAGE_EN, "libCURL fehler (%d): %s"},
	{CFG_LANGUAGE_ES, 0},
	{CFG_LANGUAGE_PL, "Błąd libCURL (%d): %s"},
};
LanguageString str_libcurl_date_and_time = {
	{CFG_LANGUAGE_EN, "Make sure that your systems date and time are set correctly!"},
	{CFG_LANGUAGE_DE, "Stelle sicher, dass die Systemszeit und -datum richtig gesetzt sind!"},
	{CFG_LANGUAGE_ES, 0},
	{CFG_LANGUAGE_PL, "Upewnij się, że data i czas systemowy jest ustawiony poprawnie!"},
};
LanguageString str_httpstatus_error = {
	{CFG_LANGUAGE_EN, "HTTP status code %d"},
	{CFG_LANGUAGE_DE, NULL},
	{CFG_LANGUAGE_ES, 0},
	{CFG_LANGUAGE_PL, "Błąd HTTP o kodzie %d"},
};
LanguageString str_3ds_error = {
	{CFG_LANGUAGE_EN, "3DS error code %08lx"},
	{CFG_LANGUAGE_DE, NULL},
	{CFG_LANGUAGE_ES, 0},
	{CFG_LANGUAGE_PL, "Błąd 3DS o kodzie %08lx"},
};
LanguageString str_at_home = {
	{CFG_LANGUAGE_EN, "You are at home."},
	{CFG_LANGUAGE_DE, "Du bist zuhause."},
	{CFG_LANGUAGE_ES, 0},
	{CFG_LANGUAGE_PL, "Jesteś w domu."},
};
LanguageString str_goto_train_station = {
	{CFG_LANGUAGE_EN, "Go to Train Station"},
	{CFG_LANGUAGE_DE, "Gehe zum Bahnhof"},
	{CFG_LANGUAGE_ES, 0},
	{CFG_LANGUAGE_PL, "Idź na Stacje Kolejową"},
};
LanguageString str_at_train_station = {
	{CFG_LANGUAGE_EN, "You are at the Train Station"},
	{CFG_LANGUAGE_DE, "Du bist am Bahnhof"},
	{CFG_LANGUAGE_ES, 0},
	{CFG_LANGUAGE_PL, "Jesteś na Stacji Kolejowej"},
};
LanguageString str_goto_plaza = {
	{CFG_LANGUAGE_EN, "Go to Plaza"},
	{CFG_LANGUAGE_DE, "Gehe zum Plaza"},
	{CFG_LANGUAGE_ES, 0},
	{CFG_LANGUAGE_PL, "Idź na Rynek"},
};
LanguageString str_at_plaza = {
	{CFG_LANGUAGE_EN, "You are at the Plaza"},
	{CFG_LANGUAGE_DE, "Du bist am Plaza"},
	{CFG_LANGUAGE_ES, 0},
	{CFG_LANGUAGE_PL, "Jesteś na Rynku"},
};
LanguageString str_goto_mall = {
	{CFG_LANGUAGE_EN, "Go to Mall"},
	{CFG_LANGUAGE_DE, "Gehe zum Einkaufszentrum"},
	{CFG_LANGUAGE_ES, 0},
	{CFG_LANGUAGE_PL, "Idź do Centrum Handlowego"},
};
LanguageString str_at_mall = {
	{CFG_LANGUAGE_EN, "You are at the Mall"},
	{CFG_LANGUAGE_DE, "Du bist im Einkaufszentrum"},
	{CFG_LANGUAGE_ES, 0},
	{CFG_LANGUAGE_PL, "Jesteś w Centrum Handlowym"},
};
LanguageString str_exit = {
	{CFG_LANGUAGE_EN, "Exit"},
	{CFG_LANGUAGE_DE, "Verlassen"},
	{CFG_LANGUAGE_ES, 0},
	{CFG_LANGUAGE_PL, "Wyjście"},
};

static u8 _language;

C2D_Font font_default;
C2D_Font font_local;

void stringsInit(void) {
	CFGU_GetSystemLanguage(&_language);
	printf("Got language %d\n", _language);
	font_default = C2D_FontLoadSystem(CFG_REGION_USA);
	if (_language == CFG_LANGUAGE_ZH) {
		font_local = C2D_FontLoadSystem(CFG_REGION_CHN);
	} else if (_language == CFG_LANGUAGE_KO) {
		font_local = C2D_FontLoadSystem(CFG_REGION_KOR);
	} else if (_language == CFG_LANGUAGE_TW) {
		font_local = C2D_FontLoadSystem(CFG_REGION_TWN);
	} else {
		font_local = font_default;
	}
}

const char* _s(LanguageString s) {
	for (int i = 0; i < NUM_LANGUAGES; i++) {
		if (s[i].language == _language && s[i].text) {
			return s[i].text;
		}
	}
	return s[0].text;
}

C2D_Font _font(LanguageString s) {
	for (int i = 0; i < NUM_LANGUAGES; i++) {
		if (s[i].language == _language && s[i].text) {
			return font_local;
		}
	}
	return font_default;
}

void TextLangParse(C2D_Text* staticText, C2D_TextBuf staticBuf, LanguageString s) {
	const char* text = 0;
	for (int i = 0; i < NUM_LANGUAGES; i++) {
		if (s[i].language == _language) {
			text = s[i].text;
		}
	}
	C2D_Font font = font_local;
	if (!text) {
		text = s[0].text;
		font = font_default;
	}
	C2D_TextFontParse(staticText, font, staticBuf, text);
}
