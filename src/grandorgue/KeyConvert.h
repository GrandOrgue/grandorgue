/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef KEYCONVERT_H
#define KEYCONVERT_H

int WXKtoVK(int what)
{
    if ((what >= 'A' && what <= 'Z') || (what >= '0' && what <= '9') || what == WXK_SPACE || what == WXK_BACK || what == WXK_TAB || what == WXK_RETURN || what == WXK_ESCAPE)
        return what;
    if (what >= WXK_SHIFT && what <= WXK_CAPITAL)
        return what - WXK_SHIFT + 16;
    if (what >= WXK_PAGEUP && what <= WXK_INSERT)
        return what - WXK_PAGEUP + 33;
    if (what >= WXK_NUMPAD0 && what <= WXK_F24)
        return what - WXK_NUMPAD0 + 96;
    if (what >= WXK_WINDOWS_LEFT && what <= WXK_WINDOWS_MENU)
        return what - WXK_WINDOWS_LEFT + 91;
    if (what >= WXK_NUMPAD_F1 && what <= WXK_NUMPAD_F4)
        return what - WXK_NUMPAD_F1 + 112;
    if (what >= WXK_NUMPAD_LEFT && what <= WXK_NUMPAD_DOWN)
        return what - WXK_NUMPAD_LEFT + 37;
    if (what >= WXK_NUMPAD_MULTIPLY && what <= WXK_NUMPAD_DIVIDE)
        return what - WXK_NUMPAD_MULTIPLY + 106;

    if (what==WXK_NUMPAD_TAB) return 9;
    if (what==WXK_NUMPAD_ENTER) return 13;
    if (what==WXK_NUMPAD_SPACE) return 32;
    if (what==WXK_NUMPAD_PAGEUP) return 33;
    if (what==WXK_NUMPAD_PAGEDOWN) return 34;
    if (what==WXK_NUMPAD_END) return 35;
    if (what==WXK_NUMPAD_HOME) return 36;
    if (what==WXK_NUMPAD_INSERT) return 45;
    if (what==WXK_NUMPAD_DELETE) return 46;
    if (what==WXK_HELP) return 47;
    if (what==WXK_NUMLOCK) return 144;
    if (what==WXK_SCROLL) return 145;
    if (what==':') return 186;
    if (what==WXK_NUMPAD_EQUAL) return 187;
    if (what==',') return 188;
    if (what=='_') return 189;
    if (what=='.') return 190;
    if (what=='?') return 191;
    if (what=='"') return 192;
    if (what=='{') return 219;
    if (what=='|') return 220;
    if (what== '}') return 221;
    if (what=='#') return 222;
    if (what=='`') return 223;
    if (what==WXK_LEFT) return 256;
    if (what==WXK_RIGHT) return 257;
    if (what==WXK_DOWN) return 258;
    return 0;
}

#endif
