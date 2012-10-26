/*!
 * \file src/rtree.h
 *
 * \author Copyright (c) 2004 Harry Eaton, it's based on C. Scott's
 * kdtree.h template.
 *
 * \brief Prototypes for r-tree routines.
 */

/*
 *                            COPYRIGHT
 *
 *  PCB, interactive printed circuit board design
 *  Copyright (C) 1994,1995,1996 Thomas Nau
 *  Copyright (C) 1998,1999,2000,2001 harry eaton
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
 *  harry eaton, 6697 Buttonhole Ct, Columbia, MD 21044 USA
 *  haceaton@aplcomm.jhuapl.edu
 *
 */

#ifndef PCB_RTREE_H
#define PCB_RTREE_H

#include "global.h"


/*!
 * \brief Create a rtree from the list of boxes.
 *
 * If 'manage' is true, then the tree will take ownership of 'boxlist'
 * and free it when the tree is destroyed.
 */
rtree_t *r_create_tree (const BoxType * boxlist[], int N, int manage);

/*!
 * \brief Destroy a rtree.
 */
void r_destroy_tree (rtree_t ** rtree);

bool r_delete_entry (rtree_t * rtree, const BoxType * which);
void r_insert_entry (rtree_t * rtree, const BoxType * which, int manage);

/*!
 * \brief Generic search routine.
 *
 * \c region_in_search should return true if "what you're looking for"
 * is within the specified region; regions, like rectangles, are closed
 * on top and left and open on bottom and right.\n
 * \c rectangle_in_region should return true if the given rectangle is
 * "what you're looking for".\n
 * The search will find all rectangles matching the criteria given
 * by \c region_in_search and \c rectangle_in_region and return a count
 * of how many things \c rectangle_in_region returned true for.\n
 * closure is used to abort the search if desired from within
 * \c rectangel_in_region.\n
 * Look at the implementation of \c r_region_is_empty for how to
 * abort the search if that is the desired behavior.
 */

int r_search (rtree_t * rtree, const BoxType * starting_region,
	      int (*region_in_search) (const BoxType * region, void *cl),
	      int (*rectangle_in_region) (const BoxType * box, void *cl),
	      void *closure);
static inline int r_search_pt (rtree_t * rtree, const PointType * pt,
	      int radius,
	      int (*region_in_search) (const BoxType * region, void *cl),
	      int (*rectangle_in_region) (const BoxType * box, void *cl),
	      void *closure)
{
  BoxType box;

  box.X1 = pt->X - radius;
  box.X2 = pt->X + radius;
  box.Y1 = pt->Y - radius;
  box.Y2 = pt->Y + radius;

  return r_search(rtree, &box, region_in_search, rectangle_in_region, closure);
}

/*!
 * \brief Special-purpose searches build upon \c r_search.
 *
 * \return 0 if there are any rectangles in the given region.
 */
int r_region_is_empty (rtree_t * rtree, const BoxType * region);

void __r_dump_tree (struct rtree_node *, int);

#endif
