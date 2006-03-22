/* $Id$ */

/*
 *                            COPYRIGHT
 *
 *  PCB, interactive printed circuit board design
 *  Copyright (C) 1994,1995,1996, 2004 Thomas Nau
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
 *  Thomas Nau, Schlehenweg 15, 88471 Baustetten, Germany
 *  Thomas.Nau@rz.uni-ulm.de
 *
 */


/* misc functions used by several modules
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdarg.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <memory.h>
#include <ctype.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <pwd.h>

#include "global.h"

#include "crosshair.h"
#include "create.h"
#include "data.h"
#include "draw.h"
#include "file.h"
#include "error.h"
#include "mymem.h"
#include "misc.h"
#include "move.h"
#include "output.h"
#include "remove.h"
#include "rtree.h"
#include "rotate.h"
#include "rubberband.h"
#include "search.h"
#include "set.h"
#include "undo.h"
#include "action.h"


RCSID ("$Id$");




/*	forward declarations	*/
static char *BumpName (char *);
static void RightAngles (int, float *, float *);
static void GetGridLockCoordinates (int, void *, void *, void *, LocationType *,
				    LocationType *);


/* Local variables */

/* 
 * Used by SaveStackAndVisibility() and 
 * RestoreStackAndVisibility()
 */

static struct
{
  Boolean ElementOn, InvisibleObjectsOn, PinOn, ViaOn, RatOn;
  int LayerStack[MAX_LAYER];
  Boolean LayerOn[MAX_LAYER];
  int cnt;
} SavedStack;

/* Get Value returns a numeric value passed from the string and sets the
 * Boolean variable absolute to False if it leads with a +/- character
 */
float
GetValue (char *val, char *units, Boolean *absolute)
{
  float value;

  /* if the first character is a sign we have to add the
   * value to the current one
   */
  if (*val == '=')
    {
      *absolute = True;
      value = atof (val + 1);
    }
  else
    {
      if (isdigit (*val))
	*absolute = True;
      else
	*absolute = False;
      value = atof (val);
    }
  if (units && *units)
    {
      if (strncasecmp (units, "mm", 2) == 0)
	value *= MM_TO_COOR;
      else if (strncasecmp (units, "mil", 3) == 0)
	value *= 100;
    }
  return value;
}

/* ---------------------------------------------------------------------------
 * sets the bounding box of a point (which is silly)
 */
void
SetPointBoundingBox (PointTypePtr Pnt)
{
  Pnt->X2 = Pnt->X;
  Pnt->Y2 = Pnt->Y;
}

/* ---------------------------------------------------------------------------
 * sets the bounding box of a pin or via
 */
void
SetPinBoundingBox (PinTypePtr Pin)
{
  BDimension width;

  /* the bounding box covers the extent of influence
   * so it must include the clearance values too
   */
  width = (Pin->Clearance + 1) / 2 + (Pin->Thickness + 1) / 2;
  width = MAX (width, (Pin->Mask + 1) / 2);
  Pin->BoundingBox.X1 = Pin->X - width;
  Pin->BoundingBox.Y1 = Pin->Y - width;
  Pin->BoundingBox.X2 = Pin->X + width;
  Pin->BoundingBox.Y2 = Pin->Y + width;
}

/* ---------------------------------------------------------------------------
 * sets the bounding box of a pad
 */
void
SetPadBoundingBox (PadTypePtr Pad)
{
  BDimension width;

  /* the bounding box covers the extent of influence
   * so it must include the clearance values too
   */
  width = (Pad->Thickness + Pad->Clearance + 1) / 2;
  width = MAX (width, (Pad->Mask + 1) / 2);
  Pad->BoundingBox.X1 = MIN (Pad->Point1.X, Pad->Point2.X) - width;
  Pad->BoundingBox.X2 = MAX (Pad->Point1.X, Pad->Point2.X) + width;
  Pad->BoundingBox.Y1 = MIN (Pad->Point1.Y, Pad->Point2.Y) - width;
  Pad->BoundingBox.Y2 = MAX (Pad->Point1.Y, Pad->Point2.Y) + width;
}

/* ---------------------------------------------------------------------------
 * sets the bounding box of a line
 */
void
SetLineBoundingBox (LineTypePtr Line)
{
  BDimension width;

  width = (Line->Thickness + Line->Clearance + 1) / 2;

  Line->BoundingBox.X1 = MIN (Line->Point1.X, Line->Point2.X) - width;
  Line->BoundingBox.X2 = MAX (Line->Point1.X, Line->Point2.X) + width;
  Line->BoundingBox.Y1 = MIN (Line->Point1.Y, Line->Point2.Y) - width;
  Line->BoundingBox.Y2 = MAX (Line->Point1.Y, Line->Point2.Y) + width;
  SetPointBoundingBox (&Line->Point1);
  SetPointBoundingBox (&Line->Point2);
}

/* ---------------------------------------------------------------------------
 * sets the bounding box of a polygons
 */
void
SetPolygonBoundingBox (PolygonTypePtr Polygon)
{
  Polygon->BoundingBox.X1 = Polygon->BoundingBox.Y1 = MAX_COORD;
  Polygon->BoundingBox.X2 = Polygon->BoundingBox.Y2 = 0;
  POLYGONPOINT_LOOP (Polygon);
  {
    MAKEMIN (Polygon->BoundingBox.X1, point->X);
    MAKEMIN (Polygon->BoundingBox.Y1, point->Y);
    MAKEMAX (Polygon->BoundingBox.X2, point->X);
    MAKEMAX (Polygon->BoundingBox.Y2, point->Y);
  }
  END_LOOP;
}

/* ---------------------------------------------------------------------------
 * sets the bounding box of an elements
 */
void
SetElementBoundingBox (DataTypePtr Data, ElementTypePtr Element,
		       FontTypePtr Font)
{
  BoxTypePtr box, vbox;

  if (Data && Data->element_tree)
    r_delete_entry (Data->element_tree, (BoxType *) Element);
  /* first update the text objects */
  ELEMENTTEXT_LOOP (Element);
  {
    if (Data && Data->name_tree[n])
      r_delete_entry (Data->name_tree[n], (BoxType *) text);
    SetTextBoundingBox (Font, text);
    if (Data && !Data->name_tree[n])
      Data->name_tree[n] = r_create_tree (NULL, 0, 0);
    if (Data)
      r_insert_entry (Data->name_tree[n], (BoxType *) text, 0);
  }
  END_LOOP;

  /* do not include the elementnames bounding box which
   * is handles seperatly
   */
  box = &Element->BoundingBox;
  vbox = &Element->VBox;
  box->X1 = box->Y1 = MAX_COORD;
  box->X2 = box->Y2 = 0;
  ELEMENTLINE_LOOP (Element);
  {
    SetLineBoundingBox (line);
    MAKEMIN (box->X1, line->Point1.X - (line->Thickness + 1) / 2);
    MAKEMIN (box->Y1, line->Point1.Y - (line->Thickness + 1) / 2);
    MAKEMIN (box->X1, line->Point2.X - (line->Thickness + 1) / 2);
    MAKEMIN (box->Y1, line->Point2.Y - (line->Thickness + 1) / 2);
    MAKEMAX (box->X2, line->Point1.X + (line->Thickness + 1) / 2);
    MAKEMAX (box->Y2, line->Point1.Y + (line->Thickness + 1) / 2);
    MAKEMAX (box->X2, line->Point2.X + (line->Thickness + 1) / 2);
    MAKEMAX (box->Y2, line->Point2.Y + (line->Thickness + 1) / 2);
  }
  END_LOOP;
  ARC_LOOP (Element);
  {
    SetArcBoundingBox (arc);
    MAKEMIN (box->X1, arc->BoundingBox.X1);
    MAKEMIN (box->Y1, arc->BoundingBox.Y1);
    MAKEMAX (box->X2, arc->BoundingBox.X2);
    MAKEMAX (box->Y2, arc->BoundingBox.Y2);
  }
  END_LOOP;
  *vbox = *box;
  PIN_LOOP (Element);
  {
    if (Data && Data->pin_tree)
      r_delete_entry (Data->pin_tree, (BoxType *) pin);
    SetPinBoundingBox (pin);
    if (Data)
      {
	if (!Data->pin_tree)
	  Data->pin_tree = r_create_tree (NULL, 0, 0);
	r_insert_entry (Data->pin_tree, (BoxType *) pin, 0);
      }
    MAKEMIN (box->X1, pin->BoundingBox.X1);
    MAKEMIN (box->Y1, pin->BoundingBox.Y1);
    MAKEMAX (box->X2, pin->BoundingBox.X2);
    MAKEMAX (box->Y2, pin->BoundingBox.Y2);
    MAKEMIN (vbox->X1, pin->X - pin->Thickness / 2);
    MAKEMIN (vbox->Y1, pin->Y - pin->Thickness / 2);
    MAKEMAX (vbox->X2, pin->X + pin->Thickness / 2);
    MAKEMAX (vbox->Y2, pin->Y + pin->Thickness / 2);
  }
  END_LOOP;
  PAD_LOOP (Element);
  {
    if (Data && Data->pad_tree)
      r_delete_entry (Data->pad_tree, (BoxType *) pad);
    SetPadBoundingBox (pad);
    if (Data)
      {
	if (!Data->pad_tree)
	  Data->pad_tree = r_create_tree (NULL, 0, 0);
	r_insert_entry (Data->pad_tree, (BoxType *) pad, 0);
      }
    MAKEMIN (box->X1, pad->BoundingBox.X1);
    MAKEMIN (box->Y1, pad->BoundingBox.Y1);
    MAKEMAX (box->X2, pad->BoundingBox.X2);
    MAKEMAX (box->Y2, pad->BoundingBox.Y2);
    MAKEMIN (vbox->X1,
	     MIN (pad->Point1.X, pad->Point2.X) - pad->Thickness / 2);
    MAKEMIN (vbox->Y1,
	     MIN (pad->Point1.Y, pad->Point2.Y) - pad->Thickness / 2);
    MAKEMAX (vbox->X2,
	     MAX (pad->Point1.X, pad->Point2.X) + pad->Thickness / 2);
    MAKEMAX (vbox->Y2,
	     MAX (pad->Point1.Y, pad->Point2.Y) + pad->Thickness / 2);
  }
  END_LOOP;
  /* now we set the EDGE2FLAG of the pad if Point2
   * is closer to the outside edge than Point1
   */
  PAD_LOOP (Element);
  {
    if (pad->Point1.Y == pad->Point2.Y)
      {
	/* horizontal pad */
	if (box->X2 - pad->Point2.X < pad->Point1.X - box->X1)
	  SET_FLAG (EDGE2FLAG, pad);
	else
	  CLEAR_FLAG (EDGE2FLAG, pad);
      }
    else
      {
	/* vertical pad */
	if (box->Y2 - pad->Point2.Y < pad->Point1.Y - box->Y1)
	  SET_FLAG (EDGE2FLAG, pad);
	else
	  CLEAR_FLAG (EDGE2FLAG, pad);
      }
  }
  END_LOOP;

  /* mark pins with component orientation */
  if ((box->X2 - box->X1) > (box->Y2 - box->Y1))
    {
      PIN_LOOP (Element);
      {
	SET_FLAG (EDGE2FLAG, pin);
      }
      END_LOOP;
    }
  else
    {
      PIN_LOOP (Element);
      {
	CLEAR_FLAG (EDGE2FLAG, pin);
      }
      END_LOOP;
    }
  if (Data && !Data->element_tree)
    Data->element_tree = r_create_tree (NULL, 0, 0);
  if (Data)
    r_insert_entry (Data->element_tree, box, 0);
}

/* ---------------------------------------------------------------------------
 * creates the bounding box of a text object
 */
void
SetTextBoundingBox (FontTypePtr FontPtr, TextTypePtr Text)
{
  SymbolTypePtr symbol = FontPtr->Symbol;
  unsigned char *s = (unsigned char *) Text->TextString;
  LocationType width = 0, height = 0;
  BDimension maxThick = 0;
  int i;

  /* calculate size of the bounding box */
  for (; s && *s; s++)
    if (*s <= MAX_FONTPOSITION && symbol[*s].Valid)
      {
	LineTypePtr line = symbol[*s].Line;
	for (i = 0; i < symbol[*s].LineN; line++, i++)
	  if (line->Thickness > maxThick)
	    maxThick = line->Thickness;
	width += symbol[*s].Width + symbol[*s].Delta;
	height = MAX (height, (LocationType) symbol[*s].Height);
      }
    else
      {
	width +=
	  ((FontPtr->DefaultSymbol.X2 - FontPtr->DefaultSymbol.X1) * 6 / 5);
	height = (FontPtr->DefaultSymbol.Y2 - FontPtr->DefaultSymbol.Y1);
      }

  /* scale values */
  width *= Text->Scale / 100.;
  height *= Text->Scale / 100.;
  maxThick *= Text->Scale / 200.;
  if (maxThick < 400)
    maxThick = 400;

  /* set upper-left and lower-right corner;
   * swap coordinates if necessary (origin is already in 'swapped')
   * and rotate box
   */
  Text->BoundingBox.X1 = Text->X;
  Text->BoundingBox.Y1 = Text->Y;
  if (TEST_FLAG (ONSOLDERFLAG, Text))
    {
      Text->BoundingBox.X1 -= maxThick;
      Text->BoundingBox.Y1 -= SWAP_SIGN_Y (maxThick);
      Text->BoundingBox.X2 =
	Text->BoundingBox.X1 + SWAP_SIGN_X (width + maxThick);
      Text->BoundingBox.Y2 =
	Text->BoundingBox.Y1 + SWAP_SIGN_Y (height + 2 * maxThick);
      RotateBoxLowLevel (&Text->BoundingBox, Text->X, Text->Y,
			 (4 - Text->Direction) & 0x03);
    }
  else
    {
      Text->BoundingBox.X1 -= maxThick;
      Text->BoundingBox.Y1 -= maxThick;
      Text->BoundingBox.X2 = Text->BoundingBox.X1 + width + maxThick;
      Text->BoundingBox.Y2 = Text->BoundingBox.Y1 + height + 2 * maxThick;
      RotateBoxLowLevel (&Text->BoundingBox,
			 Text->X, Text->Y, Text->Direction);
    }
}

/* ---------------------------------------------------------------------------
 * returns True if data area is empty
 */
Boolean
IsDataEmpty (DataTypePtr Data)
{
  Boolean hasNoObjects;
  Cardinal i;

  hasNoObjects = (Data->ViaN == 0);
  hasNoObjects &= (Data->ElementN == 0);
  for (i = 0; i < MAX_LAYER + 2; i++)
    hasNoObjects = hasNoObjects &&
      Data->Layer[i].LineN == 0 &&
      Data->Layer[i].ArcN == 0 &&
      Data->Layer[i].TextN == 0 && Data->Layer[i].PolygonN == 0;
  return (hasNoObjects);
}

int
FlagIsDataEmpty (int parm)
{
  int i = IsDataEmpty (PCB->Data);
  return parm ? !i : i;
}

/* FLAG(DataEmpty,FlagIsDataEmpty,0) */
/* FLAG(DataNonEmpty,FlagIsDataEmpty,1) */

/* ---------------------------------------------------------------------------
 * gets minimum and maximum coordinates
 * returns NULL if layout is empty
 */
BoxTypePtr
GetDataBoundingBox (DataTypePtr Data)
{
  static BoxType box;

  /* preset identifiers with highest and lowest possible values */
  box.X1 = box.Y1 = MAX_COORD;
  box.X2 = box.Y2 = -MAX_COORD;

  /* now scan for the lowest/highest X and Y coodinate */
  VIA_LOOP (Data);
  {
    box.X1 = MIN (box.X1, via->X - via->Thickness / 2);
    box.Y1 = MIN (box.Y1, via->Y - via->Thickness / 2);
    box.X2 = MAX (box.X2, via->X + via->Thickness / 2);
    box.Y2 = MAX (box.Y2, via->Y + via->Thickness / 2);
  }
  END_LOOP;
  ELEMENT_LOOP (Data);
  {
    box.X1 = MIN (box.X1, element->BoundingBox.X1);
    box.Y1 = MIN (box.Y1, element->BoundingBox.Y1);
    box.X2 = MAX (box.X2, element->BoundingBox.X2);
    box.Y2 = MAX (box.Y2, element->BoundingBox.Y2);
    {
      TextTypePtr text = &NAMEONPCB_TEXT (element);
      box.X1 = MIN (box.X1, text->BoundingBox.X1);
      box.Y1 = MIN (box.Y1, text->BoundingBox.Y1);
      box.X2 = MAX (box.X2, text->BoundingBox.X2);
      box.Y2 = MAX (box.Y2, text->BoundingBox.Y2);
    };
  }
  END_LOOP;
  ALLLINE_LOOP (Data);
  {
    box.X1 = MIN (box.X1, line->Point1.X - line->Thickness / 2);
    box.Y1 = MIN (box.Y1, line->Point1.Y - line->Thickness / 2);
    box.X1 = MIN (box.X1, line->Point2.X - line->Thickness / 2);
    box.Y1 = MIN (box.Y1, line->Point2.Y - line->Thickness / 2);
    box.X2 = MAX (box.X2, line->Point1.X + line->Thickness / 2);
    box.Y2 = MAX (box.Y2, line->Point1.Y + line->Thickness / 2);
    box.X2 = MAX (box.X2, line->Point2.X + line->Thickness / 2);
    box.Y2 = MAX (box.Y2, line->Point2.Y + line->Thickness / 2);
  }
  ENDALL_LOOP;
  ALLARC_LOOP (Data);
  {
    box.X1 = MIN (box.X1, arc->BoundingBox.X1);
    box.Y1 = MIN (box.Y1, arc->BoundingBox.Y1);
    box.X2 = MAX (box.X2, arc->BoundingBox.X2);
    box.Y2 = MAX (box.Y2, arc->BoundingBox.Y2);
  }
  ENDALL_LOOP;
  ALLTEXT_LOOP (Data);
  {
    box.X1 = MIN (box.X1, text->BoundingBox.X1);
    box.Y1 = MIN (box.Y1, text->BoundingBox.Y1);
    box.X2 = MAX (box.X2, text->BoundingBox.X2);
    box.Y2 = MAX (box.Y2, text->BoundingBox.Y2);
  }
  ENDALL_LOOP;
  ALLPOLYGON_LOOP (Data);
  {
    box.X1 = MIN (box.X1, polygon->BoundingBox.X1);
    box.Y1 = MIN (box.Y1, polygon->BoundingBox.Y1);
    box.X2 = MAX (box.X2, polygon->BoundingBox.X2);
    box.Y2 = MAX (box.Y2, polygon->BoundingBox.Y2);
  }
  ENDALL_LOOP;
  return (IsDataEmpty (Data) ? NULL : &box);
}

/* ---------------------------------------------------------------------------
 * centers the displayed PCB around the specified point (X,Y)
 * if Delta is false, X,Y are in absolute PCB coordinates
 * if Delta is true, simply move the center by an amount X, Y in screen
 * coordinates
 */
void
CenterDisplay (LocationType X, LocationType Y, Boolean Delta)
{
  int save_grid = PCB->Grid;
  PCB->Grid = 1;
  if (Delta)
    MoveCrosshairRelative(X, Y);
  else
    MoveCrosshairAbsolute(X, Y);
  PCB->Grid = save_grid;
}

/* ---------------------------------------------------------------------------
 * transforms symbol coordinates so that the left edge of each symbol
 * is at the zero position. The y coordinates are moved so that min(y) = 0
 * 
 */
void
SetFontInfo (FontTypePtr Ptr)
{
  Cardinal i, j;
  SymbolTypePtr symbol;
  LineTypePtr line;
  LocationType totalminy = MAX_COORD;

  /* calculate cell with and height (is at least DEFAULT_CELLSIZE)
   * maximum cell width and height
   * minimum x and y position of all lines
   */
  Ptr->MaxWidth = DEFAULT_CELLSIZE;
  Ptr->MaxHeight = DEFAULT_CELLSIZE;
  for (i = 0, symbol = Ptr->Symbol; i <= MAX_FONTPOSITION; i++, symbol++)
    {
      LocationType minx, miny, maxx, maxy;

      /* next one if the index isn't used or symbol is empty (SPACE) */
      if (!symbol->Valid || !symbol->LineN)
	continue;

      minx = miny = MAX_COORD;
      maxx = maxy = 0;
      for (line = symbol->Line, j = symbol->LineN; j; j--, line++)
	{
	  minx = MIN (minx, line->Point1.X);
	  miny = MIN (miny, line->Point1.Y);
	  minx = MIN (minx, line->Point2.X);
	  miny = MIN (miny, line->Point2.Y);
	  maxx = MAX (maxx, line->Point1.X);
	  maxy = MAX (maxy, line->Point1.Y);
	  maxx = MAX (maxx, line->Point2.X);
	  maxy = MAX (maxy, line->Point2.Y);
	}

      /* move symbol to left edge */
      for (line = symbol->Line, j = symbol->LineN; j; j--, line++)
	MOVE_LINE_LOWLEVEL (line, -minx, 0);

      /* set symbol bounding box with a minimum cell size of (1,1) */
      symbol->Width = maxx - minx + 1;
      symbol->Height = maxy + 1;

      /* check total min/max  */
      Ptr->MaxWidth = MAX (Ptr->MaxWidth, symbol->Width);
      Ptr->MaxHeight = MAX (Ptr->MaxHeight, symbol->Height);
      totalminy = MIN (totalminy, miny);
    }

  /* move coordinate system to the upper edge (lowest y on screen) */
  for (i = 0, symbol = Ptr->Symbol; i <= MAX_FONTPOSITION; i++, symbol++)
    if (symbol->Valid)
      {
	symbol->Height -= totalminy;
	for (line = symbol->Line, j = symbol->LineN; j; j--, line++)
	  MOVE_LINE_LOWLEVEL (line, 0, -totalminy);
      }

  /* setup the box for the default symbol */
  Ptr->DefaultSymbol.X1 = Ptr->DefaultSymbol.Y1 = 0;
  Ptr->DefaultSymbol.X2 = Ptr->DefaultSymbol.X1 + Ptr->MaxWidth;
  Ptr->DefaultSymbol.Y2 = Ptr->DefaultSymbol.Y1 + Ptr->MaxHeight;
}

static void
GetNum (char **s, BDimension * num)
{
  *num = atoi (*s);
  while (isdigit (**s))
    (*s)++;
}


char *
build_route_string(RouteStyleType *rs)
	{
#ifdef FIXME
	char	*str, *s, *t, *colon;
	int	i;

	str = MyStrdup("", __FUNCTION__);
	for (i = 0; i < NUM_STYLES; ++i, ++rs)
		{
		s = g_strdup_printf("%s,%d,%d,%d,%d", rs->Name,
				rs->Thick, rs->Diameter, rs->Hole, rs->Keepaway);
		colon = (i == NUM_STYLES - 1) ? NULL : ":";
		t = str;
		str = g_strconcat(str, s, colon, NULL);
		free(t);
		}
	return str;
#else
	return "Please_fix_build_route_string";
#endif
	}

/* ----------------------------------------------------------------------
 * parses the routes definition string which is a colon seperated list of
 * comma seperated Name, Dimension, Dimension, Dimension, Dimension
 * e.g. Signal,20,40,20,10:Power,40,60,28,10:...
 */
int
ParseRouteString (char *s, RouteStyleTypePtr routeStyle, int scale)
{
  int i, style;
  char Name[256];

  memset (routeStyle, 0, NUM_STYLES * sizeof (RouteStyleType));
  for (style = 0; style < NUM_STYLES; style++, routeStyle++)
    {
      while (*s && isspace (*s))
	s++;
      for (i = 0; *s && *s != ','; i++)
	Name[i] = *s++;
      Name[i] = '\0';
      routeStyle->Name = MyStrdup (Name, "ParseRouteString()");
      if (!isdigit (*++s))
	goto error;
      GetNum (&s, &routeStyle->Thick);
      routeStyle->Thick *= scale;
      while (*s && isspace (*s))
	s++;
      if (*s++ != ',')
	goto error;
      while (*s && isspace (*s))
	s++;
      if (!isdigit (*s))
	goto error;
      GetNum (&s, &routeStyle->Diameter);
      routeStyle->Diameter *= scale;
      while (*s && isspace (*s))
	s++;
      if (*s++ != ',')
	goto error;
      while (*s && isspace (*s))
	s++;
      if (!isdigit (*s))
	goto error;
      GetNum (&s, &routeStyle->Hole);
      routeStyle->Hole *= scale;
      /* for backwards-compatibilty, we use a 10-mil default
       * for styles which omit the keepaway specification. */
      if (*s != ',')
	routeStyle->Keepaway = 1000;
      else
	{
	  s++;
	  while (*s && isspace (*s))
	    s++;
	  if (!isdigit (*s))
	    goto error;
	  GetNum (&s, &routeStyle->Keepaway);
	  routeStyle->Keepaway *= scale;
	  while (*s && isspace (*s))
	    s++;
	}
      if (style < NUM_STYLES - 1)
	{
	  while (*s && isspace (*s))
	    s++;
	  if (*s++ != ':')
	    goto error;
	}
    }
  return (0);

 error:
  memset (routeStyle, 0, NUM_STYLES * sizeof (RouteStyleType));
  return (1);
}

/* ----------------------------------------------------------------------
 * parses the group definition string which is a colon seperated list of
 * comma seperated layer numbers (1,2,b:4,6,8,t)
 */
int
ParseGroupString (char *s, LayerGroupTypePtr LayerGroup)
{
  int group, member, layer;
  Boolean c_set = False,	/* flags for the two special layers to */
    s_set = False;		/* provide a default setting for old formats */

  /* clear struct */
  memset (LayerGroup, 0, sizeof (LayerGroupType));

  /* loop over all groups */
  for (group = 0; s && *s && group < MAX_LAYER; group++)
    {
      while (*s && isspace (*s))
	s++;

      /* loop over all group members */
      for (member = 0; *s; s++)
	{
	  /* ignore white spaces and get layernumber */
	  while (*s && isspace (*s))
	    s++;
	  switch (*s)
	    {
	    case 'c':
	    case 'C':
	      layer = MAX_LAYER + COMPONENT_LAYER;
	      c_set = True;
	      break;

	    case 's':
	    case 'S':
	      layer = MAX_LAYER + SOLDER_LAYER;
	      s_set = True;
	      break;

	    default:
	      if (!isdigit (*s))
		goto error;
	      layer = atoi (s) - 1;
	      break;
	    }
	  if (layer > MAX_LAYER + MAX (SOLDER_LAYER, COMPONENT_LAYER) ||
	      member >= MAX_LAYER + 1)
	    goto error;
	  LayerGroup->Entries[group][member++] = layer;
	  while (*++s && isdigit (*s));

	  /* ignore white spaces and check for seperator */
	  while (*s && isspace (*s))
	    s++;
	  if (!*s || *s == ':')
	    break;
	  if (*s != ',')
	    goto error;
	}
      LayerGroup->Number[group] = member;
      if (*s == ':')
	s++;
    }
  if (!s_set)
    LayerGroup->Entries[SOLDER_LAYER][LayerGroup->Number[SOLDER_LAYER]++] =
      MAX_LAYER + SOLDER_LAYER;
  if (!c_set)
    LayerGroup->
      Entries[COMPONENT_LAYER][LayerGroup->Number[COMPONENT_LAYER]++] =
      MAX_LAYER + COMPONENT_LAYER;
  return (0);

  /* reset structure on error */
error:
  memset (LayerGroup, 0, sizeof (LayerGroupType));
  return (1);
}

/* ---------------------------------------------------------------------------
 * quits application
 */
void
QuitApplication (void)
{
  /* save data if necessary */
  if (PCB->Changed && Settings.SaveInTMP)
    EmergencySave ();

  /*
   * if Settings.init_done is not > 0 then we haven't even called
   * gtk_main() yet so gtk_main_quit() will give an error.  In
   * this case just set the flat to -1 and we will exit instead
   * of calling gtk_main()
   */
  if( Settings.init_done > 0 )
    exit(0);
  else
    Settings.init_done = -1;
}

/* ---------------------------------------------------------------------------
 * creates a filename from a template
 * %f is replaced by the filename 
 * %p by the searchpath
 */
char *
EvaluateFilename (char *Template, char *Path, char *Filename, char *Parameter)
{
  static DynamicStringType command;
  char *p;

  if (Settings.verbose)
    {
      printf("EvaluateFilename:\n");
      printf("\tTemplate: \033[33m%s\033[0m\n", Template);
      printf("\tPath: \033[33m%s\033[0m\n", Path);
      printf("\tFilename: \033[33m%s\033[0m\n", Filename);
      printf("\tParameter: \033[33m%s\033[0m\n", Parameter);
    }

  DSClearString (&command);
	
  for (p = Template; p && *p; p++)
    {
      /* copy character or add string to command */
      if (*p == '%'
	  && (*(p + 1) == 'f' || *(p + 1) == 'p' || *(p + 1) == 'a'))
	switch (*(++p))
	  {
	  case 'a':
	    DSAddString (&command, Parameter);
	    break;
	  case 'f':
	    DSAddString (&command, Filename);
	    break;
	  case 'p':
	    DSAddString (&command, Path);
	    break;
	  }
      else
	DSAddCharacter (&command, *p);
    }
  DSAddCharacter (&command, '\0');
  if (Settings.verbose)
    printf("EvaluateFilename: \033[32m%s\033[0m\n", command.Data);

  return (MyStrdup (command.Data, "EvaluateFilename()"));
}

/* ---------------------------------------------------------------------------
 * concatenates directory and filename if directory != NULL,
 * expands them with a shell and returns the found name(s) or NULL
 */
char *
ExpandFilename (char *Dirname, char *Filename)
{
  static DynamicStringType answer;
  char *command;
  FILE *pipe;
  int c;

  /* allocate memory for commandline and build it */
  DSClearString (&answer);
  if (Dirname)
    {
      command = MyCalloc (strlen (Filename) + strlen (Dirname) + 7,
			  sizeof (char), "ExpandFilename()");
      sprintf (command, "echo %s/%s", Dirname, Filename);
    }
  else
    {
      command = MyCalloc (strlen (Filename) + 6, sizeof (char), "Expand()");
      sprintf (command, "echo %s", Filename);
    }

  /* execute it with shell */
  if ((pipe = popen (command, "r")) != NULL)
    {
      /* discard all but the first returned line */
      for (;;)
	{
	  if ((c = fgetc (pipe)) == EOF || c == '\n' || c == '\r')
	    break;
	  else
	    DSAddCharacter (&answer, c);
	}

      SaveFree (command);
      return (pclose (pipe) ? NULL : answer.Data);
    }

  /* couldn't be expanded by the shell */
  PopenErrorMessage (command);
  SaveFree (command);
  return (NULL);
}


/* ---------------------------------------------------------------------------
 * returns the layer number for the passed pointer
 */
int
GetLayerNumber (DataTypePtr Data, LayerTypePtr Layer)
{
  int i;

  for (i = 0; i < MAX_LAYER + 2; i++)
    if (Layer == &Data->Layer[i])
      break;
  return (i);
}

/* ---------------------------------------------------------------------------
 * move layer (number is passed in) to top of layerstack
 */
static void
PushOnTopOfLayerStack (int NewTop)
{
  int i;

  /* ignore COMPONENT_LAYER and SOLDER_LAYER */
  if (NewTop < MAX_LAYER)
    {
      /* first find position of passed one */
      for (i = 0; i < MAX_LAYER; i++)
	if (LayerStack[i] == NewTop)
	  break;

      /* bring this element to the top of the stack */
      for (; i; i--)
	LayerStack[i] = LayerStack[i - 1];
      LayerStack[0] = NewTop;
    }
}


/* ----------------------------------------------------------------------
 * changes the visibility of all layers in a group
 * returns the number of changed layers
 */
int
ChangeGroupVisibility (int Layer, Boolean On, Boolean ChangeStackOrder)
{
  int group, i, changed = 1;	/* at least the current layer changes */

  /* Warning: these special case values must agree with what gui-top-window.c
  |  thinks the are.
  */

	if (Settings.verbose)
		printf("ChangeGroupVisibility(Layer=%d, On=%d, ChangeStackOrder=%d)\n",
				Layer, On, ChangeStackOrder);

  /* special case of rat (netlist layer) */
#ifdef FIXME
  if (Layer == GUI_RATS_LAYER)
    {
      PCB->RatOn = On;
      PCB->RatDraw = On && ChangeStackOrder;
      if (PCB->RatDraw)
	SetMode (NO_MODE);
      gui_layer_buttons_update();
      return (0);
    }
  else
    PCB->RatDraw = (On && ChangeStackOrder) ? False : PCB->RatDraw;

  /* special case of silk layer */
  if (Layer == GUI_SILK_LAYER)
    {
      PCB->ElementOn = On;
      PCB->SilkActive = On && ChangeStackOrder;
      PCB->Data->SILKLAYER.On = On;
      PCB->Data->BACKSILKLAYER.On = (On && PCB->InvisibleObjectsOn);
      gui_layer_buttons_update();
      return (0);
    }
  PCB->SilkActive = (On && ChangeStackOrder) ? False : PCB->SilkActive;
#endif

  /* decrement 'i' to keep stack in order of layergroup */
  if ((group = GetGroupOfLayer (Layer)) < MAX_LAYER)
    for (i = PCB->LayerGroups.Number[group]; i;)
      {
	int layer = PCB->LayerGroups.Entries[group][--i];

	/* dont count the passed member of the group */
	if (layer != Layer && layer < MAX_LAYER)
	  {
	    PCB->Data->Layer[layer].On = On;

	    /* push layer on top of stack if switched on */
	    if (On && ChangeStackOrder)
	      PushOnTopOfLayerStack (layer);
	    changed++;
	  }
      }

  /* change at least the passed layer */
  PCB->Data->Layer[Layer].On = On;
  if (On && ChangeStackOrder)
    PushOnTopOfLayerStack (Layer);

  /* update control panel and exit */
  hid_action("LayersChanged");
  return (changed);
}

/* ----------------------------------------------------------------------
 * lookup the group to which a layer belongs to
 * returns MAX_LAYER if no group is found
 */
int
GetGroupOfLayer (int Layer)
{
  int group, i;

  if (Layer == MAX_LAYER)
    return (Layer);
  for (group = 0; group < MAX_LAYER; group++)
    for (i = 0; i < PCB->LayerGroups.Number[group]; i++)
      if (PCB->LayerGroups.Entries[group][i] == Layer)
	return (group);
  return (MAX_LAYER);
}


/* ---------------------------------------------------------------------------
 * returns the layergroup number for the passed pointer
 */
int
GetLayerGroupNumberByPointer (LayerTypePtr Layer)
{
  return (GetLayerGroupNumberByNumber (GetLayerNumber (PCB->Data, Layer)));
}

/* ---------------------------------------------------------------------------
 * returns the layergroup number for the passed layernumber
 */
int
GetLayerGroupNumberByNumber (Cardinal Layer)
{
  int group, entry;

  for (group = 0; group < MAX_LAYER; group++)
    for (entry = 0; entry < PCB->LayerGroups.Number[group]; entry++)
      if (PCB->LayerGroups.Entries[group][entry] == Layer)
	return (group);

  /* since every layer belongs to a group it is safe to return
   * the value without boundary checking
   */
  return (group);
}

/* ---------------------------------------------------------------------------
 * returns a pointer to an objects bounding box;
 * data is valid until the routine is called again
 */
BoxTypePtr
GetObjectBoundingBox (int Type, void *Ptr1, void *Ptr2, void *Ptr3)
{
  static BoxType box;

  switch (Type)
    {
    case VIA_TYPE:
      {
	PinTypePtr via = (PinTypePtr) Ptr1;

	box.X1 = via->X - via->Thickness / 2;
	box.Y1 = via->Y - via->Thickness / 2;
	box.X2 = via->X + via->Thickness / 2;
	box.Y2 = via->Y + via->Thickness / 2;
	break;
      }

    case LINE_TYPE:
      {
	LineTypePtr line = (LineTypePtr) Ptr2;

	box.X1 = MIN (line->Point1.X, line->Point2.X);
	box.Y1 = MIN (line->Point1.Y, line->Point2.Y);
	box.X2 = MAX (line->Point1.X, line->Point2.X);
	box.Y2 = MAX (line->Point1.Y, line->Point2.Y);
	box.X1 -= line->Thickness / 2;
	box.Y1 -= line->Thickness / 2;
	box.X2 += line->Thickness / 2;
	box.Y2 += line->Thickness / 2;
	break;
      }

    case ARC_TYPE:
      box = ((ArcTypePtr) Ptr2)->BoundingBox;
      break;

    case TEXT_TYPE:
    case ELEMENTNAME_TYPE:
      box = ((TextTypePtr) Ptr2)->BoundingBox;
      break;

    case POLYGON_TYPE:
      box = ((PolygonTypePtr) Ptr2)->BoundingBox;
      break;

    case ELEMENT_TYPE:
      box = ((ElementTypePtr) Ptr1)->BoundingBox;
      {
	TextTypePtr text = &NAMEONPCB_TEXT ((ElementTypePtr) Ptr1);
	if (text->BoundingBox.X1 < box.X1)
	  box.X1 = text->BoundingBox.X1;
	if (text->BoundingBox.Y1 < box.Y1)
	  box.Y1 = text->BoundingBox.Y1;
	if (text->BoundingBox.X2 > box.X2)
	  box.X2 = text->BoundingBox.X2;
	if (text->BoundingBox.Y2 > box.Y2)
	  box.Y2 = text->BoundingBox.Y2;
      }
      break;

    case PAD_TYPE:
      {
	PadTypePtr pad = (PadTypePtr) Ptr2;

	box.X1 = MIN (pad->Point1.X, pad->Point2.X);
	box.Y1 = MIN (pad->Point1.Y, pad->Point2.Y);
	box.X2 = MAX (pad->Point1.X, pad->Point2.X);
	box.Y2 = MAX (pad->Point1.Y, pad->Point2.Y);
	box.X1 -= pad->Thickness;
	box.Y1 -= pad->Thickness;
	box.Y2 += pad->Thickness;
	box.X2 += pad->Thickness;
	break;
      }

    case PIN_TYPE:
      {
	PinTypePtr pin = (PinTypePtr) Ptr2;

	box.X1 = pin->X - pin->Thickness / 2;
	box.Y1 = pin->Y - pin->Thickness / 2;
	box.X2 = pin->X + pin->Thickness / 2;
	box.Y2 = pin->Y + pin->Thickness / 2;
	break;
      }

    case LINEPOINT_TYPE:
      {
	PointTypePtr point = (PointTypePtr) Ptr3;

	box.X1 = box.X2 = point->X;
	box.Y1 = box.Y2 = point->Y;
	break;
      }

    case POLYGONPOINT_TYPE:
      {
	PointTypePtr point = (PointTypePtr) Ptr3;

	box.X1 = box.X2 = point->X;
	box.Y1 = box.Y2 = point->Y;
	break;
      }

    default:
      Message ("Request for bounding box of unsupported typ %d\n", Type);
      box.X1 = box.X2 = box.Y1 = box.Y2 = 0;
      break;
    }
  return (&box);
}

/* ---------------------------------------------------------------------------
 * computes the bounding box of an arc
 */
void
SetArcBoundingBox (ArcTypePtr Arc)
{
  register double ca1, ca2, sa1, sa2;
  register LocationType ang1, ang2;
  register LocationType width;

  /* first put angles into standard form */
  if (Arc->Delta > 360)
    Arc->Delta = 360;
  ang1 = Arc->StartAngle;
  ang2 = Arc->StartAngle + Arc->Delta;
  if (Arc->Delta < 0)
    {
      LocationType temp;
      temp = ang1;
      ang1 = ang2;
      ang2 = temp;
    }
  if (ang1 < 0)
    {
      ang1 += 360;
      ang2 += 360;
    }
  /* calculate sines, cosines */
  switch (ang1)
    {
    case 0:
      ca1 = 1.0;
      sa1 = 0;
      break;
    case 90:
      ca1 = 0;
      sa1 = 1.0;
      break;
    case 180:
      ca1 = -1.0;
      sa1 = 0;
      break;
    case 270:
      ca1 = 0;
      sa1 = -1.0;
      break;
    default:
      ca1 = M180 * (double) ang1;
      sa1 = sin (ca1);
      ca1 = cos (ca1);
    }
  switch (ang2)
    {
    case 0:
      ca2 = 1.0;
      sa2 = 0;
      break;
    case 90:
      ca2 = 0;
      sa2 = 1.0;
      break;
    case 180:
      ca2 = -1.0;
      sa2 = 0;
      break;
    case 270:
      ca2 = 0;
      sa2 = -1.0;
      break;
    default:
      ca2 = M180 * (double) ang2;
      sa2 = sin (ca2);
      ca2 = cos (ca2);
    }

  Arc->BoundingBox.X2 = Arc->X - Arc->Width *
    ((ang1 < 180 && ang2 > 180) ? -1 : MIN (ca1, ca2));

  Arc->BoundingBox.X1 = Arc->X - Arc->Width *
    ((ang1 < 360 && ang2 > 360) ? 1 : MAX (ca1, ca2));

  Arc->BoundingBox.Y2 = Arc->Y + Arc->Height *
    ((ang1 < 90 && ang2 > 90) ? 1 : MAX (sa1, sa2));

  Arc->BoundingBox.Y1 = Arc->Y + Arc->Height *
    ((ang1 < 270 && ang2 > 270) ? -1 : MIN (sa1, sa2));

  width = (Arc->Thickness + Arc->Clearance) / 2;
  Arc->BoundingBox.X1 -= width;
  Arc->BoundingBox.X2 += width;
  Arc->BoundingBox.Y1 -= width;
  Arc->BoundingBox.Y2 += width;
}

/* ---------------------------------------------------------------------------
 * resets the layerstack setting
 */
void
ResetStackAndVisibility (void)
{
  Cardinal i;

  for (i = 0; i < MAX_LAYER + 2; i++)
    {
      if (i < MAX_LAYER)
	LayerStack[i] = i;
      PCB->Data->Layer[i].On = True;
    }
  PCB->ElementOn = True;
  PCB->InvisibleObjectsOn = True;
  PCB->PinOn = True;
  PCB->ViaOn = True;
  PCB->RatOn = True;
}

/* ---------------------------------------------------------------------------
 * saves the layerstack setting
 */
void
SaveStackAndVisibility (void)
{
  Cardinal i;
  static Boolean run = False;

  if (run == False)
    {
      SavedStack.cnt = 0;
      run = True;
    }

  if (SavedStack.cnt != 0)
    {
      fprintf (stderr,
	       "SaveStackAndVisibility()  layerstack was already saved and not"
	       "yet restored.  cnt = %d\n", SavedStack.cnt);
    }

  for (i = 0; i < MAX_LAYER + 2; i++)
    {
      if (i < MAX_LAYER)
	SavedStack.LayerStack[i] = LayerStack[i];
      SavedStack.LayerOn[i] = PCB->Data->Layer[i].On;
    }
  SavedStack.ElementOn = PCB->ElementOn;
  SavedStack.InvisibleObjectsOn = PCB->InvisibleObjectsOn;
  SavedStack.PinOn = PCB->PinOn;
  SavedStack.ViaOn = PCB->ViaOn;
  SavedStack.RatOn = PCB->RatOn;
  SavedStack.cnt++;
}

/* ---------------------------------------------------------------------------
 * restores the layerstack setting
 */
void
RestoreStackAndVisibility (void)
{
  Cardinal i;

  if (SavedStack.cnt == 0)
    {
      fprintf (stderr, "RestoreStackAndVisibility()  layerstack has not"
	       " been saved.  cnt = %d\n", SavedStack.cnt);
      return;
    }
  else if (SavedStack.cnt != 1)
    {
      fprintf (stderr, "RestoreStackAndVisibility()  layerstack save count is"
	       " wrong.  cnt = %d\n", SavedStack.cnt);
    }

  for (i = 0; i < MAX_LAYER + 2; i++)
    {
      if (i < MAX_LAYER)
	LayerStack[i] = SavedStack.LayerStack[i];
      PCB->Data->Layer[i].On = SavedStack.LayerOn[i];
    }
  PCB->ElementOn = SavedStack.ElementOn;
  PCB->InvisibleObjectsOn = SavedStack.InvisibleObjectsOn;
  PCB->PinOn = SavedStack.PinOn;
  PCB->ViaOn = SavedStack.ViaOn;
  PCB->RatOn = SavedStack.RatOn;

  SavedStack.cnt--;
}

/* ----------------------------------------------------------------------
 * returns pointer to current working directory.  If 'path' is not
 * NULL, then the current working directory is copied to the array
 * pointed to by 'path'
 */
char *
GetWorkingDirectory (char *path)
{
#if defined(SYSV) || defined(linux) || defined(__NetBSD__)
  return getcwd (path, MAXPATHLEN);
#else
  /* seems that some BSD releases lack of a prototype for getwd() */
  return getwd (path);
#endif

}

/* ---------------------------------------------------------------------------
 * writes a string to the passed file pointer
 * some special characters are quoted
 */
void
CreateQuotedString (DynamicStringTypePtr DS, char *S)
{
  DSClearString (DS);
  DSAddCharacter (DS, '"');
  while (*S)
    {
      if (*S == '"' || *S == '\\')
	DSAddCharacter (DS, '\\');
      DSAddCharacter (DS, *S++);
    }
  DSAddCharacter (DS, '"');
}

/* ---------------------------------------------------------------------------
 * returns the current possible grid factor or zero
 */
int
GetGridFactor (void)
{
#ifdef FIXME
  static int factor[] = { 1, 2, 5, 10 };
  int i, delta;

  /* check to see if we have to draw every point,
   * every 2nd, 5th or 10th
   */
  for (i = 0; i < ENTRIES (factor); i++)
    {
      delta = PCB->Grid * factor[i];
      if (TO_SCREEN (delta) >= MIN_GRID_DISTANCE)
	{
	  if (Settings.GridFactor != factor[i])
	    {
	      Settings.GridFactor = factor[i];
	    }
	  return (factor[i]);
	}
    }
  Settings.GridFactor = 0;
#endif
  return (0);
}

static void
RightAngles (int Angle, float *cosa, float *sina)
{
  *cosa = (float) cos ((double) Angle * M180);
  *sina = (float) sin ((double) Angle * M180);
}

BoxTypePtr
GetArcEnds (ArcTypePtr Arc)
{
  static BoxType box;
  float ca, sa;

  RightAngles (Arc->StartAngle, &ca, &sa);
  box.X1 = Arc->X - Arc->Width * ca;
  box.Y1 = Arc->Y + Arc->Height * sa;
  RightAngles (Arc->StartAngle + Arc->Delta, &ca, &sa);
  box.X2 = Arc->X - Arc->Width * ca;
  box.Y2 = Arc->Y + Arc->Height * sa;
  return (&box);
}

void
ChangeArcAngles(LayerTypePtr Layer, ArcTypePtr a,
		long int new_sa, long int new_da)
{
  if (new_da >= 360)
    {
      new_da = 360;
      new_sa = 0;
    }
  r_delete_entry (Layer->arc_tree, (BoxTypePtr) a);
  AddObjectToChangeAnglesUndoList (ARC_TYPE, a, a, a);
  a->StartAngle = new_sa;
  a->Delta = new_da;
  SetArcBoundingBox(a);
  r_insert_entry (Layer->arc_tree, (BoxTypePtr) a, 0);
}

static char *
BumpName (char *Name)
{
  int num;
  char c, *start;
  static char temp[256];

  start = Name;
  /* seek end of string */
  while (*Name != 0)
    Name++;
  /* back up to potential number */
  for (Name--; isdigit (*Name); Name--);
  Name++;
  if (*Name)
    num = atoi (Name) + 1;
  else
    num = 1;
  c = *Name;
  *Name = 0;
  sprintf (temp, "%s%d", start, num);
  /* if this is not our string, put back the blown character */
  if (start != temp)
    *Name = c;
  return (temp);
}

/*
 * make a unique name for the name on board 
 * this can alter the contents of the input string
 */
char *
UniqueElementName (DataTypePtr Data, char *Name)
{
  Boolean unique = True;
  /* null strings are ok */
  if (!Name || !*Name)
    return (Name);

  for (;;)
    {
      ELEMENT_LOOP (Data);
      {
	if (NAMEONPCB_NAME (element) &&
	    NSTRCMP (NAMEONPCB_NAME (element), Name) == 0)
	  {
	    Name = BumpName (Name);
	    unique = False;
	    break;
	  }
      }
      END_LOOP;
      if (unique)
	return (Name);
      unique = True;
    }
}

static void
GetGridLockCoordinates (int type, void *ptr1,
			void *ptr2, void *ptr3, LocationType * x, LocationType * y)
{
  switch (type)
    {
    case VIA_TYPE:
      *x = ((PinTypePtr) ptr2)->X;
      *y = ((PinTypePtr) ptr2)->Y;
      break;
    case LINE_TYPE:
      *x = ((LineTypePtr) ptr2)->Point1.X;
      *y = ((LineTypePtr) ptr2)->Point1.Y;
      break;
    case TEXT_TYPE:
    case ELEMENTNAME_TYPE:
      *x = ((TextTypePtr) ptr2)->X;
      *y = ((TextTypePtr) ptr2)->Y;
      break;
    case ELEMENT_TYPE:
      *x = ((ElementTypePtr) ptr2)->MarkX;
      *y = ((ElementTypePtr) ptr2)->MarkY;
      break;
    case POLYGON_TYPE:
      *x = ((PolygonTypePtr) ptr2)->Points[0].X;
      *y = ((PolygonTypePtr) ptr2)->Points[0].Y;
      break;

    case LINEPOINT_TYPE:
    case POLYGONPOINT_TYPE:
      *x = ((PointTypePtr) ptr3)->X;
      *y = ((PointTypePtr) ptr3)->Y;
      break;
    case ARC_TYPE:
      {
	BoxTypePtr box;

	box = GetArcEnds ((ArcTypePtr) ptr2);
	*x = box->X1;
	*y = box->Y1;
	break;
      }
    }
}

void
AttachForCopy (LocationType PlaceX, LocationType PlaceY)
{
  BoxTypePtr box;
  LocationType mx, my;

  Crosshair.AttachedObject.RubberbandN = 0;
  if (TEST_FLAG (SNAPPINFLAG, PCB))
    {
      mx = 0;
      my = 0;
    }
  else
    {
      /* dither the grab point so that the mark, center, etc
       * will end up on a grid coordinate
       */
      GetGridLockCoordinates (Crosshair.AttachedObject.Type,
			      Crosshair.AttachedObject.Ptr1,
			      Crosshair.AttachedObject.Ptr2,
			      Crosshair.AttachedObject.Ptr3, &mx, &my);
      mx = GRIDFIT_X (mx, PCB->Grid) - mx;
      my = GRIDFIT_Y (my, PCB->Grid) - my;
    }
  Crosshair.AttachedObject.X = PlaceX - mx;
  Crosshair.AttachedObject.Y = PlaceY - my;
  if (!Marked.status || TEST_FLAG (LOCALREFFLAG, PCB))
    SetLocalRef (PlaceX - mx, PlaceY - my, True);
  Crosshair.AttachedObject.State = STATE_SECOND;

  /* get boundingbox of object and set cursor range */
  box = GetObjectBoundingBox (Crosshair.AttachedObject.Type,
			      Crosshair.AttachedObject.Ptr1,
			      Crosshair.AttachedObject.Ptr2,
			      Crosshair.AttachedObject.Ptr3);
  SetCrosshairRange (Crosshair.AttachedObject.X - box->X1,
		     Crosshair.AttachedObject.Y - box->Y1,
		     PCB->MaxWidth - (box->X2 - Crosshair.AttachedObject.X),
		     PCB->MaxHeight - (box->Y2 - Crosshair.AttachedObject.Y));

  /* get all attached objects if necessary */
  if ((Settings.Mode != COPY_MODE) && TEST_FLAG (RUBBERBANDFLAG, PCB))
    LookupRubberbandLines (Crosshair.AttachedObject.Type,
			   Crosshair.AttachedObject.Ptr1,
			   Crosshair.AttachedObject.Ptr2,
			   Crosshair.AttachedObject.Ptr3);
  if (Settings.Mode != COPY_MODE &&
      (Crosshair.AttachedObject.Type == ELEMENT_TYPE ||
       Crosshair.AttachedObject.Type == VIA_TYPE ||
       Crosshair.AttachedObject.Type == LINE_TYPE ||
       Crosshair.AttachedObject.Type == LINEPOINT_TYPE))
    LookupRatLines (Crosshair.AttachedObject.Type,
		    Crosshair.AttachedObject.Ptr1,
		    Crosshair.AttachedObject.Ptr2,
		    Crosshair.AttachedObject.Ptr3);
}

/*
 * Return nonzero if the given file exists and is readable.
 */
int
FileExists (const char *name)
{
  FILE *f;
  f = fopen (name, "r");
  if (f)
    {
      fclose (f);
      return 1;
    }
  return 0;
}

char *
Concat (const char *first, ...)
{
  char *rv;
  int len;
  va_list a;

  len = strlen (first);
  rv = (char *) malloc (len + 1);
  strcpy (rv, first);

  va_start (a, first);
  while (1)
    {
      const char *s = va_arg (a, const char *);
      if (!s)
	break;
      len += strlen (s);
      rv = (char *) realloc (rv, len + 1);
      strcat (rv, s);
    }
  va_end (a);
  return rv;
}

int
mem_any_set (unsigned char *ptr, int bytes)
{
  while (bytes--)
    if (*ptr++)
      return 1;
  return 0;
}

/* This just fills in a FlagType with current flags.  */
FlagType
MakeFlags (unsigned int flags)
{
  FlagType rv;
  memset(&rv, 0, sizeof(rv));
  rv.f = flags;
  return rv;
}

/* This converts old flag bits (from saved PCB files) to new format.  */
FlagType
OldFlags (unsigned int flags)
{
  FlagType rv;
  memset(&rv, 0, sizeof(rv));
  /* If we move flag bits around, this is where we map old bits to them.  */
  rv.f = flags & 0xffff;
  rv.t[0] = (flags >> 16) & 0xff;
  rv.p[0] = (flags >> 24) & 0xff;
  return rv;
}

FlagType
AddFlags (FlagType flag, unsigned int flags)
{
  flag.f |= flags;
  return flag;
}

FlagType
MaskFlags (FlagType flag, unsigned int flags)
{
  flag.f &= ~flags;
  return flag;
}

/***********************************************************************
 * Layer Group Functions
 */

int
MoveLayerToGroup (int layer, int group)
{
  int prev, i, j;

  if (layer < 0 || layer > MAX_LAYER+1)
    return -1;
  prev = GetLayerGroupNumberByNumber(layer);
  if ((layer == MAX_LAYER
       && group == GetLayerGroupNumberByNumber(MAX_LAYER+1))
      ||  (layer == MAX_LAYER+1
	   && group == GetLayerGroupNumberByNumber(MAX_LAYER))
      || (group < 0 || group >= MAX_LAYER)
      || (prev == group))
    return prev;

  /* Remove layer from prev group */
  printf("removing layer %d from group %d\n", layer, prev);
  for (j=i=0; i<PCB->LayerGroups.Number[prev]; i++)
    if (PCB->LayerGroups.Entries[prev][i] != layer)
      PCB->LayerGroups.Entries[prev][j++] =
	PCB->LayerGroups.Entries[prev][i];
  PCB->LayerGroups.Number[prev] --;

  /* Add layer to new group.  */
  printf("adding layer %d to group %d\n", layer, group);
  i = PCB->LayerGroups.Number[group] ++;
  PCB->LayerGroups.Entries[group][i] = layer;

  return group;
}

char *
LayerGroupsToString(LayerGroupTypePtr lg)
{
#if MAX_LAYER < 9998
  /* Allows for layer numbers 0..9999 */
  static char buf[(MAX_LAYER+2)*5+1];
#endif
  char *cp = buf;
  int group, entry;
  for (group = 0; group < MAX_LAYER; group++)
    if (PCB->LayerGroups.Number[group])
      {
	for (entry = 0; entry < PCB->LayerGroups.Number[group]; entry++)
	  {
	    switch (PCB->LayerGroups.Entries[group][entry])
	      {
	      case MAX_LAYER + COMPONENT_LAYER:
		*cp++ = 'c';
		break;

	      case MAX_LAYER + SOLDER_LAYER:
		*cp++ = 's';
		break;

	      default:
		sprintf (cp, "%d",
			 PCB->LayerGroups.Entries[group][entry] + 1);
		while (*++cp)
		  ;
		break;
	      }
	    if (entry != PCB->LayerGroups.Number[group] - 1)
	      *cp++ = ',';
	  }
	if (group != MAX_LAYER - 1)
	  *cp++ = ':';
      }
  *cp++ = 0;
  return buf;
}

char *
pcb_author (void)
{
  static struct passwd *pwentry;
  static char *fab_author = 0;

  if (!fab_author)
    {
      if (Settings.FabAuthor && Settings.FabAuthor[0])
	fab_author = Settings.FabAuthor;
      else
	{
	  int len;
	  unsigned char *comma, *gecos;

	  /* ID the user. */
	  pwentry = getpwuid (getuid ());
	  gecos = pwentry->pw_gecos;
	  comma = strchr (gecos,',');
	  if (comma)
	    len = comma-gecos;
	  else
	    len = strlen (gecos);
	  fab_author = malloc (len+1);
	  if (!fab_author)
	    {
	      perror ("pcb: out of memory.\n");
	      exit (-1);
	    }
	  memcpy (fab_author, gecos, len);
	  fab_author[len] = 0;
	}
    }
  return fab_author;
}
