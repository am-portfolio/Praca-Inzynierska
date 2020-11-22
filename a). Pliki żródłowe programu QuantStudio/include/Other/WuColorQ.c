#pragma once

#ifndef DWORD
#define BYTE unsigned char
#define DWORD unsigned int
#define USHORT unsigned short
#define TRUE true
#define FALSE false
#endif

/**********************************************************************
C Implementation of Wu's Color Quantizer (v. 2)
(see Graphics Gems vol. II, pp. 126-133)
Author:	Xiaolin Wu
Dept. of Computer Science
Univ. of Western Ontario
London, Ontario N6A 5B7
wu@csd.uwo.ca
Algorithm: Greedy orthogonal bipartition of RGB space for variance
minimization aided by inclusion-exclusion tricks.
For speed no nearest neighbor search is done. Slightly
better performance can be expected by more sophisticated
but more expensive versions.
The author thanks Tom Lane at Tom_Lane@G.GP.CS.CMU.EDU for much of
additional documentation and a cure to a previous bug.
Free to distribute, comments and suggestions are appreciated.
**********************************************************************/

#include <memory.h>
#include <malloc.h>

#include<stdio.h>

#define WU_MAXCOLOR	256
#define	RED	2
#define	GREEN	1
#define BLUE	0

namespace wucq
{
struct box {
	int r0;			 /* min value, exclusive */
	int r1;			 /* max value, inclusive */
	int g0;
	int g1;
	int b0;
	int b1;
	int vol;
};
typedef struct box _BOX;

/* Histogram is in elements 1..HISTSIZE along each axis,
* element 0 is for base or marginal value
* NB: these must start out 0!
*/


float m2[33][33][33];
int wt[33][33][33];
int mr[33][33][33];
int mg[33][33][33];
int mb[33][33][33];


// These items set from main
DWORD *Idata; // 32 bit pointer to ARGB
int size; //image size, how many ARGB items
int K; /*color look - up table size*/
	   //USHORT *Qadd;

void Hist3d(int *vwt, int *vmr, int *vmg, int *vmb, float *m2)
{
	int ind, r, g, b;
	int inr, ing, inb, table[256];
	int i;

	// zero out elements
	for (int i = 0; i < 33 * 33 * 33; i++)
	{
		m2[i] = 0.0f;
		vwt[i] = vmr[i] = vmg[i] = vmb[i] = 0;
	}

	for (i = 0; i < 256; ++i)
		table[i] = i * i;

	//Qadd = (USHORT *)malloc(sizeof(USHORT) * size);

	//if (Qadd == NULL) { return false; }

	for (i = 0; i < size; i++)
	{
		b = (Idata[i] >> 16) & 0xFF;
		g = (Idata[i] >> 8) & 0xFF;
		r = (Idata[i]) & 0xFF;

		inr = (r >> 3) + 1;
		ing = (g >> 3) + 1;
		inb = (b >> 3) + 1;

		ind = (inr << 10) + (inr << 6) + inr + (ing << 5) + ing + inb;
		//Qadd[i] = ind;

		/*[inr][ing][inb]*/

		++vwt[ind];
		vmr[ind] += r;
		vmg[ind] += g;
		vmb[ind] += b;
		m2[ind] += (float)(table[r] + table[g] + table[b]);
	}
}

/* At conclusion of the histogram step, we can interpret
*   wt[r][g][b] = sum over voxel of P(c)
*   mr[r][g][b] = sum over voxel of r*P(c)  ,  similarly for mg, mb
*   m2[r][g][b] = sum over voxel of c^2*P(c)
* Actually each of these should be divided by 'size' to give the usual
* interpretation of P() as ranging from 0 to 1, but we needn't do that here.
*/

/* We now convert histogram into moments so that we can rapidly calculate
* the sums of the above quantities over any desired box.
*/


void M3d(int *vwt, int *vmr, int *vmg, int *vmb, float *m2) /*compute cumulative moments.*/
{
	unsigned short int ind1, ind2;
	unsigned char i, r, g, b;
	int line, line_r, line_g, line_b, area[33], area_r[33], area_g[33], area_b[33];
	float line2, area2[33];

	for (r = 1; r <= 32; ++r)
	{
		for (i = 0; i <= 32; ++i)
		{
			area2[i] = 0.0f;
			area[i] = area_r[i] = area_g[i] = area_b[i] = 0;
		}

		for (g = 1; g <= 32; ++g)
		{
			line2 = 0.0f;
			line = line_r = line_g = line_b = 0;

			for (b = 1; b <= 32; ++b)
			{
				ind1 = (r << 10) + (r << 6) + r + (g << 5) + g + b; /* [r][g][b] */
				line += vwt[ind1];
				line_r += vmr[ind1];
				line_g += vmg[ind1];
				line_b += vmb[ind1];
				line2 += m2[ind1];
				area[b] += line;
				area_r[b] += line_r;
				area_g[b] += line_g;
				area_b[b] += line_b;
				area2[b] += line2;
				ind2 = ind1 - 1089; /* [r-1][g][b] */
				vwt[ind1] = vwt[ind2] + area[b];
				vmr[ind1] = vmr[ind2] + area_r[b];
				vmg[ind1] = vmg[ind2] + area_g[b];
				vmb[ind1] = vmb[ind2] + area_b[b];
				m2[ind1] = m2[ind2] + area2[b];
			}
		}
	}
}


int Vol(box *cube, int mmt[33][33][33]) /*Compute sum over a box of any given statistic*/
{
	return(mmt[cube->r1][cube->g1][cube->b1]
		- mmt[cube->r1][cube->g1][cube->b0]
		- mmt[cube->r1][cube->g0][cube->b1]
		+ mmt[cube->r1][cube->g0][cube->b0]
		- mmt[cube->r0][cube->g1][cube->b1]
		+ mmt[cube->r0][cube->g1][cube->b0]
		+ mmt[cube->r0][cube->g0][cube->b1]
		- mmt[cube->r0][cube->g0][cube->b0]);
}

/* The next two routines allow a slightly more efficient calculation
* of Vol() for a proposed subbox of a given box.  The sum of Top()
* and Bottom() is the Vol() of a subbox split in the given direction
* and with the specified new upper bound.
*/

int Bottom(box *cube, unsigned char dir, int mmt[33][33][33])
/*Compute part of Vol(cube, mmt) that doesn't depend on r1, g1, or b1 (depending on dir)*/
{
	switch (dir) {
	case RED:
		return(-mmt[cube->r0][cube->g1][cube->b1]
			+ mmt[cube->r0][cube->g1][cube->b0]
			+ mmt[cube->r0][cube->g0][cube->b1]
			- mmt[cube->r0][cube->g0][cube->b0]);
		break;
	case GREEN:
		return(-mmt[cube->r1][cube->g0][cube->b1]
			+ mmt[cube->r1][cube->g0][cube->b0]
			+ mmt[cube->r0][cube->g0][cube->b1]
			- mmt[cube->r0][cube->g0][cube->b0]);
		break;
	case BLUE:
		return(-mmt[cube->r1][cube->g1][cube->b0]
			+ mmt[cube->r1][cube->g0][cube->b0]
			+ mmt[cube->r0][cube->g1][cube->b0]
			- mmt[cube->r0][cube->g0][cube->b0]);
		break;
	}
}


int Top(box *cube, BYTE dir, int pos, int mmt[33][33][33])
/*Compute remainder of Vol(cube, mmt), substituting pos for r1, g1, or b1(depending on dir)*/
{
	switch (dir) {
	case RED:
		return(mmt[pos][cube->g1][cube->b1]
			- mmt[pos][cube->g1][cube->b0]
			- mmt[pos][cube->g0][cube->b1]
			+ mmt[pos][cube->g0][cube->b0]);
		break;
	case GREEN:
		return(mmt[cube->r1][pos][cube->b1]
			- mmt[cube->r1][pos][cube->b0]
			- mmt[cube->r0][pos][cube->b1]
			+ mmt[cube->r0][pos][cube->b0]);
		break;
	case BLUE:
		return(mmt[cube->r1][cube->g1][pos]
			- mmt[cube->r1][cube->g0][pos]
			- mmt[cube->r0][cube->g1][pos]
			+ mmt[cube->r0][cube->g0][pos]);
		break;
	}
}


float Var(box *cube)
/* Compute the weighted variance of a box */
/* NB: as with the raw statistics, this is really the variance * size */
{
	float dr, dg, db, xx;

	dr = Vol(cube, mr);
	dg = Vol(cube, mg);
	db = Vol(cube, mb);
	xx = m2[cube->r1][cube->g1][cube->b1]
		- m2[cube->r1][cube->g1][cube->b0]
		- m2[cube->r1][cube->g0][cube->b1]
		+ m2[cube->r1][cube->g0][cube->b0]
		- m2[cube->r0][cube->g1][cube->b1]
		+ m2[cube->r0][cube->g1][cube->b0]
		+ m2[cube->r0][cube->g0][cube->b1]
		- m2[cube->r0][cube->g0][cube->b0];

	return(xx - (dr*dr + dg * dg + db * db) / (float)Vol(cube, wt));
}

/* We want to minimize the sum of the variances of two subboxes.
* The sum(c^2) terms can be ignored since their sum over both subboxes
* is the same (the sum for the whole box) no matter where we split.
* The remaining terms have a minus sign in the variance formula,
* so we drop the minus sign and MAXIMIZE the sum of the two terms.
*/


float Maximize(box *cube, unsigned char dir,
	int first, int last, int *cut,
	int whole_r, int whole_g, int whole_b, int whole_w)
{
	int half_r, half_g, half_b, half_w;
	int base_r, base_g, base_b, base_w;
	int i;
	float temp, max;

	base_r = Bottom(cube, dir, mr);
	base_g = Bottom(cube, dir, mg);
	base_b = Bottom(cube, dir, mb);
	base_w = Bottom(cube, dir, wt);
	max = 0.0;
	*cut = -1;
	for (i = first; i < last; ++i) {
		half_r = base_r + Top(cube, dir, i, mr);
		half_g = base_g + Top(cube, dir, i, mg);
		half_b = base_b + Top(cube, dir, i, mb);
		half_w = base_w + Top(cube, dir, i, wt);
		/* now half_x is sum over lower half of box, if split at i */
		if (half_w == 0) {      /* subbox could be empty of pixels! */
			continue;             /* never split into an empty box */
		}
		else
			temp = ((float)half_r*half_r + (float)half_g*half_g +
			(float)half_b*half_b) / half_w;

		half_r = whole_r - half_r;
		half_g = whole_g - half_g;
		half_b = whole_b - half_b;
		half_w = whole_w - half_w;
		if (half_w == 0) {      /* subbox could be empty of pixels! */
			continue;             /* never split into an empty box */
		}
		else
			temp += ((float)half_r*half_r + (float)half_g*half_g +
			(float)half_b*half_b) / half_w;

		if (temp > max) { max = temp; *cut = i; }
	}
	return(max);
}

int Cut(box *set1, box *set2)
{
	unsigned char dir;
	int cutr, cutg, cutb;
	float maxr, maxg, maxb;
	long int whole_r, whole_g, whole_b, whole_w;

	whole_r = Vol(set1, mr);
	whole_g = Vol(set1, mg);
	whole_b = Vol(set1, mb);
	whole_w = Vol(set1, wt);

	maxr = Maximize(set1, RED, set1->r0 + 1, set1->r1, &cutr,
		whole_r, whole_g, whole_b, whole_w);
	maxg = Maximize(set1, GREEN, set1->g0 + 1, set1->g1, &cutg,
		whole_r, whole_g, whole_b, whole_w);
	maxb = Maximize(set1, BLUE, set1->b0 + 1, set1->b1, &cutb,
		whole_r, whole_g, whole_b, whole_w);

	if ((maxr >= maxg) && (maxr >= maxb)) {
		dir = RED;
		if (cutr < 0) return 0; /* can't split the box */
	}
	else
		if ((maxg >= maxr) && (maxg >= maxb))
			dir = GREEN;
		else
			dir = BLUE;

	set2->r1 = set1->r1;
	set2->g1 = set1->g1;
	set2->b1 = set1->b1;

	switch (dir) {
	case RED:
		set2->r0 = set1->r1 = cutr;
		set2->g0 = set1->g0;
		set2->b0 = set1->b0;
		break;
	case GREEN:
		set2->g0 = set1->g1 = cutg;
		set2->r0 = set1->r0;
		set2->b0 = set1->b0;
		break;
	case BLUE:
		set2->b0 = set1->b1 = cutb;
		set2->r0 = set1->r0;
		set2->g0 = set1->g0;
		break;
	}
	set1->vol = (set1->r1 - set1->r0)*(set1->g1 - set1->g0)*(set1->b1 - set1->b0);
	set2->vol = (set2->r1 - set2->r0)*(set2->g1 - set2->g0)*(set2->b1 - set2->b0);
	return 1;
}

/*
void Mark(box *cube, int label, unsigned char *tag)
{
register int r, g, b;

for (r = cube->r0 + 1; r <= cube->r1; ++r)
for (g = cube->g0 + 1; g <= cube->g1; ++g)
for (b = cube->b0 + 1; b <= cube->b1; ++b)
tag[(r << 10) + (r << 6) + r + (g << 5) + g + b] = label;
}
*/

//MAKSYMALNA ILOŒC KOLOROW TO 256 - USTAWIONE W WU_MAXCOLOR
//Format pikseli i palety to RGBA
int WuColorQuantizer(unsigned int *pixels, unsigned int image_size, unsigned int colors, unsigned int* output_palette)
{
	bool update_bmp = false;

	box		cube[WU_MAXCOLOR];
	//BYTE	*tag;
	BYTE	lut_r[WU_MAXCOLOR], lut_g[WU_MAXCOLOR], lut_b[WU_MAXCOLOR];
	int     next;
	int		i, weight;
	int		k;
	float	vv[WU_MAXCOLOR], temp;


	/****************************************/

	/* input R,G,B components into Ir, Ig, Ib;
	set size to width*height */
	Idata = pixels;
	size = image_size;
	K = colors; // 250 color lookup

				/****************************************/


	Hist3d(&wt[0][0][0], &mr[0][0][0], &mg[0][0][0], &mb[0][0][0], &m2[0][0][0]);
	M3d(&wt[0][0][0], &mr[0][0][0], &mg[0][0][0], &mb[0][0][0], &m2[0][0][0]);

	cube[0].r0 = cube[0].g0 = cube[0].b0 = 0;
	cube[0].r1 = cube[0].g1 = cube[0].b1 = 32;
	next = 0;
	for (i = 1; i < K; ++i) {
		if (Cut(&cube[next], &cube[i])) {
			/* volume test ensures we won't try to cut one-cell box */
			vv[next] = (cube[next].vol > 1) ? Var(&cube[next]) : 0.0;
			vv[i] = (cube[i].vol > 1) ? Var(&cube[i]) : 0.0;
		}
		else {
			vv[next] = 0.0;   /* don't try to split this box again */
			i--;              /* didn't create box i */
		}
		next = 0; temp = vv[0];
		for (k = 1; k <= i; ++k)
			if (vv[k] > temp) {
				temp = vv[k]; next = k;
			}
		if (temp <= 0.0) {
			K = i + 1;
			//fprintf(stderr, "Only got %d boxes\n", K);
			break;
		}
	}
	//printf("Partition done\n");

	/* the space for array m2 can be freed now */

	//tag = (BYTE *)malloc(33 * 33 * 33);

	/*
	if (tag == NULL)
	{
	//free(Qadd); Qadd = NULL;
	return false;
	}
	*/


	for (k = 0; k < K; k++)
	{
		//Mark(&cube[k], k, tag);
		weight = Vol(&cube[k], wt);

		if (weight)
		{
			lut_r[k] = Vol(&cube[k], mr) / weight;
			lut_g[k] = Vol(&cube[k], mg) / weight;
			lut_b[k] = Vol(&cube[k], mb) / weight;
		}
		else
		{
			//fprintf(stderr, "bogus box %d\n", k);
			lut_r[k] = lut_g[k] = lut_b[k] = 0;
		}

		/****************************************/

		output_palette[k] = 0xFF000000 | (lut_r[k]) | (lut_g[k] << 8) | (lut_b[k] << 16); // put pla back into calling program var ARGB

																						  /****************************************/
	}

	return K;
}
}

#undef	RED
#undef	GREEN
#undef BLUE

#undef BYTE unsigned char
#undef DWORD unsigned int
#undef USHORT unsigned short
#undef TRUE true
#undef FALSE false