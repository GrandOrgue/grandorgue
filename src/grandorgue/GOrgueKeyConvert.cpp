/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GOrgueKeyConvert.h"

static GOShortcutKey shortcuts[] = {
	{ wxTRANSLATE("back"), 8 },
	{ wxTRANSLATE("tab"), 9 },
	{ wxTRANSLATE("return"), 13 },
	{ wxTRANSLATE("alt"), 17 },
	{ wxTRANSLATE("control"), 18 },
	{ wxTRANSLATE("menu"), 19 },
	{ wxTRANSLATE("pause"), 20 },
	{ wxTRANSLATE("capital"), 21 },
	{ wxTRANSLATE("escape"), 27 },
	{ wxTRANSLATE("space"), 32 },
	{ wxTRANSLATE("pageup"), 33 },
	{ wxTRANSLATE("pagedown"), 34 },
	{ wxTRANSLATE("end"), 35 },
	{ wxTRANSLATE("home"), 36 },
	{ wxTRANSLATE("left"), 37 },
	{ wxTRANSLATE("up"), 38 },
	{ wxTRANSLATE("right"), 39 },
	{ wxTRANSLATE("down"), 40 },
	{ wxTRANSLATE("select"), 41 },
	{ wxTRANSLATE("print"), 42 },
	{ wxTRANSLATE("execute"), 43 },
	{ wxTRANSLATE("snapshot"), 44 },
	{ wxTRANSLATE("insert"), 45 },
	{ wxTRANSLATE("delete"), 46 },
	{ wxTRANSLATE("help"), 47 },
	{ wxTRANSLATE("0"), 48 },
	{ wxTRANSLATE("1"), 49 },
	{ wxTRANSLATE("2"), 50 },
	{ wxTRANSLATE("3"), 51 },
	{ wxTRANSLATE("4"), 52 },
	{ wxTRANSLATE("5"), 53 },
	{ wxTRANSLATE("6"), 54 },
	{ wxTRANSLATE("7"), 55 },
	{ wxTRANSLATE("8"), 56 },
	{ wxTRANSLATE("9"), 57 },
	{ wxTRANSLATE("A"), 65 },
	{ wxTRANSLATE("B"), 66 },
	{ wxTRANSLATE("C"), 67 },
	{ wxTRANSLATE("D"), 68 },
	{ wxTRANSLATE("E"), 69 },
	{ wxTRANSLATE("F"), 70 },
	{ wxTRANSLATE("G"), 71 },
	{ wxTRANSLATE("H"), 72 },
	{ wxTRANSLATE("I"), 73 },
	{ wxTRANSLATE("J"), 74 },
	{ wxTRANSLATE("K"), 75 },
	{ wxTRANSLATE("L"), 76 },
	{ wxTRANSLATE("M"), 77 },
	{ wxTRANSLATE("N"), 78 },
	{ wxTRANSLATE("O"), 79 },
	{ wxTRANSLATE("P"), 80 },
	{ wxTRANSLATE("Q"), 81 },
	{ wxTRANSLATE("R"), 82 },
	{ wxTRANSLATE("S"), 83 },
	{ wxTRANSLATE("T"), 84 },
	{ wxTRANSLATE("U"), 85 },
	{ wxTRANSLATE("V"), 86 },
	{ wxTRANSLATE("W"), 87 },
	{ wxTRANSLATE("X"), 88 },
	{ wxTRANSLATE("Y"), 89 },
	{ wxTRANSLATE("Z"), 90 },
	{ wxTRANSLATE("Windows left"), 91 },
	{ wxTRANSLATE("Windows right"), 92 },
	{ wxTRANSLATE("Windows menu"), 93 },
	{ wxTRANSLATE("numpad 0"), 96 },
	{ wxTRANSLATE("numpad 1"), 97 },
	{ wxTRANSLATE("numpad 2"), 98 },
	{ wxTRANSLATE("numpad 3"), 99 },
	{ wxTRANSLATE("numpad 4"), 100 },
	{ wxTRANSLATE("numpad 5"), 101 },
	{ wxTRANSLATE("numpad 6"), 102 },
	{ wxTRANSLATE("numpad 7"), 103 },
	{ wxTRANSLATE("numpad 8"), 104 },
	{ wxTRANSLATE("numpad 9"), 105 },
	{ wxTRANSLATE("multiply"), 106 },
	{ wxTRANSLATE("add"), 107 },
	{ wxTRANSLATE("seperator"), 108 },
	{ wxTRANSLATE("subtract"), 109 },
	{ wxTRANSLATE("decimal"), 110 },
	{ wxTRANSLATE("divide"), 111 },
	{ wxTRANSLATE("F1"), 112 },
	{ wxTRANSLATE("F2"), 113 },
	{ wxTRANSLATE("F3"), 114 },
	{ wxTRANSLATE("F4"), 115 },
	{ wxTRANSLATE("F5"), 116 },
	{ wxTRANSLATE("F6"), 117 },
	{ wxTRANSLATE("F7"), 118 },
	{ wxTRANSLATE("F8"), 119 },
	{ wxTRANSLATE("F9"), 120 },
	{ wxTRANSLATE("F10"), 121 },
	{ wxTRANSLATE("F11"), 122 },
	{ wxTRANSLATE("F12"), 123 },
	{ wxTRANSLATE("F13"), 124 },
	{ wxTRANSLATE("F14"), 125 },
	{ wxTRANSLATE("F15"), 126 },
	{ wxTRANSLATE("F16"), 127 },
	{ wxTRANSLATE("F17"), 128 },
	{ wxTRANSLATE("F18"), 129 },
	{ wxTRANSLATE("F19"), 130 },
	{ wxTRANSLATE("F20"), 131 },
	{ wxTRANSLATE("F21"), 132 },
	{ wxTRANSLATE("F22"), 133 },
	{ wxTRANSLATE("F23"), 134 },
	{ wxTRANSLATE("F24"), 135 },
	{ wxTRANSLATE("numlock"), 144 },
	{ wxTRANSLATE("scroll"), 145 },
	{ wxTRANSLATE(":"), 186 },
	{ wxTRANSLATE("numpad equal"), 187 },
	{ wxTRANSLATE(","), 188 },
	{ wxTRANSLATE("_"), 189 },
	{ wxTRANSLATE("."), 190 },
	{ wxTRANSLATE("?"), 191 },
	{ wxTRANSLATE("\""), 192 },
	{ wxTRANSLATE("{"), 219 },
	{ wxTRANSLATE("|"), 220 },
	{ wxTRANSLATE("}"), 221 },
	{ wxTRANSLATE("#"), 222 },
	{ wxTRANSLATE("`"), 223 },
};

unsigned GetShortcutKeyCount()
{
	return sizeof(shortcuts)/sizeof(shortcuts[0]);
}

const GOShortcutKey* GetShortcutKeys()
{
	return shortcuts;
}

int WXKtoVK(int what)
{
	switch(what)
	{
	case WXK_BACK:
		return 8;
	case WXK_TAB:
	case WXK_NUMPAD_TAB:
		return 9;
	case WXK_RETURN:
	case WXK_NUMPAD_ENTER:
		return 13;
	case WXK_ALT:
		return 17;
	case WXK_CONTROL:
		return 18;
	case WXK_MENU:
		return 19;
	case WXK_PAUSE:
		return 20;
	case WXK_CAPITAL:
		return 21;
	case WXK_ESCAPE:
		return 27;
	case WXK_SPACE:
	case WXK_NUMPAD_SPACE:
		return 32;
	case WXK_PAGEUP:
	case WXK_NUMPAD_PAGEUP:
		return 33;
	case WXK_PAGEDOWN:
	case WXK_NUMPAD_PAGEDOWN:
		return 34;
	case WXK_END:
	case WXK_NUMPAD_END:
		return 35;
	case WXK_HOME:
	case WXK_NUMPAD_HOME:
		return 36;
	case WXK_LEFT:
	case WXK_NUMPAD_LEFT:
		return 37;
	case WXK_UP:
	case WXK_NUMPAD_UP:
		return 38;
	case WXK_RIGHT:
	case WXK_NUMPAD_RIGHT:
		return 39;
	case WXK_DOWN:
	case WXK_NUMPAD_DOWN:
		return 40;
	case WXK_SELECT:
		return 41;
	case WXK_PRINT:
		return 42;
	case WXK_EXECUTE:
		return 43;
	case WXK_SNAPSHOT:
		return 44;
	case WXK_INSERT:
	case WXK_NUMPAD_INSERT:
		return 45;
	case WXK_NUMPAD_DELETE:
		return 46;
	case WXK_HELP:
		return 47;
	case '0':
		return 48;
	case '1':
		return 49;
	case '2':
		return 50;
	case '3':
		return 51;
	case '4':
		return 52;
	case '5':
		return 53;
	case '6':
		return 54;
	case '7':
		return 55;
	case '8':
		return 56;
	case '9':
		return 57;
	case 'A':
		return 65;
	case 'B':
		return 66;
	case 'C':
		return 67;
	case 'D':
		return 68;
	case 'E':
		return 69;
	case 'F':
		return 70;
	case 'G':
		return 71;
	case 'H':
		return 72;
	case 'I':
		return 73;
	case 'J':
		return 74;
	case 'K':
		return 75;
	case 'L':
		return 76;
	case 'M':
		return 77;
	case 'N':
		return 78;
	case 'O':
		return 79;
	case 'P':
		return 80;
	case 'Q':
		return 81;
	case 'R':
		return 82;
	case 'S':
		return 83;
	case 'T':
		return 84;
	case 'U':
		return 85;
	case 'V':
		return 86;
	case 'W':
		return 87;
	case 'X':
		return 88;
	case 'Y':
		return 89;
	case 'Z':
		return 90;
	case WXK_WINDOWS_LEFT:
		return 91;
	case WXK_WINDOWS_RIGHT:
		return 92;
	case WXK_WINDOWS_MENU:
		return 93;
	case WXK_NUMPAD0:
		return 96;
	case WXK_NUMPAD1:
		return 97;
	case WXK_NUMPAD2:
		return 98;
	case WXK_NUMPAD3:
		return 99;
	case WXK_NUMPAD4:
		return 100;
	case WXK_NUMPAD5:
		return 101;
	case WXK_NUMPAD6:
		return 102;
	case WXK_NUMPAD7:
		return 103;
	case WXK_NUMPAD8:
		return 104;
	case WXK_NUMPAD9:
		return 105;
	case WXK_MULTIPLY:
	case WXK_NUMPAD_MULTIPLY:
		return 106;
	case WXK_ADD:
	case WXK_NUMPAD_ADD:
		return 107;
	case WXK_SEPARATOR:
	case WXK_NUMPAD_SEPARATOR:
		return 108;
	case WXK_SUBTRACT:
	case WXK_NUMPAD_SUBTRACT:
		return 109;
	case WXK_DECIMAL:
	case WXK_NUMPAD_DECIMAL:
		return 110;
	case WXK_DIVIDE:
	case WXK_NUMPAD_DIVIDE:
		return 111;
	case WXK_F1:
	case WXK_NUMPAD_F1:
		return 112;
	case WXK_F2:
	case WXK_NUMPAD_F2:
		return 113;
	case WXK_F3:
	case WXK_NUMPAD_F3:
		return 114;
	case WXK_F4:
	case WXK_NUMPAD_F4:
		return 115;
	case WXK_F5:
		return 116;
	case WXK_F6:
		return 117;
	case WXK_F7:
		return 118;
	case WXK_F8:
		return 119;
	case WXK_F9:
		return 120;
	case WXK_F10:
		return 121;
	case WXK_F11:
		return 122;
	case WXK_F12:
		return 123;
	case WXK_F13:
		return 124;
	case WXK_F14:
		return 125;
	case WXK_F15:
		return 126;
	case WXK_F16:
		return 127;
	case WXK_F17:
		return 128;
	case WXK_F18:
		return 129;
	case WXK_F19:
		return 130;
	case WXK_F20:
		return 131;
	case WXK_F21:
		return 132;
	case WXK_F22:
		return 133;
	case WXK_F23:
		return 134;
	case WXK_F24:
		return 135;
	case WXK_NUMLOCK:
		return 144;
	case WXK_SCROLL:
		return 145;
	case ':':
		return 186;
	case WXK_NUMPAD_EQUAL:
		return 187;
	case ',':
		return 188;
	case '_':
		return 189;
	case '.':
		return 190;
	case '?':
		return 191;
	case '"':
		return 192;
	case '{':
		return 219;
	case '|':
		return 220;
	case  '}':
		return 221;
	case '#':
		return 222;
	case '`':
		return 223;

	default:
		return 0;
	}
}
