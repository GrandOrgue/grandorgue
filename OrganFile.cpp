/*
 * GrandOrgue - Copyright (C) 2009 JLDER - free pipe organ simulator based on MyOrgan Copyright (C) 2006 Kloria Publishing LLC
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

#include "MyOrgan.h"

#if 0
void * _cache_p_[1048576];
size_t _cache_i_ = 0;
size_t _cache_size_ = 0;
FILE *_debug_ = 0;
void _do_alloc_(void* what, size_t size, void* retaddr, int line)
{
    if (!_debug_)
        _debug_ = fopen("c:\\log.txt", "w");
    fprintf(_debug_, "%08X+ %08X %08X %04d\n", (unsigned)what, size, (unsigned)retaddr, line);
    _cache_p_[_cache_i_++] = what;
    _cache_p_[_cache_i_++] = (void*)size;
    _cache_size_ += size;
}
void _do_free_(void* what, void* retaddr, int line)
{
    size_t i;
    for (i = 0; i < _cache_i_; i += 2)
        if (_cache_p_[i] == what)
            break;
    if (i >= _cache_i_)
        ::wxLogMessage("*** Attempt to free %p by %p (Line #%d)", what, retaddr, line);
    else
    {
        if (!_debug_)
            _debug_ = fopen("c:\\log.txt", "w");
        fprintf(_debug_, "%08X- %08X %08X %04d\n", (unsigned)what, (unsigned)_cache_p_[i + 1], (unsigned)retaddr, line);
        _cache_size_ -= (unsigned)_cache_p_[i + 1];
        _cache_i_ -= 2;
        for (; i < _cache_i_; i += 2)
        {
            _cache_p_[i + 0] = _cache_p_[i + 2];
            _cache_p_[i + 1] = _cache_p_[i + 3];
        }
    }
}
void* _malloc_(size_t size, int line = 0)
{
    void* ptr = malloc(size);
    _do_alloc_(ptr, size, __builtin_return_address(0), line);
    return ptr;
}
void* _realloc_(void* what, size_t size, int line = 0)
{
    void* ptr = realloc(what, size);
    if (ptr)
    {
        _do_free_(what, __builtin_return_address(0), line);
        _do_alloc_(ptr, size, __builtin_return_address(0), line);
    }
    return ptr;
}
void _free_(void* what, int line = 0)
{
    free(what);
    _do_free_(what, __builtin_return_address(0), line);
}
inline void* operator new(size_t size)
{
    return _malloc_(size);
}
inline void operator delete(void* what)
{
    return _free_(what);
}
inline void* operator new[](size_t size)
{
    return _malloc_(size);
}
inline void operator delete[](void* what)
{
    return _free_(what);
}

#define malloc(a) _malloc_(a, __LINE__)
#define realloc(a, b) _realloc_(a, b, __LINE__)
#define free(a) _free_(a, __LINE__)

#endif

extern void _heapwalk_();

extern MySound* g_sound;
extern char* s_MIDIMessages[];
MyOrganFile* organfile = 0;

WX_DECLARE_STRING_HASH_MAP(MyPipe*, MyPipeHash);
MyPipeHash pipehash;

extern const unsigned char* Images_Wood[];
extern int c_Images_Wood[];

void* MyObject::operator new (size_t s)
{
	void* chunk = ::operator new(s);
	memset(chunk, 0, s);
	return chunk;
}

void* MyObject::operator new[] (size_t s)
{
	void* chunk = ::operator new(s);
	memset(chunk, 0, s);
	return chunk;
}

inline int ra_getindex(MyPipe* pipe, int *f, int *v)
{
	register int retval = 0;
	if (v[0] + v[1] + v[2] + v[3] >= 0)
		retval = PHASE_ALIGN_RES;
	retval += (((((f[0] + f[1] + f[2] + f[3]) >> 2) + pipe->ra_volume) * pipe->ra_factor) >> 26);
	wxASSERT(retval >= 0 && retval < PHASE_ALIGN_RES_VA);
//	if (retval>PHASE_ALIGN_RES_VA) retval=PHASE_ALIGN_RES_VA;
//	if (retval<0) retval=0;
	return retval;
}

inline int ra_getindex(MyPipe* pipe, short *f, short *v)
{
	register int retval = 0;
	if ((int)v[0] + (int)v[1] + (int)v[2] + (int)v[3] >= 0)
		retval = PHASE_ALIGN_RES;
	retval += ((((((int)f[0] + (int)f[1] + (int)f[2] + (int)f[3]) >> 2) + pipe->ra_volume) * pipe->ra_factor) >> 26);
	wxASSERT(retval >= 0 && retval < PHASE_ALIGN_RES_VA);
	return retval;
}

void MyPipe::Set(bool on)
{
	if (on)
	{
		if (instances < 1)
		{
			if (g_sound->samplers_count >= g_sound->polyphony)
				return;
			MySampler* sampler = g_sound->samplers_open[g_sound->samplers_count++];
			memset(sampler, 0, sizeof(MySampler));
			if (instances < 0)
				sampler->stage = 2;
			else if (g_sound->b_random && !g_sound->windchests[WindchestGroup])
				sampler->offset = rand() & 0x78;	// random 0-15, *2 stereo *4 sizeof(int)
			sampler->type = types[0];
			sampler->fade = sampler->fademax = ra_amp;
			sampler->current = offset[0];
			sampler->ptr = ptr[0];
			sampler->pipe = this;
			sampler->shift = ra_shift;
			sampler->f = *(wxInt64*)(f[0]);
			sampler->v = *(wxInt64*)(v[0]);
			sampler->time = organfile->m_elapsed;
			this->sampler = sampler;

			sampler->next = g_sound->windchests[WindchestGroup];
			g_sound->windchests[WindchestGroup] = sampler;
		}
		if (instances > -1)
			instances++;
	}
	else
	{
		if (instances > 0)
		{
			if (instances == 1)
			{
				if (g_sound->samplers_count < g_sound->polyphony && organfile->windchest[WindchestGroup].m_Volume)
				{
					MySampler* sampler = g_sound->samplers_open[g_sound->samplers_count++];
					memset(sampler, 0, sizeof(MySampler));
					sampler->current = offset[2];
					sampler->ptr = ptr[2];
					sampler->pipe = this;
                    sampler->shift = ra_shift;
					sampler->stage = 2;
					sampler->type = types[2];
					int time = organfile->m_elapsed - this->sampler->time;
                    sampler->time = organfile->m_elapsed;
					sampler->fademax = ra_amp;
					if (g_sound->b_scale && WindchestGroup >= organfile->NumberOfTremulants)
					{
						if (time < 256)
							sampler->fademax = (sampler->fademax * (16384 + (time << 5))) >> 15;
						if (time < 1024)
							sampler->fadeout = 0x0001;
					}
					if (g_sound->b_detach && WindchestGroup >= organfile->NumberOfTremulants)
						sampler->fademax = lrint(((double)sampler->fademax) * organfile->windchest[WindchestGroup].m_Volume);
					sampler->fadein = (sampler->fademax + 128) >> 8;
					if (!sampler->fadein)
						sampler->fadein--;
					sampler->faderemain = 512;	// 32768*65536 / 64*65536

					if (g_sound->b_align)
					{
						register int index = ra_getindex(this, (short*)&this->sampler->f, (short*)&this->sampler->v);
						sampler->current = ra_offset[index];
						sampler->f = *(wxInt64*)(ra_f[index]);
						sampler->v = *(wxInt64*)(ra_v[index]);
					}
					else
					{
						sampler->current = offset[2];
						sampler->f = *(wxInt64*)(f[2]);
						sampler->v = *(wxInt64*)(v[2]);
					}

					if (g_sound->b_detach && WindchestGroup >= organfile->NumberOfTremulants)
					{
						sampler->next = g_sound->windchests[organfile->NumberOfTremulants];
						g_sound->windchests[organfile->NumberOfTremulants] = sampler;
					}
					else
					{
						sampler->next = g_sound->windchests[WindchestGroup];
						g_sound->windchests[WindchestGroup] = sampler;
					}
				}
                sampler->fadeout = (-ra_amp - 128) >> 8;
                if (!sampler->fadeout)
                    sampler->fadeout++;
				this->sampler = 0;
			}
			instances--;
		}
	}
}

inline void mmemcpy(char*& dest, const void* src, size_t size)
{
	memcpy(dest, src, size);
    dest += size;
}

inline short SynthTrem(double amp, double angle)
{
	return (short)(amp * sin(angle));
}

inline short SynthTrem(double amp, double angle, double fade)
{
	return (short)(fade * amp * sin(angle));
}

void MyOrganFile::CompressWAV(char*& compress, short* fv, short* ptr, int count, int channels, int stage)
{
	int f[4] = {0, 0, 0, 0}, v[4] = {0, 0, 0, 0}, a[4] = {0, 0, 0, 0};
	int size = 0, index;
	char* origlength;
	int i, j, count2;

	// not recommended, but if they want to, reduce stereo to mono
	if (channels == 2 && !(g_sound->b_stereo))
	{
		for (i = 0, j = 0; i < count; i += 2)
			ptr[j++] = ((int)ptr[i] + (int)ptr[i + 1]) >> 1;
		count >>= 1;
		channels = 1;
	}

	int maxsearch = 2206 * channels;	// ~20Hz maximum search width for phase alignment table
	if (maxsearch > count)
		maxsearch = count - (count & channels);

	origlength = compress;
	m_compress_p->types[stage] = (channels - 1) << 1;

	for (i = 0; i < 4; i++)
	{
		j = i >> (2 - channels);
		if (count > channels * 2)
            v[i] = fv[i + 12] = ptr[j + channels * 2] - ptr[j];
		else
			v[i] = fv[i + 12] = 0;
		f[i] = fv[i] = ptr[j] - v[i];
	}

	bool flags[PHASE_ALIGN_RES_VA];
	if (stage == 2)
	{
		for (j = 0; j < PHASE_ALIGN_RES_VA; j++)
		{
			flags[j] = false;
			m_compress_p->ra_offset[j] = 0;
			for (i = 0; i < 4; i++)
			{
				m_compress_p->ra_f[j][i] = f[i];
				m_compress_p->ra_v[j][i] = v[i];
			}
		}

		int prev_index = ra_getindex(m_compress_p, f, v), prev_i = 0;
		for (i = 0, j = 0; i < maxsearch; )
		{
			v[j] = ptr[i] - f[j];
			f[j] = ptr[i];
			j++;
			if (channels == 1)
			{
				v[j] = v[j-1];
				f[j] = f[j-1];
				j++;
			}
			j &= 3;
			i++;
			if (!j)
			{
				index = ra_getindex(m_compress_p, f, v);
				if (index != prev_index)
				{
					if (!m_compress_p->ra_offset[prev_index])
						m_compress_p->ra_offset[prev_index] = ((prev_i + i) >> (channels + 1)) << (channels + 1);
					prev_i = i;
					prev_index = index;
				}
			}
		}

		for (i = 0; i < 4; i++)
		{
			f[i] = fv[i];
			v[i] = fv[i + 12];
		}
	}

	if (!b_squash)
		goto CantCompress;

	count2 = count + (count & channels);
	if (count & channels)
		for (j = 0; j < channels; j++)
			*(ptr + count + j) = 2 * (int)*(ptr + count + j - 2 * channels) - (int)*(ptr + count + j - 4 * channels);

	if (channels == 2)
	{
		for (i = 0; i < count2; )
		{
			register int value;
			value = ptr[i] - f[0];
			a[0] = value - v[0];
			v[0] = value;
			f[0] = ptr[i++];
			value = ptr[i] - f[1];
			a[1] = value - v[1];
			v[1] = value;
			f[1] = ptr[i++];
			value = ptr[i] - f[2];
			a[2] = value - v[2];
			v[2] = value;
			f[2] = ptr[i++];
			value = ptr[i] - f[3];
			a[3] = value - v[3];
			v[3] = value;
			f[3] = ptr[i++];

            if (((((v[0] + 32768) | (v[1] + 32768) | (v[2] + 32768) | (v[3] + 32768)) >> 16) | (((a[0] + 16384) | (a[1] + 16384) | (a[2] + 16384) | (a[3] + 16384)) >> 15)) && i < count)
                goto CantCompress;
            if ( (((a[0] + 128) | (a[1] + 128) | (a[2] + 128)) >> 8) | ((a[3] + 64) >> 7))
            {
                *(int*)(compress     ) = ((a[3] & 0xFF00) << 17) | ((a[2] & 0xFF00) <<  9) | ((a[1] & 0xFF00) <<  1) | ((a[0] & 0xFF00) >>  7);
                *(int*)(compress += 4) = ((a[3] & 0x00FF) << 24) | ((a[2] & 0x00FF) << 16) | ((a[1] & 0x00FF) <<  8) | ((a[0] & 0x00FF)      );
                size += i <= count ? 8 : 4;
            }
            else
            {
                *(int*)(compress     ) = ((a[3] & 0x00FF) << 25) | ((a[2] & 0x00FF) << 17) | ((a[1] & 0x00FF) <<  9) | ((a[0] & 0x00FF) <<  1) | 1;
                size += i <= count ? 4 : 2;
            }
            compress += 4;

            if (i < maxsearch && stage == 2)
            {
                index = ra_getindex(m_compress_p, f, v);
                if (!flags[index] && m_compress_p->ra_offset[index] == i << 1)
                {
                    flags[index] = true;
                    m_compress_p->ra_offset[index] = size;
                    for (j = 0; j < 4; j++)
                    {
                        m_compress_p->ra_f[index][j] = f[j];
                        m_compress_p->ra_v[index][j] = v[j];
                    }
                }
            }
		}
	}
	else
	{
		for (i = 0; i < count2; )
		{
			register int value;
			value = ptr[i] - f[0];
			a[0] = value - v[0];
			v[0] = value;
			f[0] = ptr[i++];
			value = ptr[i] - f[2];
			a[2] = value - v[2];
			v[2] = value;
			f[2] = ptr[i++];

            if (((((v[0] + 32768) | (v[2] + 32768)) >> 16) | (((a[0] + 16384) | (a[2] + 16384)) >> 15)) && i < count)
                goto CantCompress;

            if (((a[0] + 128) >> 8) | ((a[2] + 64) >> 7))
            {
                *(short*)(compress     ) = ((a[2] & 0xFF00) <<  1) | ((a[0] & 0xFF00) >>  7);
                *(short*)(compress += 2) = ((a[2] & 0x00FF) <<  8) | ((a[0] & 0x00FF)      );
                size += i <= count ? 4 : 2;
            }
            else
            {
                *(short*)(compress     ) = ((a[2] & 0x00FF) <<  9) | ((a[0] & 0x00FF) <<  1) | 1;
                size += i <= count ? 2 : 1;
            }
            compress += 2;

            if (i < maxsearch && stage == 2)
            {
                f[1] = f[0];
                f[3] = f[2];
                v[1] = v[0];
                v[3] = v[2];
                index = ra_getindex(m_compress_p, f, v);
                if (!flags[index] && m_compress_p->ra_offset[index] == i << 1)
                {
                    flags[index] = true;
                    m_compress_p->ra_offset[index] = size;
                    for (j = 0; j < 4; j++)
                    {
                        m_compress_p->ra_f[index][j] = f[j];
                        m_compress_p->ra_v[index][j] = v[j];
                    }
                }
            }
		}
	}
	compress += channels << 1;

	goto Done;

CantCompress:
	m_compress_p->types[stage] |= 1;

	compress = origlength;
	mmemcpy(compress, ptr, size = sizeof(short) * count);
	mmemcpy(compress, ptr, sizeof(short) * (count & channels));

Done:
	m_compress_p->ptr[stage]    = (wxByte*)((wxByte*)(origlength + size) - (wxByte*)m_compress_p);
	m_compress_p->offset[stage] = -size;
	if (stage == 2)
		for (j = 0; j < PHASE_ALIGN_RES_VA; j++)
			m_compress_p->ra_offset[j] -= size;
}

bool INIReadKey(wxFileConfig* cfg, const char* group, const char* key, void* retval, ValueType type, bool required = true, int nmin = 0, int nmax = 0)
{
	wxString string;
	wxInt16 integer;
	static wxInt16 sizes[2][4] = {{783, 1007, 1263, 1583}, {495, 663, 855, 1095}};

	try
	{
		cfg->SetPath("/");
		if (!cfg->HasGroup(group))
		{//JB: strncasecmp was strnicmp
			if (strlen(group) >= 8 && !strncasecmp(group, "General", 7) && group[7] > '0')	// FrameGeneral groups aren't required.
			{
			    if (type == ORGAN_INTEGER)
                    *(wxInt16*)retval = 0;
				return false;
			}
			throw -1;
		}
		cfg->SetPath(wxString("/") + group);

		if (type >= ORGAN_INTEGER)
		{
			if (!cfg->Read(key, &string) || string.empty())
			{
				integer = nmin;
				if (required)
					throw -2;
			}
			else
			{
				if (!::wxIsdigit(string[0]) && string[0] != '+' && string[0] != '-' && string.CmpNoCase("none"))
					throw -3;
				integer = atoi(string.c_str() + (string[0] == '+' ? 1 : 0));
			}

			if (integer >= nmin && integer <= nmax)
			{
				if (type == ORGAN_LONG)
					*(wxInt32*)retval = integer;
				else
					*(wxInt16*)retval = integer;
			}
			else
				throw -4;
		}
		else
		{
			if (!cfg->Read(key, &string))
			{
				if (required)
					throw -2;
			}
			else
			{
				string.Trim();
				if (type < ORGAN_STRING)
					string.MakeUpper();

				integer = 0;
				switch(type)
				{
				case ORGAN_BOOLEAN:
					// we cannot assign bitfield bool's the normal way
					// pointers to bitfields are not allowed!
					if (string[0] == 'Y')
						return true;
					else if (string[0] == 'N')
						return false;
					else
						throw -3;
					break;
				case ORGAN_FONTSIZE:
					if (string == "SMALL")
						integer = 6;
					else if (string == "NORMAL")
						integer = 7;
					else if (string == "LARGE")
						integer = 10;
					else
						throw -3;
					*(wxInt16*)retval = integer;
					break;
				case ORGAN_SIZE:
					if (string == "SMALL")
						integer = 0;
					else if (string == "MEDIUM")
						integer = 1;
					else if (string == "MEDIUM LARGE")
						integer = 2;
					else if (string == "LARGE")
						integer = 3;
					else
						throw -3;
					*(wxInt16*)retval = sizes[nmin][integer];
					break;
				case ORGAN_COLOR:
					if (string == "BLACK")
						*(wxColour*)retval = wxColour(0x00, 0x00, 0x00);
					else if (string == "BLUE")
						*(wxColour*)retval = wxColour(0x00, 0x00, 0xFF);
					else if (string == "DARK BLUE")
						*(wxColour*)retval = wxColour(0x00, 0x00, 0x80);
					else if (string == "GREEN")
						*(wxColour*)retval = wxColour(0x00, 0xFF, 0x00);
					else if (string == "DARK GREEN")
						*(wxColour*)retval = wxColour(0x00, 0x80, 0x00);
					else if (string == "CYAN")
						*(wxColour*)retval = wxColour(0x00, 0xFF, 0xFF);
					else if (string == "DARK CYAN")
						*(wxColour*)retval = wxColour(0x00, 0x80, 0x80);
					else if (string == "RED")
						*(wxColour*)retval = wxColour(0xFF, 0x00, 0x00);
					else if (string == "DARK RED")
						*(wxColour*)retval = wxColour(0x80, 0x00, 0x00);
					else if (string == "MAGENTA")
						*(wxColour*)retval = wxColour(0xFF, 0x00, 0xFF);
					else if (string == "DARK MAGENTA")
						*(wxColour*)retval = wxColour(0x80, 0x00, 0x80);
					else if (string == "YELLOW")
						*(wxColour*)retval = wxColour(0xFF, 0xFF, 0x00);
					else if (string == "DARK YELLOW")
						*(wxColour*)retval = wxColour(0x80, 0x80, 0x00);
					else if (string == "LIGHT GREY")
						*(wxColour*)retval = wxColour(0xC0, 0xC0, 0xC0);
					else if (string == "DARK GREY")
						*(wxColour*)retval = wxColour(0x80, 0x80, 0x80);
					else if (string == "WHITE")
						*(wxColour*)retval = wxColour(0xFF, 0xFF, 0xFF);
					else
						throw -3;
					break;
				case ORGAN_STRING:
					if ((int)string.length() > nmax)
						throw -4;
					*(wxString*)retval = string;
                default:
					break;
				}
			}
		}
	}
	catch(int exception)
	{
		wxString error;
		switch(exception)
		{
		case -1:
			error.Printf("Missing required group '/%s'", group);
			break;
		case -2:
			error.Printf("Missing required value '/%s/%s'", group, key);
			break;
		case -3:
			error.Printf("Invalid value '/%s/%s'", group, key);
			break;
		case -4:
			error.Printf("Out of range value '/%s/%s'", group, key);
			break;
		}
		throw error;
	}

	return false;
}

bool INIReadBoolean(wxFileConfig* cfg, const char* group, const char* key, bool required = true)
{
	return INIReadKey(cfg, group, key, 0, ORGAN_BOOLEAN, required);
}

wxColour INIReadColor(wxFileConfig* cfg, const char* group, const char* key, bool required = true)
{
	wxColour retval;
	INIReadKey(cfg, group, key, &retval, ORGAN_COLOR, required);
	return retval;
}

wxString INIReadString(wxFileConfig* cfg, const char* group, const char* key, int nmax = 4096, bool required = true)
{
	wxString retval;
	INIReadKey(cfg, group, key, &retval, ORGAN_STRING, required, 0, nmax);
	return retval;
}

wxInt16 INIReadInteger(wxFileConfig* cfg, const char* group, const char* key, int nmin = 0, int nmax = 0, bool required = true)
{
	wxInt16 retval;
	INIReadKey(cfg, group, key, &retval, ORGAN_INTEGER, required, nmin, nmax);
	return retval;
}

wxInt32 INIReadLong(wxFileConfig* cfg, const char* group, const char* key, int nmin = 0, int nmax = 0, bool required = true)
{
	wxInt32 retval;
	INIReadKey(cfg, group, key, &retval, ORGAN_LONG, required, nmin, nmax);
	return retval;
}

wxInt16 INIReadSize(wxFileConfig* cfg, const char* group, const char* key, int nmin = 0, bool required = true)
{
	wxInt16 retval;
	INIReadKey(cfg, group, key, &retval, ORGAN_SIZE, required, nmin);
	return retval;
}

wxInt16 INIReadFontSize(wxFileConfig* cfg, const char* group, const char* key, bool required = true)
{
	wxInt16 retval;
	INIReadKey(cfg, group, key, &retval, ORGAN_FONTSIZE, required);
	return retval;
}

#define INIREAD(x,y) (x) = INIRead##y(cfg, group, #x
#define GET_BIT(x,y,z) (x[y >> 3][z] & (0x80 >> (y & 7)) ? true : false)
#define SET_BIT(x,y,z,b) (b ? x[y >> 3][z] |= (0x80 >> (y & 7)) : x[y >> 3][z] &= (0xFFFFFF7F >> (y & 7)))

void MyManual::Load(wxFileConfig* cfg, const char* group)
{
	INIREAD(Name                                         ,String  ),   32);
	INIREAD(NumberOfLogicalKeys                          ,Integer ),    1,  192);
	INIREAD(FirstAccessibleKeyLogicalKeyNumber           ,Integer ),    1, NumberOfLogicalKeys);
	INIREAD(FirstAccessibleKeyMIDINoteNumber             ,Integer ),    0,  127);	// spec says 128
	INIREAD(NumberOfAccessibleKeys                       ,Integer ),    0,   85);
	INIREAD(MIDIInputNumber                              ,Integer ),    1,    6);
	INIREAD(Displayed                                    ,Boolean ));
	INIREAD(DispKeyColourInverted                        ,Boolean ));
	INIREAD(DispKeyColourWooden                          ,Boolean ),false);
	INIREAD(NumberOfStops                                ,Integer ),    0,   64);
	INIREAD(NumberOfCouplers                             ,Integer ),    0,   16, false);
	INIREAD(NumberOfDivisionals                          ,Integer ),    0,   32, false);
	INIREAD(NumberOfTremulants                           ,Integer ),    0,   10, false);

	int i, j;
	char buffer[64];

	stop = new MyStop[NumberOfStops];
	for (i = 0; i < NumberOfStops; i++)
	{
		sprintf(buffer, "Stop%03d", i + 1);
		sprintf(buffer, "Stop%03d", INIReadInteger(cfg, group, buffer, 1, 448));
		stop[i].m_ManualNumber = m_ManualNumber;
		stop[i].Load(cfg, buffer);
	}

	coupler = new MyCoupler[NumberOfCouplers];
	for (i = 0; i < NumberOfCouplers; i++)
	{
		sprintf(buffer, "Coupler%03d", i + 1);
		sprintf(buffer, "Coupler%03d", INIReadInteger(cfg, group, buffer, 1, 64));
		coupler[i].Load(cfg, buffer);
	}

	divisional = new MyDivisional[NumberOfDivisionals];
	for (i = 0; i < NumberOfDivisionals; i++)
	{
		sprintf(buffer, "Divisional%03d", i + 1);
		sprintf(buffer, "Divisional%03d", INIReadInteger(cfg, group, buffer, 1, 224));
		divisional[i].m_ManualNumber = m_ManualNumber;
		divisional[i].m_DivisionalNumber = i;
		divisional[i].Load(cfg, buffer);
	}

	for (i = 0; i < NumberOfTremulants; i++)
	{
		sprintf(buffer, "Tremulant%03d", i + 1);
		tremulant[i] = INIReadInteger(cfg, group, buffer, 1, organfile->NumberOfTremulants);
	}

	for (i = 0; i < NumberOfStops; i++)
	{
		if (!stop[i].m_auto)
			continue;
		for (j = 0; j < NumberOfStops; j++)
		{
			if (i == j)
				continue;
			if (stop[j].FirstAccessiblePipeLogicalKeyNumber < stop[i].FirstAccessiblePipeLogicalKeyNumber + stop[i].NumberOfAccessiblePipes &&
				stop[j].FirstAccessiblePipeLogicalKeyNumber + stop[j].NumberOfAccessiblePipes > stop[i].FirstAccessiblePipeLogicalKeyNumber)
			{
				stop[i].m_auto = stop[j].m_auto = false;
                break;
			}
		}
	}
}

void MyManual::Set(int note, bool on, bool pretend, int depth, MyCoupler* prev)
{
	int i, j;

	// test polyphony?
#if 0
	for (i = 0; i < organfile->NumberOfPipes; i++)
		organfile->pipe[i]->Set(on);
	return;
#endif

	if (depth > 32)
	{
		::wxLogFatalError("Infinite recursive coupling detected!");
		return;
	}

	note -= FirstAccessibleKeyMIDINoteNumber;
	bool outofrange = note < 0 || note >= NumberOfAccessibleKeys;

	if (!depth && outofrange)
		return;

	if (!outofrange && !pretend)
	{
		if (depth)
		{
		    if (!(m_MIDI[note] >> 1) && !on)
                return;
			m_MIDI[note] += on ? 2 : -2;
		}
		else
		{
		    if ((m_MIDI[note] & 1) ^ !on)
                return;
			m_MIDI[note]  = (m_MIDI[note] & 0xFFFFFFFE) | (on ? 1 : 0);
		}
	}

	bool unisonoff = false;
	for (i = 0; i < NumberOfCouplers; i++)
	{
		if (!coupler[i].DefaultToEngaged)
			continue;
		if (coupler[i].UnisonOff && (!depth || (prev && prev->CoupleToSubsequentUnisonIntermanualCouplers)))
		{
			unisonoff = true;
			continue;
		}
		j = coupler[i].DestinationManual;
		if (!depth || (prev && (
			(j == m_ManualNumber && coupler[i].DestinationKeyshift < 0 && prev->CoupleToSubsequentDownwardIntramanualCouplers) ||
			(j == m_ManualNumber && coupler[i].DestinationKeyshift > 0 && prev->CoupleToSubsequentUpwardIntramanualCouplers) ||
			(j != m_ManualNumber && coupler[i].DestinationKeyshift < 0 && prev->CoupleToSubsequentDownwardIntermanualCouplers) ||
			(j != m_ManualNumber && coupler[i].DestinationKeyshift > 0 && prev->CoupleToSubsequentUpwardIntermanualCouplers)
			)))
		{
			organfile->manual[j].Set(note + FirstAccessibleKeyMIDINoteNumber + coupler[i].DestinationKeyshift, on, false, depth + 1, coupler + i);
		}
	}

	if (!unisonoff)
	{
		for (i = 0; i < NumberOfStops; i++)
		{
			if (!stop[i].DefaultToEngaged)
				continue;
			j = note - (stop[i].FirstAccessiblePipeLogicalKeyNumber - 1);
			if (j < 0 || j >= stop[i].NumberOfAccessiblePipes)
				continue;
			j += stop[i].FirstAccessiblePipeLogicalPipeNumber - 1;

			organfile->pipe[stop[i].pipe[j]]->Set(on);
		}
	}

	if (!outofrange)
	{
		wxCommandEvent event(wxEVT_NOTEONOFF, 0);
		event.SetInt(m_ManualNumber);
		event.SetExtraLong(note);
		::wxGetApp().frame->AddPendingEvent(event);
	}
}

MyManual::~MyManual(void)
{
	if (stop)
		delete[] stop;
	if (coupler)
		delete[] coupler;
	if (divisional)
		delete[] divisional;
}

void MyWindchest::Load(wxFileConfig* cfg, const char* group)
{
	INIREAD(NumberOfEnclosures                           ,Integer ),    0,    6);
	INIREAD(NumberOfTremulants                           ,Integer ),    0,    6);

	int i;
	char buffer[64];

	for (i = 0; i < NumberOfEnclosures; i++)
	{
		sprintf(buffer, "Enclosure%03d", i + 1);
		enclosure[i] = INIReadInteger(cfg, group, buffer, 1, organfile->NumberOfEnclosures) - 1;
	}
	for (i = 0; i < NumberOfTremulants; i++)
	{
		sprintf(buffer, "Tremulant%03d", i + 1);
		tremulant[i] = INIReadInteger(cfg, group, buffer, 1, organfile->NumberOfTremulants) - 1;
	}
}

bool MyEnclosure::Draw(int xx, int yy, wxDC* dc, wxDC* dc2)
{
	if (!dc)
	{
        wxRect rect(m_X, organfile->m_EnclosureY, 46, 61);
        return rect.Contains(xx, yy);
	}

	dc->SetBrush(*wxBLACK_BRUSH);
	dc->DrawRectangle(m_X, organfile->m_EnclosureY + 13, 46, 44);
	int dx = 1 + ( 3 * MIDIValue) / 127;
	int dy = 1 + (13 * MIDIValue) / 127;
	wxPoint points[4];
	points[0].x = m_X +  7 + dx;
	points[1].x = m_X + 38 - dx;
	points[2].x = m_X + 38 + dx;
	points[3].x = m_X +  7 - dx;
	points[0].y = points[1].y = organfile->m_EnclosureY + 13 + dy;
	points[2].y = points[3].y = organfile->m_EnclosureY + 56 - dy;
	dc->SetBrush(::wxGetApp().frame->m_pedalBrush);
	dc->DrawPolygon(4, points);

	if (dc2)
		dc2->Blit(m_X, organfile->m_EnclosureY + 13, 46, 44, dc, m_X, organfile->m_EnclosureY + 13);
	return false;
}

void MyEnclosure::Load(wxFileConfig* cfg, const char* group)
{
	INIREAD(Name                                         ,String  ),   64);
	INIREAD(AmpMinimumLevel                              ,Integer ),    0,  100);
	INIREAD(MIDIInputNumber                              ,Integer ),    1,    6);
	Set(127);	// default to full volume until we receive any messages
}

void MyEnclosure::Set(int n)
{
    if (n < 0)
        n = 0;
    if (n > 127)
        n = 127;
	MIDIValue = n;
    wxCommandEvent event(wxEVT_ENCLOSURE, 0);
	event.SetClientData(this);
	::wxGetApp().frame->AddPendingEvent(event);
}

void MyLabel::Load(wxFileConfig* cfg, const char* group)
{
	INIREAD(Name                                         ,String  ),   64);
	INIREAD(FreeXPlacement                               ,Boolean ));
	INIREAD(FreeYPlacement                               ,Boolean ));

	if (!FreeXPlacement)
	{
		INIREAD(DispDrawstopCol                              ,Integer ),    1, organfile->DispDrawstopCols);
		INIREAD(DispSpanDrawstopColToRight                   ,Boolean ));
	}
	else
		INIREAD(DispXpos                                     ,Integer ),    0, organfile->DispScreenSizeHoriz);

	if (!FreeYPlacement)
		INIREAD(DispAtTopOfDrawstopCol                       ,Boolean ));
	else
		INIREAD(DispYpos                                     ,Integer ),    0, organfile->DispScreenSizeVert);

	// NOTICE: this should not be allowed, but some existing definition files use improper values
	if (!DispXpos)
		DispYpos++;
	if (!DispYpos)
		DispYpos++;

	INIREAD(DispLabelColour                              ,Color   ));
	INIREAD(DispLabelFontSize                            ,FontSize));
	INIREAD(DispImageNum                                 ,Integer ),    1,    1);
}

void MyControl::Load(wxFileConfig* cfg, const char* group)
{
	INIREAD(Name                                         ,String  ),   64);
	INIREAD(ShortcutKey                                  ,Integer ),    0,  255, false);
	INIREAD(Displayed                                    ,Boolean ));
	INIREAD(DispKeyLabelOnLeft                           ,Boolean ));
	INIREAD(DispLabelColour                              ,Color   ));
	INIREAD(DispLabelFontSize                            ,FontSize));

	if (strlen(group) >= 3)
		ObjectNumber = atoi(group + strlen(group) - 3);
}

void MyDrawstop::Load(wxFileConfig* cfg, const char* group)
{
	INIREAD(DispDrawstopRow                              ,Integer ),    1, 99 + organfile->DispExtraDrawstopRows);
	INIREAD(DispDrawstopCol                              ,Integer ),    1, DispDrawstopRow > 99 ? organfile->DispExtraDrawstopCols : organfile->DispDrawstopCols);
	INIREAD(DefaultToEngaged                             ,Boolean ));
	INIREAD(DisplayInInvertedState                       ,Boolean ));
	INIREAD(DispImageNum                                 ,Integer ),    1,    2);
	INIREAD(StopControlMIDIKeyNumber                     ,Integer ),    0,  127, false);

	MyControl::Load(cfg, group);
}

bool MyDrawstop::Draw(int xx, int yy, wxDC* dc, wxDC* dc2)
{
	int x, y, i;
	if (!Displayed)
		return false;

	if (DispDrawstopRow > 99)
	{
		x = organfile->m_JambTopX + (DispDrawstopCol - 1) * 78 + 6;
		y = organfile->m_JambTopDrawstop + (DispDrawstopRow - 100) * 69 + 2;
	}
	else
	{
		i = organfile->DispDrawstopCols >> 1;
		if (DispDrawstopCol <= i)
		{
			x = organfile->m_JambLeftX + (DispDrawstopCol - 1) * 78 + 6;
			y = organfile->m_JambLeftRightY + (DispDrawstopRow - 1) * 69 + 32;
		}
		else
		{
			x = organfile->m_JambRightX + (DispDrawstopCol - 1 - i) * 78 + 6;
			y = organfile->m_JambLeftRightY + (DispDrawstopRow - 1) * 69 + 32;
		}
		if (organfile->DispPairDrawstopCols)
			x += (((DispDrawstopCol - 1) % i) >> 1) * 18;
		if (DispDrawstopCol <= i)
			i = DispDrawstopCol;
		else
			i = organfile->DispDrawstopCols - DispDrawstopCol + 1;
		if (organfile->DispDrawstopColsOffset && (i & 1) ^ organfile->DispDrawstopOuterColOffsetUp)
			y += 35;
	}

	if (!dc)
		return !(xx < x || xx > x + 64 || yy < y || yy > y + 64 || (x + 32 - xx) * (x + 32 - xx) + (y + 32 - yy) * (y + 32 - yy) > 1024);

	wxRect rect(x, y + 1, 65, 65 - 1);
	wxBitmap bmp = organfile->m_images[((DispImageNum - 1) << 1) + (DisplayInInvertedState ^ DefaultToEngaged ? 1 : 0)];
	dc->DrawBitmap(bmp, x, y, true);
	dc->SetTextForeground(DispLabelColour);
	wxFont font = *wxNORMAL_FONT;
	font.SetFaceName(organfile->DispControlLabelFont);
	font.SetPointSize(DispLabelFontSize);
	dc->SetFont(font);
	dc->DrawLabel(Name, rect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);

	if (dc2)
		dc2->Blit(x, y, 65, 65, dc, x, y);
	return false;
}

bool MyDrawstop::Set(bool on)
{
	if (DefaultToEngaged == on)
		return on;
	DefaultToEngaged = on;
    wxCommandEvent event(wxEVT_DRAWSTOP, 0);
	event.SetClientData(this);
	::wxGetApp().frame->AddPendingEvent(event);
	for (int i = 0; i < organfile->NumberOfReversiblePistons; i++)
	{
	    if (organfile->piston[i].drawstop == this)
	    {
	        organfile->piston[i].DispImageNum = (organfile->piston[i].DispImageNum & 1) | (on ^ DisplayInInvertedState ? 2 : 0);
            wxCommandEvent event(wxEVT_PUSHBUTTON, 0);
            event.SetClientData(organfile->piston + i);
            ::wxGetApp().frame->AddPendingEvent(event);
	    }
	}
#ifdef __VFD__
	MyLCD_WriteLineTwo(Name, 2000);
	return !on;
#endif
}

void MyManual::MIDI(void)
{
	int index = MIDIInputNumber + 7;
	MIDIListenDialog dlg(::wxGetApp().frame, _(s_MIDIMessages[index]), g_sound->i_midiEvents[index], 1);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxConfigBase::Get()->Write(wxString("MIDI/") + s_MIDIMessages[index], dlg.GetEvent());
		g_sound->ResetSound();
	}
}

void MyEnclosure::MIDI(void)
{
	int index = MIDIInputNumber + 1;
	MIDIListenDialog dlg(::wxGetApp().frame, _(s_MIDIMessages[index]), g_sound->i_midiEvents[index], 0);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxConfigBase::Get()->Write(wxString("MIDI/") + s_MIDIMessages[index], dlg.GetEvent());
		g_sound->ResetSound();
	}
}

void MyDrawstop::MIDI(void)
{
	MIDIListenDialog dlg(::wxGetApp().frame, _("Drawstop Trigger"), g_sound->i_midiEvents[14] | StopControlMIDIKeyNumber, 4);
	if (dlg.ShowModal() == wxID_OK)
	{
		StopControlMIDIKeyNumber = dlg.GetEvent() & 0x7F;
		::wxGetApp().m_docManager->GetCurrentDocument()->Modify(true);
	}
}

void MyPushbutton::MIDI(void)
{
	int event = 0xC000;
	if (m_ManualNumber > -1)
		event = g_sound->i_midiEvents[organfile->manual[m_ManualNumber].MIDIInputNumber + 7] ^ 0x5000;
	MIDIListenDialog dlg(::wxGetApp().frame, _("Pushbutton Trigger"), event | (MIDIProgramChangeNumber - 1), m_ManualNumber > -1 ? 4 : 5);
	if (dlg.ShowModal() == wxID_OK)
	{
		MIDIProgramChangeNumber = (dlg.GetEvent() & 0x7F) + 1;
		::wxGetApp().m_docManager->GetCurrentDocument()->Modify(true);
	}
}

void MyPushbutton::Load(wxFileConfig* cfg, const char* group)
{
	INIREAD(DispButtonRow                                ,Integer ),    0, 99 + organfile->DispExtraButtonRows);
	INIREAD(DispButtonCol                                ,Integer ),    1, organfile->DispButtonCols);
	INIREAD(DispImageNum                                 ,Integer ),    1,    2);
	INIREAD(MIDIProgramChangeNumber                      ,Integer ),    1,  128);
	DispImageNum--;

	MyControl::Load(cfg, group);
}

bool MyPushbutton::Draw(int xx, int yy, wxDC* dc, wxDC* dc2)
{
	int x, y, i;
	if (!Displayed)
		return false;

	x = organfile->m_PistonX + (DispButtonCol - 1) * 44 + 6;
	if (DispButtonRow > 99)
	{
		y = organfile->m_JambTopPiston + (DispButtonRow - 100) * 40 + 5;
	}
	else
	{
		i = DispButtonRow;
		if (i == 99)
			i = 0;

        if (i > organfile->NumberOfManuals)
            y = organfile->m_HackY - (i - organfile->NumberOfManuals) * 72 + 32 + 5;
        else
            y = organfile->manual[i].m_PistonY + 5;

		if (organfile->DispExtraPedalButtonRow && !DispButtonRow)
			y += 40;
		if (organfile->DispExtraPedalButtonRowOffset && DispButtonRow == 99)
			x -= 22;
	}
	if (!DispKeyLabelOnLeft)
		x -= 13;

	if (!dc)
		return !(xx < x || xx > x + 30 || yy < y || yy > y + 30 || (x + 15 - xx) * (x + 15 - xx) + (y + 15 - yy) * (y + 15 - yy) > 225);

	wxMemoryDC mdc;
	wxRect rect(x + 1, y + 1, 31 - 1, 30 - 1);
	wxBitmap bmp = organfile->m_images[DispImageNum + 4];
	dc->DrawBitmap(bmp, x, y, true);
	dc->SetTextForeground(DispLabelColour);
	wxFont font = *wxNORMAL_FONT;
	font.SetFaceName(organfile->DispControlLabelFont);
	font.SetPointSize(DispLabelFontSize);
	dc->SetFont(font);
	dc->DrawLabel(Name, rect, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
	if (dc2)
		dc2->Blit(x, y, 31, 30, dc, x, y);
	return false;
}

void MyStop::Load(wxFileConfig* cfg, const char* group)
{
	INIREAD(AmplitudeLevel                               ,Integer ),    0, 1000);
	INIREAD(NumberOfLogicalPipes                         ,Integer ),    1,  192);
	INIREAD(FirstAccessiblePipeLogicalPipeNumber         ,Integer ),    1, NumberOfLogicalPipes);
	INIREAD(FirstAccessiblePipeLogicalKeyNumber          ,Integer ),    1,  128);
	INIREAD(NumberOfAccessiblePipes                      ,Integer ),    1, NumberOfLogicalPipes);
	INIREAD(WindchestGroup                               ,Integer ),    1, organfile->NumberOfWindchestGroups);
	INIREAD(Percussive                                   ,Boolean ));

	WindchestGroup += organfile->NumberOfTremulants;    // we would + 1 but it already has it: clever!

	int i;
	char buffer[64];

	pipe = new wxInt16[NumberOfLogicalPipes];
	for (i = 0; i < NumberOfLogicalPipes; i++)
	{
		sprintf(buffer, "Pipe%03d", i + 1);
		wxString file = INIReadString(cfg, group, buffer);
		organfile->pipe_ptrs.push_back(pipe + i);
		organfile->pipe_files.push_back(file);
		organfile->pipe_windchests.push_back(WindchestGroup);
		organfile->pipe_percussive.push_back(Percussive);
		if (!file.StartsWith("REF:"))
		{
            organfile->pipe_amplitudes.push_back(organfile->AmplitudeLevel * AmplitudeLevel);
			organfile->NumberOfPipes++;
		}
	}

	m_auto = NumberOfLogicalPipes == 1;

	MyDrawstop::Load(cfg, group);
}

bool MyStop::Set(bool on)
{
	if (DefaultToEngaged == on)
		return on;
	DefaultToEngaged = on;
	MySound::MIDIPretend(true);
	DefaultToEngaged = !on;
    MySound::MIDIPretend(false);

	bool retval = MyDrawstop::Set(on);

	if (m_auto)
	{
        MyManual* manual = organfile->manual + m_ManualNumber;
		manual->Set(manual->FirstAccessibleKeyMIDINoteNumber + FirstAccessiblePipeLogicalKeyNumber - 1, on);
	}

	return retval;
}

MyStop::~MyStop(void)
{
	if (pipe)
		delete[] pipe;
}

void MyCoupler::Load(wxFileConfig* cfg, const char* group)
{
	INIREAD(UnisonOff                                    ,Boolean ));
	INIREAD(DestinationManual                            ,Integer ), organfile->FirstManual, organfile->NumberOfManuals, !UnisonOff);
	INIREAD(DestinationKeyshift                          ,Integer ),  -24,   24, !UnisonOff);
	INIREAD(CoupleToSubsequentUnisonIntermanualCouplers  ,Boolean ), !UnisonOff);
	INIREAD(CoupleToSubsequentUpwardIntermanualCouplers  ,Boolean ), !UnisonOff);
	INIREAD(CoupleToSubsequentDownwardIntermanualCouplers,Boolean ), !UnisonOff);
	INIREAD(CoupleToSubsequentUpwardIntramanualCouplers  ,Boolean ), !UnisonOff);
	INIREAD(CoupleToSubsequentDownwardIntramanualCouplers,Boolean ), !UnisonOff);

	MyDrawstop::Load(cfg, group);
}

bool MyCoupler::Set(bool on)
{
	if (DefaultToEngaged == on)
		return on;
	DefaultToEngaged = on;
    MySound::MIDIPretend(true);
	DefaultToEngaged = !on;
    MySound::MIDIPretend(false);
	return MyDrawstop::Set(on);
}

void MyTremulant::Load(wxFileConfig* cfg, const char* group)
{
	INIREAD(Period                                       ,Long    ),   32,  441000);
	INIREAD(StartRate                                    ,Integer ),    1,  100);
	INIREAD(StopRate                                     ,Integer ),    1,  100);
	INIREAD(AmpModDepth                                  ,Integer ),    1,  100);

	MyDrawstop::Load(cfg, group);
}

bool MyTremulant::Set(bool on)
{
	if (DefaultToEngaged == on)
		return on;

	pipe->Set(on);

	return MyDrawstop::Set(on);
}

void MyDivisionalCoupler::Load(wxFileConfig* cfg, const char* group)
{
	INIREAD(BiDirectionalCoupling                       ,Boolean ));
	INIREAD(NumberOfManuals                             ,Integer ),    1,    6);

	int i;
	char buffer[64];

	for (i = 0; i < NumberOfManuals; i++)
	{
		sprintf(buffer, "Manual%03d", i + 1);
		manual[i] = INIReadInteger(cfg, group, buffer, organfile->FirstManual, organfile->NumberOfManuals);
	}

	MyDrawstop::Load(cfg, group);
}

bool MyDivisionalCoupler::Set(bool on)
{
	if (DefaultToEngaged == on)
		return on;
	return MyDrawstop::Set(on);
}

void MyDivisional::Load(wxFileConfig* cfg, const char* group)
{
	INIREAD(NumberOfStops                               ,Integer ),    0, organfile->manual[m_ManualNumber].NumberOfStops);
	INIREAD(NumberOfCouplers                            ,Integer ),    0, organfile->manual[m_ManualNumber].NumberOfCouplers, organfile->DivisionalsStoreIntermanualCouplers || organfile->DivisionalsStoreIntramanualCouplers);
	INIREAD(NumberOfTremulants                          ,Integer ),    0, organfile->NumberOfTremulants, organfile->DivisionalsStoreTremulants);

	int i, j, k;
	char buffer[64];

	for (i = 0; i < NumberOfStops; i++)
	{
		sprintf(buffer, "Stop%03d", i + 1);
		j = INIReadInteger(cfg, group, buffer, -organfile->manual[m_ManualNumber].NumberOfStops, organfile->manual[m_ManualNumber].NumberOfStops);
		k = abs(j) - 1;
		if (k >= 0)
		{
			SET_BIT(stop, k, 0, true);
			SET_BIT(stop, k, 1, j >= 0);
		}
	}
	if (organfile->DivisionalsStoreIntermanualCouplers || organfile->DivisionalsStoreIntramanualCouplers)
	{
		for (i = 0; i < NumberOfCouplers; i++)
		{
			sprintf(buffer, "Coupler%03d", i + 1);
			j = INIReadInteger(cfg, group, buffer, -organfile->manual[m_ManualNumber].NumberOfCouplers, organfile->manual[m_ManualNumber].NumberOfCouplers);
			k = abs(j) - 1;
			if (k >= 0)
			{
				SET_BIT(coupler, k, 0, true);
				SET_BIT(coupler, k, 1, j >= 0);
			}
		}
	}
	if (organfile->DivisionalsStoreTremulants)
	{
		for (i = 0; i < NumberOfTremulants; i++)
		{
			sprintf(buffer, "Tremulant%03d", i + 1);
			j = INIReadInteger(cfg, group, buffer, -organfile->manual[m_ManualNumber].NumberOfTremulants, organfile->manual[m_ManualNumber].NumberOfTremulants);
			k = abs(j) - 1;
			if (k >= 0)
			{
				SET_BIT(tremulant, k, 0, true);
				SET_BIT(tremulant, k, 1, j >= 0);
			}
		}
	}

	MyPushbutton::Load(cfg, group);
}

void MyDivisional::Push(int depth)
{
	int i, k;

	if (g_sound->b_memset)
	{
	    NumberOfStops = NumberOfCouplers = NumberOfTremulants = 0;
		memset(stop, 0, sizeof(stop) + sizeof(coupler) + sizeof(tremulant));
		for (i = 0; i < organfile->manual[m_ManualNumber].NumberOfStops; i++)
		{
		    if (!organfile->CombinationsStoreNonDisplayedDrawstops && !organfile->manual[m_ManualNumber].stop[i].Displayed)
                continue;
		    NumberOfStops++;
			SET_BIT(stop, i, 0, true);
			SET_BIT(stop, i, 1, organfile->manual[m_ManualNumber].stop[i].DefaultToEngaged);
		}
		for (i = 0; i < organfile->manual[m_ManualNumber].NumberOfCouplers; i++)
		{
		    if (!organfile->CombinationsStoreNonDisplayedDrawstops && !organfile->manual[m_ManualNumber].coupler[i].Displayed)
                continue;
		    NumberOfCouplers++;
			if ((organfile->DivisionalsStoreIntramanualCouplers && m_ManualNumber == organfile->manual[m_ManualNumber].coupler[i].DestinationManual) || (organfile->DivisionalsStoreIntermanualCouplers && m_ManualNumber != organfile->manual[m_ManualNumber].coupler[i].DestinationManual))
			{
				SET_BIT(coupler, i, 0, true);
				SET_BIT(coupler, i, 1, organfile->manual[m_ManualNumber].coupler[i].DefaultToEngaged);
			}
		}
		if (organfile->DivisionalsStoreTremulants)
		{
			for (i = 0; i < organfile->manual[m_ManualNumber].NumberOfTremulants; i++)
			{
                if (!organfile->CombinationsStoreNonDisplayedDrawstops && !organfile->tremulant[organfile->manual[m_ManualNumber].tremulant[i] - 1].Displayed)
                    continue;
                NumberOfTremulants++;
				SET_BIT(tremulant, i, 0, true);
				SET_BIT(tremulant, i, 1, organfile->tremulant[organfile->manual[m_ManualNumber].tremulant[i] - 1].DefaultToEngaged);
			}
		}
		::wxGetApp().m_docManager->GetCurrentDocument()->Modify(true);
		depth = 1;
	}
	else
	{
        for (k = 0; ;k++)
        {
            for (i = 0; i < organfile->manual[m_ManualNumber].NumberOfStops; i++)
                if (GET_BIT(stop, i, 0))
                    SET_BIT(stop, i, 1, k < 2 ? organfile->manual[m_ManualNumber].stop[i].Set(GET_BIT(stop, i, 1)) : GET_BIT(stop, i, 1));
            for (i = 0; i < organfile->manual[m_ManualNumber].NumberOfCouplers; i++)
                if (GET_BIT(coupler, i, 0))
                    SET_BIT(coupler, i, 1, k < 2 ? organfile->manual[m_ManualNumber].coupler[i].Set(GET_BIT(coupler, i, 1)) : GET_BIT(coupler, i, 1));
            for (i = 0; i < organfile->manual[m_ManualNumber].NumberOfTremulants; i++)
                if (GET_BIT(tremulant, i, 0))
                    SET_BIT(tremulant, i, 1, k < 2 ? organfile->tremulant[organfile->manual[m_ManualNumber].tremulant[i] - 1].Set(GET_BIT(tremulant, i, 1)) : GET_BIT(tremulant, i, 1));
            if (k >= 2)
                break;
            MySound::MIDIPretend(!k);
        }
	}

	wxByte used = 0;
    for (i = 0; i < 8; i++)
        used |= stop[i][1];
    for (i = 0; i < 2; i++)
        used |= coupler[i][1] | tremulant[i][1];

	for (k = 0; k < organfile->manual[m_ManualNumber].NumberOfDivisionals; k++)
	{
	    MyDivisional *divisional = organfile->manual[m_ManualNumber].divisional + k;
	    int on = ((divisional == this && used) ? 2 : 0);
	    if ((divisional->DispImageNum & 2) != on)
	    {
	        divisional->DispImageNum = (divisional->DispImageNum & 1) | on;
            wxCommandEvent event(wxEVT_PUSHBUTTON, 0);
            event.SetClientData(divisional);
            ::wxGetApp().frame->AddPendingEvent(event);
	    }
	}

	if (depth)
		return;

	for (k = 0; k < organfile->NumberOfDivisionalCouplers; k++)
	{
		if (!organfile->divisionalcoupler[k].DefaultToEngaged)
			continue;
		for (i = 0; i < organfile->divisionalcoupler[k].NumberOfManuals; i++)
			if (organfile->divisionalcoupler[k].manual[i] == m_ManualNumber)
				break;
		if (i < organfile->divisionalcoupler[k].NumberOfManuals)
		{
			for (++i; i < organfile->divisionalcoupler[k].NumberOfManuals; i++)
				organfile->manual[organfile->divisionalcoupler[k].manual[i]].divisional[m_DivisionalNumber].Push(depth + 1);
			if (organfile->divisionalcoupler[k].BiDirectionalCoupling)
			{
				for (i = 0; i < organfile->divisionalcoupler[k].NumberOfManuals; i++)
				{
					if (organfile->divisionalcoupler[k].manual[i] == m_ManualNumber)
						break;
					organfile->manual[organfile->divisionalcoupler[k].manual[i]].divisional[m_DivisionalNumber].Push(depth + 1);
				}
			}
		}
	}
}

void MyFrameGeneral::Load(wxFileConfig* cfg, const char* group)
{
	m_ManualNumber = -1;

	INIREAD(NumberOfStops                               ,Integer ),    0,  448);	// the spec says 512, but 64 * 7 = 448
	INIREAD(NumberOfCouplers                            ,Integer ),    0,   64);
	INIREAD(NumberOfTremulants                          ,Integer ),    0,   16);
	INIREAD(NumberOfDivisionalCouplers                  ,Integer ),    0,    8, organfile->GeneralsStoreDivisionalCouplers);

	int i, j, k, m;
	char buffer[64];

	for (i = 0; i < NumberOfStops; i++)
	{
		sprintf(buffer, "StopManual%03d", i + 1);
		m = INIReadInteger(cfg, group, buffer, organfile->FirstManual, organfile->NumberOfManuals);
		sprintf(buffer, "StopNumber%03d", i + 1);
		j = INIReadInteger(cfg, group, buffer, -organfile->manual[m].NumberOfStops, organfile->manual[m].NumberOfStops);
		k = abs(j) - 1;
		if (k >= 0)
		{
			SET_BIT(stop[m], k, 0, true);
			SET_BIT(stop[m], k, 1, j >= 0);
		}
	}
	for (i = 0; i < NumberOfCouplers; i++)
	{
		sprintf(buffer, "CouplerManual%03d", i + 1);
		m = INIReadInteger(cfg, group, buffer, organfile->FirstManual, organfile->NumberOfManuals);
		sprintf(buffer, "CouplerNumber%03d", i + 1);
		j = INIReadInteger(cfg, group, buffer, -organfile->manual[m].NumberOfCouplers, organfile->manual[m].NumberOfCouplers);
		k = abs(j) - 1;
		if (k >= 0)
		{
			SET_BIT(coupler[m], k, 0, true);
			SET_BIT(coupler[m], k, 1, j >= 0);
		}
	}
	for (i = 0; i < NumberOfTremulants; i++)
	{
		sprintf(buffer, "TremulantNumber%03d", i + 1);
		j = INIReadInteger(cfg, group, buffer, -organfile->NumberOfTremulants, organfile->NumberOfTremulants);
		k = abs(j) - 1;
		if (k >= 0)
		{
			SET_BIT(tremulant, k, 0, true);
			SET_BIT(tremulant, k, 1, j >= 0);
		}
	}
	for (i = 0; i < NumberOfDivisionalCouplers; i++)
	{
		sprintf(buffer, "DivisionalCouplerNumber%03d", i + 1);
		j = INIReadInteger(cfg, group, buffer, -organfile->NumberOfDivisionalCouplers, organfile->NumberOfDivisionalCouplers);
		k = abs(j) - 1;
		if (k >= 0)
		{
			SET_BIT(divisionalcoupler, k, 0, true);
			SET_BIT(divisionalcoupler, k, 1, j >= 0);
		}
	}
}

void MyFrameGeneral::Push(int WXUNUSED(depth))
{
	int i, j, k;

	if (g_sound->b_memset)
	{
	    NumberOfStops = NumberOfCouplers = NumberOfTremulants = NumberOfDivisionalCouplers = 0;
		memset(stop, 0, sizeof(stop) + sizeof(coupler) + sizeof(tremulant) + sizeof(divisionalcoupler));
		for (j = organfile->FirstManual; j <= organfile->NumberOfManuals; j++)
		{
			for (i = 0; i < organfile->manual[j].NumberOfStops; i++)
			{
                if (!organfile->CombinationsStoreNonDisplayedDrawstops && !organfile->manual[j].stop[i].Displayed)
                    continue;
                NumberOfStops++;
				SET_BIT(stop[j], i, 0, true);
				SET_BIT(stop[j], i, 1, organfile->manual[j].stop[i].DefaultToEngaged);
			}
		}
		for (j = organfile->FirstManual; j <= organfile->NumberOfManuals; j++)
		{
			for (i = 0; i < organfile->manual[j].NumberOfCouplers; i++)
			{
                if (!organfile->CombinationsStoreNonDisplayedDrawstops && !organfile->manual[j].coupler[i].Displayed)
                    continue;
			    NumberOfCouplers++;
				SET_BIT(coupler[j], i, 0, true);
				SET_BIT(coupler[j], i, 1, organfile->manual[j].coupler[i].DefaultToEngaged);
			}
		}
		for (i = 0; i < organfile->NumberOfTremulants; i++)
		{
            if (!organfile->CombinationsStoreNonDisplayedDrawstops && !organfile->tremulant[i].Displayed)
                continue;
		    NumberOfTremulants++;
			SET_BIT(tremulant, i, 0, true);
			SET_BIT(tremulant, i, 1, organfile->tremulant[i].DefaultToEngaged);
		}
		if (organfile->GeneralsStoreDivisionalCouplers)
		{
			for (i = 0; i < organfile->NumberOfDivisionalCouplers; i++)
			{
			    NumberOfDivisionalCouplers++;
				SET_BIT(divisionalcoupler, i, 0, true);
				SET_BIT(divisionalcoupler, i, 1, organfile->divisionalcoupler[i].DefaultToEngaged);
			}
		}
		::wxGetApp().m_docManager->GetCurrentDocument()->Modify(true);
	}
	else
	{
        for (k = 0; ;k++)
        {
            for (j = organfile->FirstManual; j <= organfile->NumberOfManuals; j++)
            {
                for (i = 0; i < organfile->manual[j].NumberOfStops; i++)
                    if (GET_BIT(stop[j], i, 0))
                        SET_BIT(stop[j], i, 1, k < 2 ? organfile->manual[j].stop[i].Set(GET_BIT(stop[j], i, 1)) : GET_BIT(stop[j], i, 1));
                for (i = 0; i < organfile->manual[j].NumberOfCouplers; i++)
                    if (GET_BIT(coupler[j], i, 0))
                        SET_BIT(coupler[j], i, 1, k < 2 ? organfile->manual[j].coupler[i].Set(GET_BIT(coupler[j], i, 1)) : GET_BIT(coupler[j], i, 1));
            }
            for (i = 0; i < organfile->NumberOfTremulants; i++)
                if (GET_BIT(tremulant, i, 0))
                    SET_BIT(tremulant, i, 1, k < 2 ? organfile->tremulant[i].Set(GET_BIT(tremulant, i, 1)) : GET_BIT(tremulant, i, 1));
            for (i = 0; i < organfile->NumberOfDivisionalCouplers; i++)
                if (GET_BIT(divisionalcoupler, i, 0))
                    SET_BIT(divisionalcoupler, i, 1, k < 2 ? organfile->divisionalcoupler[i].Set(GET_BIT(divisionalcoupler, i, 1)) : GET_BIT(divisionalcoupler, i, 1));
            if (k > 1)
                break;
            MySound::MIDIPretend(!k);
        }
	}

	wxByte used = 0;
    for (j = organfile->FirstManual; j <= organfile->NumberOfManuals; j++)
    {
        for (i = 0; i < 8; i++)
            used |= stop[j][i][1];
        for (i = 0; i < 2; i++)
            used |= coupler[j][i][1];
    }
    used |= tremulant[0][1] | tremulant[1][1] | divisionalcoupler[0][1];

	for (k = 0; k < organfile->NumberOfGenerals; k++)
	{
	    MyGeneral *general = organfile->general + k;
	    int on = ((general == this && used) ? 2 : 0);
	    if ((general->DispImageNum & 2) != on)
	    {
	        general->DispImageNum = (general->DispImageNum & 1) | on;
            wxCommandEvent event(wxEVT_PUSHBUTTON, 0);
            event.SetClientData(general);
            ::wxGetApp().frame->AddPendingEvent(event);
	    }
	}

    for (j = organfile->FirstManual; j <= organfile->NumberOfManuals; j++)
    {
        for (k = 0; k < organfile->manual[j].NumberOfDivisionals; k++)
        {
            MyDivisional *divisional = organfile->manual[j].divisional + k;
            if (divisional->DispImageNum & 2)
            {
                divisional->DispImageNum &= 1;
                wxCommandEvent event(wxEVT_PUSHBUTTON, 0);
                event.SetClientData(divisional);
                ::wxGetApp().frame->AddPendingEvent(event);
            }
        }
    }
}

void MyGeneral::Load(wxFileConfig* cfg, const char* group)
{
	MyFrameGeneral::Load(cfg, group);
	MyPushbutton::Load(cfg, group);
}

void MyPiston::Load(wxFileConfig* cfg, const char* group)
{
	int i, j;
	wxString type = INIReadString(cfg, group, "ObjectType");
	type.MakeUpper();

	if (type == "STOP")
	{
		i = INIReadInteger(cfg, group, "ManualNumber", organfile->FirstManual, organfile->NumberOfManuals);
		j = INIReadInteger(cfg, group, "ObjectNumber", 1, organfile->manual[i].NumberOfStops) - 1;
		drawstop = &organfile->manual[i].stop[j];
	}
	if (type == "COUPLER")
	{
		i = INIReadInteger(cfg, group, "ManualNumber", organfile->FirstManual, organfile->NumberOfManuals);
		j = INIReadInteger(cfg, group, "ObjectNumber", 1, organfile->manual[i].NumberOfCouplers) - 1;
		drawstop = &organfile->manual[i].coupler[j];
	}
	if (type == "TREMULANT")
	{
		j = INIReadInteger(cfg, group, "ObjectNumber", 1, organfile->NumberOfTremulants) - 1;
		drawstop = &organfile->tremulant[j];
	}

	MyPushbutton::Load(cfg, group);
	if (drawstop->DefaultToEngaged ^ drawstop->DisplayInInvertedState)
        DispImageNum ^= 2;
}

void MyPiston::Push(int WXUNUSED(depth))
{
	this->drawstop->Push();
}

class MyThread : public wxThread
{
public:
    MyThread() : wxThread(wxTHREAD_JOINABLE)
    {
    }

    void Go(int file, char* buffer, unsigned length, MyPipe* src)
    {
        m_file = file;
        m_buffer = buffer;
        m_length = length;
        m_src = src;
    }

    void* Entry()
    {
        if (m_src)
        {
            memcpy(m_buffer, m_src, m_length);
        }
        else
        {
            unsigned remain = m_length;
            for(;;)
            {
            #ifdef _WIN32
                remain -= _read(m_file, m_buffer, remain >= BUFSIZ ? BUFSIZ : remain);
	    #endif
	    #ifdef linux
	    	remain -= read(m_file, m_buffer, remain >= BUFSIZ ? BUFSIZ : remain); // TODO better error handling
	    #endif
                if (!remain || TestDestroy())
                    break;
                m_buffer += BUFSIZ;
                Yield();
            }
            #ifdef _WIN32
            _close(m_file);
            #endif
            #ifdef linux
            close(m_file);
            #endif
        }
        return (void*)m_length;
    };

    int m_file;
    char* m_buffer;
    unsigned m_length;
    MyPipe* m_src;
};

wxString MyOrganFile::Load(const wxString& file, const wxString& file2)
{
	wxFileConfig *cfg = 0, *extra_cfg = 0;
	MyApp* app = &::wxGetApp();

	wxProgressDialog dlg(_("Loading sample set"), _("Parsing sample set definition file"), 32768, 0, wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);

	m_filename = file;
	b_squash = wxConfigBase::Get()->Read("LosslessCompression", 1);

	wxString key, key2, error = "!";

  	// NOTICE: unfortunately, the format is not adhered to well at all. with logging enabled, most sample sets generate warnings.
    wxLog::EnableLogging(false);
    m_cfg = cfg = new wxFileConfig(wxEmptyString, wxEmptyString, wxEmptyString, file, wxCONFIG_USE_GLOBAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
    wxLog::EnableLogging(true);

    if (!cfg->GetNumberOfGroups())
	{
		error.Printf("Unable to read '%s'", file.c_str());
		return error;
	}

	b_customized = false;
    long cookie;
    bool bCont = cfg->GetFirstGroup(key, cookie);
    while (bCont)
    {
        if (key.StartsWith("_"))
        {
            b_customized = true;
            cfg->SetPath('/' + key);
            long cookie2;
            bool bCont2 = cfg->GetFirstEntry(key2, cookie2);
            while (bCont2)
            {
                cfg->Write('/' + key.Mid(1) + '/' + key2, cfg->Read(key2));
                bCont2 = cfg->GetNextEntry(key2, cookie2);
            }
            cfg->SetPath("/");
        }
        bCont = cfg->GetNextGroup(key, cookie);
    }

	if (!file2.IsEmpty())
	{
	    // NOTICE: unfortunately, the format is not adhered to well at all. with logging enabled, most sample sets generate warnings.
        wxLog::EnableLogging(false);
		extra_cfg = new wxFileConfig(wxEmptyString, wxEmptyString, wxEmptyString, file2, wxCONFIG_USE_GLOBAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
        wxLog::EnableLogging(true);

		key = "/Organ/ChurchName";
		if (cfg->Read(key).Trim() == extra_cfg->Read(key).Trim())
		{
			long cookie;
			bool bCont = extra_cfg->GetFirstGroup(key, cookie);
			while (bCont)
			{
				extra_cfg->SetPath('/' + key);
				long cookie2;
				bool bCont2 = extra_cfg->GetFirstEntry(key2, cookie2);
				while (bCont2)
				{
					cfg->Write(key + '/' + key2, extra_cfg->Read(key2));
					bCont2 = extra_cfg->GetNextEntry(key2, cookie2);
				}
				extra_cfg->SetPath("/");
				bCont = extra_cfg->GetNextGroup(key, cookie);
			}
		}
		else
		{
			if (!extra_cfg->GetNumberOfGroups())
			{
				error.Printf("Unable to read '%s'", file2.c_str());
				return error;
			}
			::wxLogWarning("This combination file is only compatible with:\n%s", extra_cfg->Read(key).c_str());
		}
		delete extra_cfg;
	}

	int i = 0;

	try
	{
        const char group[] = "Organ";

        INIREAD(HauptwerkOrganFileFormatVersion              ,String  ),  256, false);
        INIREAD(ChurchName                                   ,String  ),  128);
        INIREAD(ChurchAddress                                ,String  ),  128);
        INIREAD(OrganBuilder                                 ,String  ),  128, false);
        INIREAD(OrganBuildDate                               ,String  ),  128, false);
        INIREAD(OrganComments                                ,String  ),  256, false);
        INIREAD(RecordingDetails                             ,String  ),  256, false);
        INIREAD(InfoFilename                                 ,String  ),  256, false);
        wxFileName fn = file;
        if (InfoFilename.IsEmpty())
            fn.SetExt(".html");
        else
            fn.SetFullName(InfoFilename);
        if (fn.FileExists())
            InfoFilename = fn.GetFullPath();
        else
            InfoFilename = wxEmptyString;

        INIREAD(NumberOfManuals                              ,Integer ),    1,    6);
        FirstManual = INIReadBoolean(cfg, group, "HasPedals") ? 0 : 1;
        INIREAD(NumberOfEnclosures                           ,Integer ),    0,    6);
        INIREAD(NumberOfTremulants                           ,Integer ),    0,   10);
        INIREAD(NumberOfWindchestGroups                      ,Integer ),    1,   12);
        INIREAD(NumberOfReversiblePistons                    ,Integer ),    0,   32);
        INIREAD(NumberOfLabels                               ,Integer ),    0,   16);
        INIREAD(NumberOfGenerals                             ,Integer ),    0,   99);
        NumberOfFrameGenerals = 512;	// we never want this to change, what's the point?
        INIREAD(NumberOfDivisionalCouplers                   ,Integer ),    0,    8);
        INIREAD(AmplitudeLevel                               ,Integer ),    0, 1000);
        INIREAD(DivisionalsStoreIntermanualCouplers          ,Boolean ));
        INIREAD(DivisionalsStoreIntramanualCouplers          ,Boolean ));
        INIREAD(DivisionalsStoreTremulants                   ,Boolean ));
        INIREAD(GeneralsStoreDivisionalCouplers              ,Boolean ));
        INIREAD(CombinationsStoreNonDisplayedDrawstops       ,Boolean ));
        INIREAD(DispScreenSizeHoriz                          ,Size    ),    0);
        INIREAD(DispScreenSizeVert                           ,Size    ),    1);
        INIREAD(DispDrawstopBackgroundImageNum               ,Integer ),    1,   30);
        INIREAD(DispConsoleBackgroundImageNum                ,Integer ),    1,   30);
        INIREAD(DispKeyHorizBackgroundImageNum               ,Integer ),    1,   30);
        INIREAD(DispKeyVertBackgroundImageNum                ,Integer ),    1,   30);
        INIREAD(DispDrawstopInsetBackgroundImageNum          ,Integer ),    1,   30);
        INIREAD(DispControlLabelFont                         ,String  ));
        INIREAD(DispShortcutKeyLabelFont                     ,String  ));
        INIREAD(DispShortcutKeyLabelColour                   ,Color   ));
        INIREAD(DispGroupLabelFont                           ,String  ));
        INIREAD(DispDrawstopCols                             ,Integer ),    2,   12);
        INIREAD(DispDrawstopRows                             ,Integer ),    1,   20);
        INIREAD(DispDrawstopColsOffset                       ,Boolean ));
        INIREAD(DispDrawstopOuterColOffsetUp                 ,Boolean ), DispDrawstopColsOffset);
        INIREAD(DispPairDrawstopCols                         ,Boolean ));
        INIREAD(DispExtraDrawstopRows                        ,Integer ),    0,    8);
        INIREAD(DispExtraDrawstopCols                        ,Integer ),    0,   40);
        INIREAD(DispButtonCols                               ,Integer ),    1,   32);
        INIREAD(DispExtraButtonRows                          ,Integer ),    0,    8);
        INIREAD(DispExtraPedalButtonRow                      ,Boolean ));
        INIREAD(DispExtraPedalButtonRowOffset                ,Boolean ), DispExtraPedalButtonRow);
        INIREAD(DispExtraPedalButtonRowOffsetRight           ,Boolean ), DispExtraPedalButtonRow);
        INIREAD(DispButtonsAboveManuals                      ,Boolean ));
        INIREAD(DispTrimAboveManuals                         ,Boolean ));
        INIREAD(DispTrimBelowManuals                         ,Boolean ));
        INIREAD(DispTrimAboveExtraRows                       ,Boolean ));
        INIREAD(DispExtraDrawstopRowsAboveExtraButtonRows    ,Boolean ));

        char buffer[64];

        for (i = FirstManual; i <= NumberOfManuals; i++)
        {
            sprintf(buffer, "Manual%03d", i);
            manual[i].m_ManualNumber = i;
            manual[i].Load(cfg, buffer);
        }

        enclosure = new MyEnclosure[NumberOfEnclosures];
        for (i = 0; i < NumberOfEnclosures; i++)
        {
            sprintf(buffer, "Enclosure%03d", i + 1);
            enclosure[i].Load(cfg, buffer);
        }

        windchest = new MyWindchest[NumberOfTremulants + 1 + NumberOfWindchestGroups];
        for (i = 0; i < NumberOfWindchestGroups; i++)
        {
            sprintf(buffer, "WindchestGroup%03d", i + 1);
            windchest[NumberOfTremulants + 1 + i].Load(cfg, buffer);
        }

        tremulant = new MyTremulant[NumberOfTremulants];
        for (i = 0; i < NumberOfTremulants; i++)
        {
            sprintf(buffer, "Tremulant%03d", i + 1);
            tremulant[i].Load(cfg, buffer);
        }

        piston = new MyPiston[NumberOfReversiblePistons];
        for (i = 0; i < NumberOfReversiblePistons; i++)
        {
            sprintf(buffer, "ReversiblePiston%03d", i + 1);
            piston[i].Load(cfg, buffer);
        }

        label = new MyLabel[NumberOfLabels];
        for (i = 0; i < NumberOfLabels; i++)
        {
            sprintf(buffer, "Label%03d", i + 1);
            label[i].Load(cfg, buffer);
        }

        general = new MyGeneral[NumberOfGenerals];
        for (i = 0; i < NumberOfGenerals; i++)
        {
            sprintf(buffer, "General%03d", i + 1);
            general[i].Load(cfg, buffer);
        }

        framegeneral = new MyFrameGeneral[NumberOfFrameGenerals];
        for (i = 0; i < NumberOfFrameGenerals; i++)
        {
            sprintf(buffer, "General%03d", i + 100);
            framegeneral[i].Load(cfg, buffer);
            framegeneral[i].ObjectNumber = i + 100;
        }

        divisionalcoupler = new MyDivisionalCoupler[NumberOfDivisionalCouplers];
        for (i = 0; i < NumberOfDivisionalCouplers; i++)
        {
            sprintf(buffer, "DivisionalCoupler%03d", i + 1);
            divisionalcoupler[i].Load(cfg, buffer);
        }
	}
	catch (wxString error_)
	{
	    return error_;
	}
#ifdef __VFD__
    MyLCD_WriteLineOne(ChurchName+" "+OrganBuilder);
    MyLCD_WriteLineTwo("Loading...");
#endif
	pipe = new MyPipe*[NumberOfPipes + NumberOfTremulants];
	memset(pipe, 0, sizeof(MyPipe*) * (NumberOfPipes + NumberOfTremulants));

	m_path = file;
	m_path.MakeAbsolute();
	wxString temp, prev_key;
	m_path.SetCwd(m_path.GetPath());
	int j, progress = 0, mbuffer_which = 0;
	bool memory = false;
	char *mbuffer_p[2] = {0, 0}, *ptr, *compress;
	short *s_ptr;
	unsigned mbuffer_s[2] = {0, 0}, compress_s = 0, prev_length = 0;
	m_compress_p = 0;
	MyPipeHash newhash(NumberOfPipes);
	MyPipeHash::iterator it;
	int ffile = -1;

	std::vector<wxString> pipe_keys;
	int ii = 0, i_e, i_prev = -1;
	ii = (int)pipe_files.size();
	for (i = 0; i < ii; i++)
	{
        if (pipe_files[i].StartsWith("REF:"))
        {
            pipe_keys.push_back(wxEmptyString);
            continue;
        }
        m_path = pipe_files[i];
        m_path.MakeAbsolute();
        key.Printf("  %s-%c%08X", m_path.GetFullPath().c_str(), (g_sound->b_stereo ? 2 : 0) + (b_squash ? '1' : '0'), 0x00000000);
        key[0] = key[1] = 0;        // for my amusement: used to be the below line :-)
        // key = "\0\0" + key;

        it = newhash.find(key);
        if (it != newhash.end())
        {
            it->second = (MyPipe*)(((char*)(it->second)) + 1);
            key[0] = ((int)(it->second)) & 255;
            key[1] = ((int)(it->second)) >> 8;
        }

		newhash[key] = 0;
		pipe_keys.push_back(key);
	}

	for (it = newhash.begin(); it != newhash.end(); it++)
        it->second = 0;
	for (it = pipehash.begin(); it != pipehash.end(); it++)
	{
		MyPipeHash::iterator it2 = newhash.find(it->first);
		if (it2 == newhash.end() || it->first[0] || it->first[1])
			free(it->second);
		else
			it2->second = it->second;
	}
	pipehash = newhash;

    int max=ii+organfile->NumberOfTremulants;
    for (i = -1; i < max; i++)
    {
        i_e = i + 1;

        if (i_e < ii)
        {
            if (pipe_files[i_e].StartsWith("REF:"))
                continue;
            if (!dlg.Update(((progress + 1) << 15) / (int)(organfile->NumberOfPipes + organfile->NumberOfTremulants + 1), pipe_files[i_e]))
            {
                error = "!";
                break;
            }
            // 327: max parameter to progress dialog divided by 100 to calculate percentage
            int n=(((progress + 1) << 15) / (int)(organfile->NumberOfPipes + organfile->NumberOfTremulants + 1))/327;
#ifdef __VFD__
            MyLCD_WriteLineTwo(wxString::Format("Loading %d%%", n));
#endif
        }
        else if (i_e != ii && !dlg.Update(((progress + 1) << 15) / (int)(organfile->NumberOfPipes + organfile->NumberOfTremulants + 1), wxString("Tremulant ") << organfile->tremulant[i - ii].ObjectNumber))
        {
            error = "!";
            break;
        }

        if (i_e < ii)
            key = pipe_keys[i_e];
        if (i_e < ii && pipehash[key])
            pipe[progress] = pipehash[key];
        else
        {
            MyThread helper;
            bool b_threading = false;
            try
            {
                if (!memory)
                {
                    wxMemorySize ram_left;
                    ram_left = ::wxGetFreeMemory();
                    if (ram_left < LOW_MEM_CONDITION)
                    {
                        memory = true;
                        if (ram_left > -1 && ::wxMessageBox(_("Physical memory is low. The system may behave\nin unexpected ways in low memory situations.\n\nContinue loading?"), APP_NAME, wxYES_NO | wxICON_EXCLAMATION, app->frame) == wxNO)
                            throw (char*)_(" Out of memory loading");
                    }
                }

                unsigned wavestart = 0;
                unsigned wavesize = 0;
                unsigned loopstart = 0;
                unsigned loopend = 0;
                unsigned release = 0, q, r, length = 0;
                int channels = 0, peak = 0, amp = 10000, k, jj;
                MyPipe* repeater = 0;

                if (key.length() >= 2 && (key[0] || key[1]))
                {
                    temp = key;
                    temp[0] = temp[1] = 0;
                    repeater = pipehash[temp];
                }

                if (i_e < ii)
                {
                    if (repeater)
                    {
                        length = offsetof(MyPipe, data);
                    }
                    else
                    {
                        temp = key.BeforeLast('-').c_str() + 2;
                        if (::wxFileExists(key.c_str() + 2) && (!::wxFileExists(temp) || ::wxFileModificationTime(temp) <= ::wxFileModificationTime(key.c_str() + 2)))
                        {
                            temp = key.c_str() + 2;
                            key[1] = key[1] | 0x80;
                        }
                        // Open file, because of efficiency wxFile is not used
                        #ifdef linux
                        temp.Replace("\\", "/");
                        ffile = open(temp, O_RDONLY);
                        struct stat ffile_info;
                        fstat(ffile, &ffile_info);
                        length = ffile_info.st_size;
                        #endif
                        #ifdef _WIN32
                        ffile = _open(temp, _O_BINARY | _O_RDONLY | _O_SEQUENTIAL);
                        length = _filelength(ffile);
                        #endif
                        if (ffile == -1) // TODO: better errorhandling (linux)
                        {
                            i = i_e;
                            throw (char*)" Failed to open";
                        }
                    }
                }
                else if (i >= ii)
                {
                    channels = 1;
                    wavestart = 4;
                    loopstart = 441000 / organfile->tremulant[i - ii].StartRate;
                    release = loopend = loopstart + 441 * organfile->tremulant[i - ii].Period / 10;
                    wavesize = release + 441000 / organfile->tremulant[i - ii].StopRate;
                    length = 8 + sizeof(short) * wavesize;
                }

                if (i_e != ii)
                {
                    if (!mbuffer_p[mbuffer_which])
                        ptr = (char*)malloc(mbuffer_s[mbuffer_which] = length + 8);
                    else if (length + 8 > mbuffer_s[mbuffer_which])
                        ptr = (char*)realloc(mbuffer_p[mbuffer_which], mbuffer_s[mbuffer_which] = length + 8);
                    else
                        ptr = (char*)mbuffer_p[mbuffer_which];
                    if (!ptr)
                    {
                        i = i_e;
                        throw (char*)_(" Out of memory loading");
                    }
                    mbuffer_p[mbuffer_which] = ptr;
                }

                if (i_e < ii)
                {
                    i_e = i_prev;
                    i_prev = i + 1;
                    helper.Go(ffile, mbuffer_p[mbuffer_which], length, repeater);
                    if (helper.Create() != wxTHREAD_NO_ERROR || helper.Run() != wxTHREAD_NO_ERROR)
                    {
                        helper.Delete();
                        i = i_e;
                        throw (char*)_(" Out of memory loading");
                    }
                    b_threading = true;
                    mbuffer_which ^= 1;
                }
                else if (i_e == ii)
                {
                    i_e = i_prev;
                    i_prev = -1;	// shouldn't matter
                    mbuffer_which ^= 1;
                }

                ptr = mbuffer_p[mbuffer_which];

                if (i >= ii)
                {
                    prev_key.Empty();
                    double trem_amp   = peak = (16384 * organfile->tremulant[i - ii].AmpModDepth / 100);
                    double trem_param = 0.14247585730565955729989312395825 / (double)organfile->tremulant[i - ii].Period;
                    double trem_fade, trem_inc, trem_angle;
                    s_ptr = (short*)(ptr + wavestart);

                    trem_inc = 1.0 / (double)loopstart;
                    trem_fade = trem_angle = 0.0;
                    j = -(int)loopstart;
                    s_ptr -= j;
                    do
                    {
                        s_ptr[j] = SynthTrem(trem_amp, trem_angle, trem_fade);
                        trem_angle += trem_param * trem_fade;
                        trem_fade += trem_inc;
                    }
                    while (++j);

                    j = (int)loopstart - (int)loopend;
                    s_ptr -= j;
                    do
                    {
                        s_ptr[j] = SynthTrem(trem_amp, trem_angle);
                        trem_angle += trem_param;
                    }
                    while (++j);

                    trem_inc = 1.0 / (double)(wavesize - release);
                    trem_fade = 1.0 - trem_inc;
                    j = (int)release - (int)wavesize;
                    s_ptr -= j;
                    do
                    {
                        s_ptr[j] = SynthTrem(trem_amp, trem_angle, trem_fade);
                        trem_angle += trem_param * trem_fade;
                        trem_fade -= trem_inc;
                    }
                    while (++j);
                }
                else if (prev_length)
                {
                    if (!prev_key[0] && !prev_key[1])
                    {
                        RIFF riff;
                        if (prev_length >= 12)
                        {
                            riff.Analyze(ptr, 0);
                            if (!(riff == "RIFF" && (riff.GetSize() == prev_length - 8 || riff.GetSize() == prev_length) && !strncmp(ptr + 8, "WAVE", 4)))
                                throw (char*)"<Invalid WAV file";
                        }
                        else
                            throw (char*)"<Invalid WAV file";

                        for (j = 12; j + 8 <= (int)prev_length; )
                        {
                            riff.Analyze(ptr, j);
                            j += 8;
                            if (riff == "data")
                            {
                                wavestart = j;
                                wavesize = riff.GetSize() / 2;
                            }
                            jj = j;
                            j += (riff.GetSize() + 1) & 0xFFFFFFFE;
                            if (j > (int)prev_length)
                                throw (char*)"<Invalid WAV file";

                            if (riff == "fmt ")
                            {
                                if (riff.GetSize() < 16 || wxINT16_SWAP_ON_BE(*(short*)(ptr + j - riff.GetSize())) != 1)
                                    throw (char*)"<Not PCM data in";
                                channels = wxUINT16_SWAP_ON_BE(*(unsigned short*)(ptr + j - riff.GetSize() + 2));
                                if (channels < 1 || channels > 2)
                                    throw (char*)"<More than 2 channels in";
                                if (wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + j - riff.GetSize() + 4)) != 44100)
                                    throw (char*)"<Not 44.1kHz sampling rate in";
                            }
                            else if (riff == "cue " && !pipe_percussive[i])
                            {
                                if (riff.GetSize() < 4)
                                    throw (char*)"<Invalid CUE chunk in";
                                k = wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + j - riff.GetSize()));
                                if (riff.GetSize() < 4 + 24 * (unsigned)k)
                                    throw (char*)"<Invalid CUE chunk in";
                                jj += 4;
                                while (k)
                                {
                                    q = wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + jj + 20)) * channels;
                                    if (q > release)
                                        release = q;
                                    k--;
                                    jj += 24;
                                }
                            }
                            else if (riff == "smpl" && !pipe_percussive[i])
                            {
                                // TODO: we could support multiple sample loops?
                                if (riff.GetSize() < 36)
                                    throw (char*)"<Invalid SMPL chunk in";
                                k = wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + j - riff.GetSize() + 28));
                                if (riff.GetSize() < 36 + 24 * (unsigned)k)
                                    throw (char*)"<Invalid SMPL chunk in";

                                jj += 36;
                                // NOTE: for now, we just get the *longest* loop.
                                while (k)
                                {
                                    q = wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + jj +  8)) * channels;
                                    r = wxUINT32_SWAP_ON_BE(*(unsigned*)(ptr + jj + 12)) * channels + channels;
                                    if (r - q > loopend - loopstart)
                                    {
                                        loopend   = r;
                                        loopstart = q;
                                    }
                                    k--;
                                    jj += 24;
                                }
                            }
                        }
                        if (!prev_length || j != (int)prev_length)
                            throw (char*)"<Invalid WAV file";

                        register int peaktemp;
                        s_ptr = (short*)(ptr + wavestart) + wavesize;
                        if (channels == 1)
                        {
                            for (k = -(int)wavesize; k; )
                            {
                                if (s_ptr[k] & 0x8000)
                                    s_ptr[k]++;
                                peaktemp  = s_ptr[k++];
                                peaktemp ^= (peaktemp >> 15);
                                peak >?= peaktemp;
                            }
                        }
                        else
                        {
                            for (k = -(int)wavesize; k; )
                            {
                                if (s_ptr[k] & 0x8000)
                                    s_ptr[k]++;
                                peaktemp  = s_ptr[k++];
                                if (s_ptr[k] & 0x8000)
                                    s_ptr[k]++;
                                peaktemp += s_ptr[k++];
                                peaktemp ^= (peaktemp >> 15);
                                peak >?= peaktemp;
                            }
                            peak >>= 1;
                        }

                        // learning lesson: never ever trust the range values of outside sources to be correct!
                        if (loopstart >= loopend || loopstart >= wavesize || loopend >= wavesize || release >= wavesize || !loopend || !release)
                            loopstart = loopend = release = 0;
                    }
                    amp = pipe_amplitudes[progress];
                }

                if (prev_length || i >= ii)
                {
                    if (!prev_key.IsEmpty() && (prev_key[0] || prev_key[1]))
                    {
                        if (prev_key[1] & 0x80)
                        {
                            if (prev_length < sizeof(MyPipe) || ((MyPipe*)ptr)->_fourcc != *(unsigned*)"MyOc" || adler32(0, (Bytef*)&((MyPipe*)ptr)->_this, prev_length - offsetof(MyPipe, _this)) != ((MyPipe*)ptr)->_adler32)
                            {
                                ::wxRemoveFile(prev_key.c_str() + 2);
                                throw (char*)"<Invalid cache";
                            }
                            pipe[progress] = (MyPipe*)ptr;
                            mbuffer_p[mbuffer_which] = 0;
                            pipe_filesizes.push_back(prev_length);
                            for (k = 0; k < 3; k++)
                                pipe[progress]->ptr[k] -= (unsigned)pipe[progress]->_this;
                        }
                        else
                        {
                            pipe[progress] = (MyPipe*)malloc(prev_length);
                            if (!pipe[progress])
                                throw (char*)_("<Out of memory loading");
                            memcpy(pipe[progress], mbuffer_p[mbuffer_which], prev_length);
                        }
                    }
                    else
                    {
                        k = offsetof(MyPipe, data) + sizeof(short) * (12 + loopstart + (loopend - loopstart) + (wavesize - release));
                        if (!m_compress_p)
                            compress = (char*)malloc(compress_s = k);
                        else if (k > (int)compress_s)
                            compress = (char*)realloc(m_compress_p, compress_s = k);
                        else
                            compress = (char*)m_compress_p;
                        if (!compress)
                            throw (char*)_("<Out of memory loading");
                        m_compress_p = (MyPipe*)compress;

                        compress = (char*)m_compress_p->data;
                        *(int*)(ptr + wavestart - 4) = 0;
                        *(int*)(ptr + wavestart + sizeof(short) * wavesize) = 0;

                        m_compress_p->ra_volume = peak;
                        m_compress_p->ra_factor = (PHASE_ALIGN_RES << 25) / (peak + 1);

                        if (release)
                        {
                            if (release + channels < wavesize)
                                release += channels;
                            if (loopstart & channels)
                            {
                                wavestart -= channels << 1;
                                loopstart += channels;
                                loopend   += channels;
                                release   += channels;
                                wavesize  += channels;
                            }
                            if ((wavesize - release) & channels)
                                wavesize   += channels;

                            CompressWAV(compress, m_compress_p->f[0], (short*)(ptr + wavestart), loopstart, channels, 0);
                            CompressWAV(compress, m_compress_p->f[1], (short*)(ptr + wavestart) + loopstart, loopend - loopstart, channels, 1);
                            CompressWAV(compress, m_compress_p->f[2], (short*)(ptr + wavestart) + release, wavesize - release, channels, 2);
                            m_compress_p->instances = 0;
                        }
                        else
                        {
                            if (wavesize & channels)
                                wavesize += channels;
                            CompressWAV(compress, m_compress_p->f[0], (short*)(ptr + wavestart), wavesize, channels, 0);
                            m_compress_p->instances = -1;
                        }

                        k = compress - (char*)m_compress_p;
                        pipe[progress] = (MyPipe*)malloc(k);
                        if (!pipe[progress])
                            throw (char*)_("<Out of memory loading");
                        memcpy(pipe[progress], m_compress_p, k);
                        if (!prev_key.IsEmpty())
                            pipe_filesizes.push_back(k);
                    }
                }
                else
                    i_e = -1;

                if (i_e > -1)
                {
                    pipe[progress]->ra_shift = 7;
                    while (amp > 10000)
                    {
                        pipe[progress]->ra_shift--;
                        amp >>= 1;
                    }
                    pipe[progress]->ra_amp = (amp << 15) / -10000;
                    pipe[progress]->sampler = 0;
                    if (!prev_key.IsEmpty())
                    {
                        prev_key[1] = prev_key[1] & 0x7F;
                        pipehash[prev_key] = pipe[progress];
                    }
                    if (prev_key.IsEmpty() || (!prev_key[0] && !prev_key[1]))
                    {
                        for (k = 0; k < 3; k++)
                            pipe[progress]->ptr[k] += (unsigned)pipe[progress];
                        if (!prev_key.IsEmpty())
                            pipe_filenames.push_back(prev_key.c_str() + 2);
                    }
                    else
                    {
                        pipe_filenames.push_back(wxEmptyString);
                        pipe_filesizes.push_back(0);
                    }
                }

                if (b_threading)
                {
                    prev_length = (int)helper.Wait();
                    prev_key = key;
                }
            }
            catch (char* str)
            {
                if (b_threading && helper.IsAlive())
                    helper.Delete();
                error.Printf("%s '%s'", str + 1, i < ii ? (str[0] == '<' ? prev_key.c_str() + 2 : key.c_str() + 2) : "Tremulant");
                break;
            }
        }

        if (i_e >= ii)
        {
            organfile->tremulant[i - ii].pipe = pipe[progress];
            pipe[progress++]->WindchestGroup = i - ii;
        }
        else if (i_e > -1)
        {
            *pipe_ptrs[i_e] = (short)progress;
            pipe[progress++]->WindchestGroup = pipe_windchests[i_e];
        }
    }
    if (mbuffer_p[0])
        free(mbuffer_p[0]);
    if (mbuffer_p[1])
        free(mbuffer_p[1]);
    if (m_compress_p)
        free(m_compress_p);
    if (i < ii + organfile->NumberOfTremulants)
        return error;

    for (i = 0; i < ii; i++)
    {
        if (!pipe_files[i].StartsWith("REF:"))
            continue;
        int manual, stop, pipe;
        sscanf(pipe_files[i].c_str() + 4, "%d:%d:%d", &manual, &stop, &pipe);
        if (manual >= organfile->FirstManual && manual <= organfile->NumberOfManuals)
        {
            if (stop >= 1 && stop <= organfile->manual[manual].NumberOfStops)
            {
                if (pipe >= 1 && pipe <= organfile->manual[manual].stop[stop-1].NumberOfLogicalPipes)
                    *pipe_ptrs[i] = organfile->manual[manual].stop[stop-1].pipe[pipe-1];
                else
                    return "Invalid reference " + pipe_files[i];
            }
            else
                return "Invalid reference " + pipe_files[i];
        }
        else
            return "Invalid reference " + pipe_files[i];
    }

	for (i = 0; i < 9; i++)
	{
		wxMemoryInputStream mem((const char*)Images_Wood[i + 18], c_Images_Wood[i + 18]);
		wxImage img(mem, wxBITMAP_TYPE_GIF);
		m_images[i] = wxBitmap(img);
	}

	pipe_files.clear();
	pipe_ptrs.clear();
	pipe_amplitudes.clear();
	pipe_windchests.clear();
	pipe_percussive.clear();

	if (m_cfg)
	{
	    ::wxGetApp().frame->m_meters[0]->SetValue(m_cfg->Read("/Organ/Volume", g_sound->volume));
        delete m_cfg;
        m_cfg = 0;
	}
#ifdef __VFD__
    MyLCD_WriteLineTwo("Ready!");
#endif
	return wxEmptyString;
}

MyOrganFile::~MyOrganFile(void)
{
	for (int i = 0; i < NumberOfTremulants; i++)
        if (tremulant && tremulant[i].pipe)
            free(tremulant[i].pipe);
	if (!m_opening)
	{
		for (MyPipeHash::iterator it = pipehash.begin(); it != pipehash.end(); it++)
            if (it->second)
                free(it->second);	// important: these are done with malloc/realloc/free, not new/delete
		pipehash.clear();
	}
	if (pipe)
		delete[] pipe;
	if (divisionalcoupler)
		delete[] divisionalcoupler;
	if (framegeneral)
		delete[] framegeneral;
	if (general)
		delete[] general;
	if (label)
		delete[] label;
	if (piston)
		delete[] piston;
	if (windchest)
		delete[] windchest;
	if (tremulant)
		delete[] tremulant;
	if (enclosure)
		delete[] enclosure;
	if (m_cfg)
		delete m_cfg;
}

void SaveHelper(wxFileConfig& cfg, bool prefix, wxString group, wxString key, wxString value)
{
    wxString str = group + '/' + key;
    if (cfg.Read(str) != value)
    {
        if (prefix)
        {
            organfile->b_customized = true;
            cfg.Write('_' + str, value);
        }
        else
            cfg.Write(str, value);
    }
}

void SaveHelper(wxFileConfig& cfg, bool prefix, wxString group, wxString key, int value, bool sign = false, bool force = false)
{
    wxString str;
    if (force)
        str.Printf("%03d", value);
    else if (sign)
    {
        if (value >= 0)
            str.Printf("+%03d", value);
        else
            str.Printf("-%03d", -value);
    }
    else
        str.Printf("%d", value);
    SaveHelper(cfg, prefix, group, key, str);
}

void MyDrawstop::Save(wxFileConfig& cfg, bool prefix, wxString group)
{
    group.Printf("%s%03d", group.c_str(), ObjectNumber);
    SaveHelper(cfg, prefix, group, "DefaultToEngaged", DefaultToEngaged ? "Y" : "N");
    SaveHelper(cfg, prefix, group, "StopControlMIDIKeyNumber", StopControlMIDIKeyNumber);
}

void MyPushbutton::Save(wxFileConfig& cfg, bool prefix, wxString group)
{
    group.Printf("%s%03d", group.c_str(), ObjectNumber);
    SaveHelper(cfg, prefix, group, "MIDIProgramChangeNumber", MIDIProgramChangeNumber);
}

void MyDivisional::Save(wxFileConfig& cfg, bool prefix, wxString group)
{
    MyPushbutton::Save(cfg, prefix, group);

    group.Printf("%s%03d", group.c_str(), ObjectNumber);
    SaveHelper(cfg, prefix, group, "NumberOfStops", NumberOfStops);
    SaveHelper(cfg, prefix, group, "NumberOfCouplers", NumberOfCouplers);
    SaveHelper(cfg, prefix, group, "NumberOfTremulants", NumberOfTremulants);

	int i, k;
	char buffer[64];

    for (i = 0, k = 1; i < organfile->manual[m_ManualNumber].NumberOfStops; i++)
    {
        if (GET_BIT(stop, i, 0))
        {
            sprintf(buffer, "Stop%03d", k++);
            SaveHelper(cfg, prefix, group, buffer, GET_BIT(stop, i, 1) ? i + 1 : -1 - i, true);
        }
    }
    for (i = 0, k = 1; i < organfile->manual[m_ManualNumber].NumberOfCouplers; i++)
    {
        if (GET_BIT(coupler, i, 0))
        {
			sprintf(buffer, "Coupler%03d", k++);
            SaveHelper(cfg, prefix, group, buffer, GET_BIT(coupler, i, 1) ? i + 1 : -1 - i, true);
        }
    }
    for (i = 0, k = 1; i < organfile->manual[m_ManualNumber].NumberOfTremulants; i++)
    {
        if (GET_BIT(tremulant, i, 0))
        {
			sprintf(buffer, "Tremulant%03d", k++);
            SaveHelper(cfg, prefix, group, buffer, GET_BIT(tremulant, i, 1) ? i + 1 : -1 - i, true);
        }
    }
}

void MyFrameGeneral::Save(wxFileConfig& cfg, bool prefix, wxString group)
{
    if (ObjectNumber > 99 && !(NumberOfStops | NumberOfCouplers | NumberOfTremulants | NumberOfDivisionalCouplers))
        return;

    MyPushbutton::Save(cfg, prefix, group);

    group.Printf("%s%03d", group.c_str(), ObjectNumber);
    SaveHelper(cfg, prefix, group, "NumberOfStops", NumberOfStops);
    SaveHelper(cfg, prefix, group, "NumberOfCouplers", NumberOfCouplers);
    SaveHelper(cfg, prefix, group, "NumberOfTremulants", NumberOfTremulants);
    SaveHelper(cfg, prefix, group, "NumberOfDivisionalCouplers", NumberOfDivisionalCouplers);

	int i, j, k;
	char buffer[64];

    for (j = organfile->FirstManual, k = 1; j <= organfile->NumberOfManuals; j++)
        for (i = 0; i < organfile->manual[j].NumberOfStops; i++)
            if (GET_BIT(stop[j], i, 0))
            {
                sprintf(buffer, "StopManual%03d", k);
                SaveHelper(cfg, prefix, group, buffer, j, true, true);
                sprintf(buffer, "StopNumber%03d", k++);
                SaveHelper(cfg, prefix, group, buffer, GET_BIT(stop[j], i, 1) ? i + 1 : -1 - i, true);
            }
    for (j = organfile->FirstManual, k = 1; j <= organfile->NumberOfManuals; j++)
        for (i = 0; i < organfile->manual[j].NumberOfCouplers; i++)
            if (GET_BIT(coupler[j], i, 0))
            {
                sprintf(buffer, "CouplerManual%03d", k);
                SaveHelper(cfg, prefix, group, buffer, j, true, true);
                sprintf(buffer, "CouplerNumber%03d", k++);
                SaveHelper(cfg, prefix, group, buffer, GET_BIT(coupler[j], i, 1) ? i + 1 : -1 - i, true);
            }
    for (i = 0, k = 1; i < organfile->NumberOfTremulants; i++)
        if (GET_BIT(tremulant, i, 0))
        {
			sprintf(buffer, "TremulantNumber%03d", k++);
            SaveHelper(cfg, prefix, group, buffer, GET_BIT(tremulant, i, 1) ? i + 1 : -1 - i, true);
        }
    for (i = 0, k = 1; i < organfile->NumberOfDivisionalCouplers; i++)
        if (GET_BIT(divisionalcoupler, i, 0))
        {
			sprintf(buffer, "DivisionalCouplerNumber%03d", k++);
            SaveHelper(cfg, prefix, group, buffer, GET_BIT(divisionalcoupler, i, 1) ? i + 1 : -1 - i, true);
        }
}

void MyOrganFile::Revert(wxFileConfig& cfg)
{
    if (!b_customized)
        return;
    long cookie;
    wxString key;
    bool bCont = cfg.GetFirstGroup(key, cookie);
    wxArrayString to_drop;
    while (bCont)
    {
        if (key.StartsWith("_"))
            to_drop.Add(key);
        bCont = cfg.GetNextGroup(key, cookie);
    }
    for (unsigned i = 0; i < to_drop.Count(); i++)
        cfg.DeleteGroup(to_drop[i]);
}

void MyOrganFile::Save(const wxString& file)
{
    wxFileName fn(file);
    bool prefix = true;

    if (fn.GetExt().CmpNoCase("organ"))
    {
        if (::wxFileExists(file) && !::wxRemoveFile(file))
        {
            ::wxLogError("Could not write to '%s'", file.c_str());
            return;
        }
        prefix = false;
    }

    wxLog::EnableLogging(false);
    wxFileConfig cfg(wxEmptyString, wxEmptyString, file, wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
    wxLog::EnableLogging(true);
    if (prefix)
        Revert(cfg);

    SaveHelper(cfg, prefix, "Organ", "ChurchName", ChurchName);
    SaveHelper(cfg, prefix, "Organ", "ChurchAddress", ChurchAddress);
    SaveHelper(cfg, prefix, "Organ", "HauptwerkOrganFileFormatVersion", HauptwerkOrganFileFormatVersion);
    SaveHelper(cfg, prefix, "Organ", "NumberOfFrameGenerals", NumberOfFrameGenerals);
    SaveHelper(cfg, prefix, "Organ", "Volume", g_sound->volume);

    int i, j;

	for (i = organfile->FirstManual; i <= organfile->NumberOfManuals; i++)
	{
		for (j = 0; j < organfile->manual[i].NumberOfStops; j++)
			organfile->manual[i].stop[j].Save(cfg, prefix);
		for (j = 0; j < organfile->manual[i].NumberOfCouplers; j++)
			organfile->manual[i].coupler[j].Save(cfg, prefix);
		for (j = 0; j < organfile->manual[i].NumberOfDivisionals; j++)
			organfile->manual[i].divisional[j].Save(cfg, prefix);
	}
	for (j = 0; j < organfile->NumberOfTremulants; j++)
		organfile->tremulant[j].Save(cfg, prefix);
	for (j = 0; j < organfile->NumberOfDivisionalCouplers; j++)
		organfile->divisionalcoupler[j].Save(cfg, prefix);
	for (j = 0; j < organfile->NumberOfGenerals; j++)
		organfile->general[j].Save(cfg, prefix);
	for (j = 0; j < organfile->NumberOfFrameGenerals; j++)
		organfile->framegeneral[j].Save(cfg, prefix);
	for (j = 0; j < organfile->NumberOfReversiblePistons; j++)
		organfile->piston[j].Save(cfg, prefix);

}
