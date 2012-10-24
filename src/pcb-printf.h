/*!
 * \file src/pcb-printf.h
 *
 * \brief This file defines a wrapper around sprintf, that defines new
 * specifiers that take pcb Coord objects as input.
 *
 * There is a fair bit of nasty (repetitious) code in here, but I feel
 * the gain in clarity for output code elsewhere in the project will
 * make it worth it.\n
 * \n
 * The new specifiers are:\n
 * <table>
 * <tr><td> %mm </td><td> output a measure in mm. </td></tr>
 * <tr><td>%mM </td><td> output a measure in scaled (mm /\f$\mu\f$m) metric. </td></tr>
 * <tr><td>%ml </td><td> output a measure in mil. </td></tr>
 * <tr><td>%mL </td><td> output a measure in scaled (mil / in) imperial. </td></tr>
 * <tr><td>%ms </td><td> output a measure in most natural mm / mil units. </td></tr>
 * <tr><td>%mS </td><td> output a measure in most natural scaled units. </td></tr>
 * <tr><td>%md </td><td> output a pair of measures in most natural mm / mil units. </td></tr>
 * <tr><td>%mD </td><td> output a pair of measures in most natural scaled units. </td></tr>
 * <tr><td>%m3 </td><td> output 3 measures in most natural scaled units. </td></tr>
 * <tr><td>... </td><td>  </td></tr>
 * <tr><td>%m9 </td><td> output 9 measures in most natural scaled units. </td></tr>
 * <tr><td>%m* </td><td> output a measure with unit given as an additional.\n
 *            const char* parameter. </td></tr>
 * <tr><td>%m+ </td><td> accepts an e_allow parameter that masks all subsequent
 *            "natural" (S/D/3/.../9) specifiers to only use certain
 *            units.\n
 * <tr><td>%mr </td><td> output a measure in a unit readable by parse_l.l
 *            (this will always append a unit suffix). </td></tr>
 * <tr><td>%ma </td><td> output an angle in degrees (expects degrees). </td></tr>
 * </table>
 * \n
 * These accept the usual printf modifiers for %f, as well as:\n
 * <table>
 * <tr><td> $ </td><td> output a unit suffix after the measure. </td></tr>
 * <tr><td> # </td><td> prevents all scaling for %mS/D/1/.../9 (this should
 *          ONLY be used for debug code since its output exposes
 *          pcb's base units). </td></tr>
 * <tr><td> ` </td><td> always use '.' as decimal separator (note that %mr uses
 *          this by default). </td></tr>
 * </table>
 * \n
 *
 * \todo No support for %zu size_t printf spec.\n
 */

/*
 *                            COPYRIGHT
 *
 *  PCB, interactive printed circuit board design
 *  Copyright (C) 2011 Andrew Poelstra
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Contact addresses for paper mail and Email:
 *  Andrew Poelstra, 16966 60A Ave, V3S 8X5 Surrey, BC, Canada
 *  asp11@sfu.ca
 *
 */

#ifndef	PCB_PCB_PRINTF_H
#define	PCB_PCB_PRINTF_H

enum e_allow {
  NO_PRINT = 0,		/* suffixes we can read but not print (i.e., "inch") */
  ALLOW_NM = 1,
  ALLOW_UM = 2,
  ALLOW_MM = 4,
  ALLOW_CM = 8,
  ALLOW_M  = 16,
  ALLOW_KM = 32,

  ALLOW_CMIL = 1024,
  ALLOW_MIL  = 2048,
  ALLOW_IN   = 4096,

  ALLOW_METRIC   = ALLOW_NM | ALLOW_UM | ALLOW_MM |
                   ALLOW_CM | ALLOW_M  | ALLOW_KM,
  ALLOW_IMPERIAL = ALLOW_CMIL | ALLOW_MIL | ALLOW_IN,
  /* This is all units allowed in parse_l.l */
#if 0
  ALLOW_READABLE = ALLOW_NM | ALLOW_UM | ALLOW_MM |
                   ALLOW_M  | ALLOW_KM | ALLOW_CMIL |
                   ALLOW_MIL | ALLOW_IN,
#else
  ALLOW_READABLE = ALLOW_MIL | ALLOW_MM,
#endif

  ALLOW_ALL = ~0
};

enum e_family { METRIC, IMPERIAL };
enum e_suffix {
  NO_SUFFIX, /*!< No suffix.  */
  SUFFIX, /*!< Suffix, prefixed with ' '. */
  FILE_MODE_NO_SUFFIX, /*!< No suffix, force '.' as decimal. */
  FILE_MODE /*!< Suffix, force '.' as decimal. */
};

struct unit {
  int index; /*!< Index into Unit[] list. */
  const char *suffix;
  const char *in_suffix; /*!< Internationalized suffix. */
  char printf_code;
  double scale_factor;
  enum e_family family;
  enum e_allow  allow;
  int default_prec;
  /* used for gui spinboxes */
  double step_tiny;
  double step_small;
  double step_medium;
  double step_large;
  double step_huge;
  const char *alias[1]; /*!< Aliases, right now we only need 1 ("inch"->"in"), add as needed. */

};

struct increments {
  const char *suffix;
  /* key g and <shift>g value  */
  Coord grid;
  Coord grid_min;
  Coord grid_max;
  /* key s and <shift>s value  */
  Coord size;
  Coord size_min;
  Coord size_max;
  /* key l and <shift>l value  */
  Coord line;
  Coord line_min;
  Coord line_max;
  /* key k and <shift>k value  */
  Coord clear;
  Coord clear_min;
  Coord clear_max;
};

void initialize_units();

const Unit *get_unit_struct (const char *suffix);
const Unit *get_unit_list (void);
int get_n_units (void);
double coord_to_unit (const Unit *, Coord);
Coord  unit_to_coord (const Unit *, double);
Increments *get_increments_struct (enum e_family family);
void copy_nonzero_increments (Increments *dst, const Increments *src);

int pcb_fprintf(FILE *f, const char *fmt, ...);
int pcb_sprintf(char *string, const char *fmt, ...);
int pcb_printf(const char *fmt, ...);
char *pcb_g_strdup_printf(const char *fmt, ...);
gchar *pcb_vprintf(const char *fmt, va_list args);

#endif

