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
 * MA 02111-1307, USA.
 */

#ifndef GORGUEVECTOR_H
#define GORGUEVECTOR_H

#ifdef __GNUC__
#define GO_VECTOR_SIZE 4
typedef float GOVector __attribute__ ((vector_size (16)));
static inline GOVector GOConstVector(float a)
{
	GOVector x = { (a), (a), (a), (a) };
	return x;
}
#else
#define GO_VECTOR_SIZE 1
typedef float GOVector;
static inline GOVector GOConstVector(float a)
{
	return a;
}
#endif

static inline void GOVectorFill(float* res, float val, unsigned elements)
{
	GOVector* result = (GOVector*)res;
	const GOVector value = GOConstVector(val);
	for(unsigned i = 0; i < elements / GO_VECTOR_SIZE; i++)
		result[i] = value;
}

static inline void GOVectorAdd(float* res, const float* val, unsigned elements)
{
	GOVector* result = (GOVector*)res;
	const GOVector* value = (const GOVector*) val;
	for(unsigned i = 0; i < elements / GO_VECTOR_SIZE; i++)
		result[i] += value[i];
}

static inline void GOVectorMul(float* res, const float* val, unsigned elements)
{
	GOVector* result = (GOVector*)res;
	const GOVector* value = (const GOVector*) val;
	for(unsigned i = 0; i < elements / GO_VECTOR_SIZE; i++)
		result[i] *= value[i];
}

static inline void GOVectorMulConst(float* res, float val, unsigned elements)
{
	GOVector* result = (GOVector*)res;
	const GOVector value = GOConstVector(val);
	for(unsigned i = 0; i < elements / GO_VECTOR_SIZE; i++)
		result[i] *= value;
}

#endif
