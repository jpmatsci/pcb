# -*- c -*-
# Note - gpcb-menu.res is used to build gpcb-menu.h
# Note - parameters are sensitive to extra spaces around the commas

Mouse =
{
  Left = {
    Mode(Notify)
    ctrl = { Mode(Save) Mode(None) Mode(Restore) Mode(Notify) }
    shift-ctrl = { Mode(Save) Mode(Remove) Mode(Notify) Mode(Restore) }
    up = Mode(Release)
  }
  Right = {
    Pan(1)
    up = Pan(0)
    shift = Popup(Popup1)
    ctrl = Display(CycleCrosshair)
  }
  Middle = {
    Mode(Stroke)
    up = Mode(Release)
    ctrl = { Mode(Save) Mode(Copy) Mode(Notify) }
    up-ctrl = { Mode(Notify) Mode(Restore) }
    shift-ctrl = { Display(ToggleRubberbandMode) Mode(Save) Mode(Move) Mode(Notify) }
    up-shift-ctrl = { Mode(Notify) Mode(Restore) Display(ToggleRubberbandMode) }
  }
  Up = {
    Zoom(0.8)
    shift = Scroll(up)
    ctrl = Scroll(left)
  }
  Down = {
    Zoom(1.25)
    shift = Scroll(down)
    ctrl = Scroll(right)
  }
# If you want zoom to center, do this instead.
  #Up = { {Zoom(0.8) Center()} }
  #Down = { {Zoom(1.25) Center()} }
}

MainMenu =
{

#
# File Menu
#

  {File
   {"Save layout" Save(Layout) tip="Saves current layout"}
   {"Save layout as..." Save(LayoutAs) tip="Saves current layout into a new file"}
   -
   {"Revert" Load(Revert,none) tip="Revert to the layout stored on disk"}
   -
   {"Import Schematics" Import() }
   {"Load layout" Load(Layout) tip="Load a layout from a file"}
   {"Load element data to paste-buffer" PasteBuffer(Clear) Load(ElementTobuffer)}
   {"Load layout data to paste-buffer" PasteBuffer(Clear) Load(LayoutTobuffer)}
   {"Load netlist file" Load(Netlist)}
   {"Load vendor resource file" LoadVendorFrom()}
   -
   {"Save connection data of"
     {" a single element" GetXY(Click to set the element mark <>) Save(ElementConnections)}
     {" all elements" Save(AllConnections)}
     {" unused pins" Save(AllUnusedPins)}
   }
   -
   {"Print layout..." Print()}
   {"Export layout..." Export()}
   {"Calibrate Printer..." PrintCalibrate()}
   -
   {"Start new layout" New()}
   -
   {"Preferences..." DoWindows(Preferences)}
   -
   {"Quit Program" Quit()  a={"Ctrl-Q" "Ctrl<Key>q"}}
  }

#
# Edit Menu
#

  {Edit 
   {"Undo last operation" Undo() a={"U" "<Key>u"}}
   {"Redo last undone operation" Redo() a={"Shift-R" "Shift<Key>r"}}
   {"Clear undo-buffer" Undo(ClearList) a={"Shift-Ctrl-U" "Shift Ctrl<Key>u"}}
   -
   {"Cut selection to buffer" GetXY(Click to set the snap point for this buffer)
    PasteBuffer(Clear) PasteBuffer(AddSelected) RemoveSelected() Mode(PasteBuffer)
    a={"Ctrl-X" "Ctrl<Key>x"}}
   {"Copy selection to buffer" GetXY(Click to set the snap point for this buffer)
    PasteBuffer(Clear) PasteBuffer(AddSelected) Unselect(All) Mode(PasteBuffer)
    a={"Ctrl-C" "Ctrl<Key>c"}}
   {"Paste buffer to layout" Mode(PasteBuffer) a={"Ctrl-V" "Ctrl<Key>v"}}
   -
   {"Unselect all" Unselect(All) a={"Shift-Alt-A" "Shift Alt<Key>a"}}
   {"Select all visible" Select(All) a={"Alt-A" "Alt<Key>a"}}
   -
   {"Edit name of"
     {"text on layout" ChangeName(Object) a={"N" "<Key>n"}}
     {"layout" ChangeName(Layout)}
     {"active layer" ChangeName(Layer)}
   }
   {"Edit attributes of"
    {"Layout" Attributes(Layout)}
    {"CurrentLayer" Attributes(Layer)}
    {"Element" Attributes(Element)}
   }
   -
   {"Move to current layer" MoveToCurrentLayer(Object) a={"M" "<Key>m"}}
   {"Move selected to current layer" MoveToCurrentLayer(Selected) a={"Shift-M" "Shift<Key>m"}}
   {"Route Styles" @routestyles
    -
    {"Edit..." AdjustStyle(0)}
   }
  }

#
# View Menu
#

  {View 
   {"Enable visible grid" checked=drawgrid Display(Grid)}
   {"Grid units"
    {"mil" checked=grid_units_mm,0 SetUnits(mil)}
    {"mm"  checked=grid_units_mm,1 SetUnits(mm)}
   }
   {"Grid size"
    {"No Grid" checked=gridsize,1 SetValue(Grid,1)}
    -
    {  "0.1 mil" checked=gridsize,10 SetUnits(mil) SetValue(Grid,10)}
    {  "1 mil"   checked=gridsize,100 SetUnits(mil) SetValue(Grid,100)}
    {  "5 mil"   checked=gridsize,500 SetUnits(mil) SetValue(Grid,500)}
    { "10 mil"   checked=gridsize,1000 SetUnits(mil) SetValue(Grid,1000)}
    { "25 mil"   checked=gridsize,2500 SetUnits(mil) SetValue(Grid,2500)}
    { "50 mil"   checked=gridsize,5000 SetUnits(mil) SetValue(Grid,5000)}
    {"100 mil"   checked=gridsize,10000 SetUnits(mil) SetValue(Grid,10000)}
    -
    {"0.01 mm" checked=gridsize,39 SetUnits(mm) SetValue(Grid,39.37007874)}
    {"0.05 mm" checked=gridsize,197 SetUnits(mm) SetValue(Grid,196.85039370)}
    {"0.1 mm"  checked=gridsize,394 SetUnits(mm) SetValue(Grid,393.70078740)}
    {"0.25 mm" checked=gridsize,984 SetUnits(mm) SetValue(Grid,984.25197)}
    {"0.5 mm"  checked=gridsize,1969 SetUnits(mm) SetValue(Grid,1968.503937)}
    {"1 mm"    checked=gridsize,3937 SetUnits(mm) SetValue(Grid,3937.00787400)}
    -
    {"Grid -5mil" SetValue(Grid,-5,mil) a={"Shift-G" "Shift<Key>g"}}
    {"Grid +5mil" SetValue(Grid,+5,mil) a={"G" "<Key>g"}}
    {"Grid -0.05mm" SetValue(Grid,-0.05,mm) a={"Shift-Ctrl-G" "Shift Ctrl<Key>g"}}
    {"Grid +0.05mm" SetValue(Grid,+0.05,mm) a={"Ctrl-G" "Ctrl<Key>g"}}
   }
   {"Realign grid" GetXY(Click to set the grid origin) Display(ToggleGrid)}
   -
   {"Displayed element name"
    {"Description" Display(Description) checked=elementname,1}
    {"Reference Designator" Display(NameOnPCB) checked=elementname,2}
    {"Value" Display(Value) checked=elementname,3}
   }
   {"Enable Pinout shows number" checked=shownumber Display(ToggleName)}
   {"Pins/Via show Name/Number" Display(PinOrPadName) a={"D" "<Key>d"}}
   -
   {"Zoom In 20%" Zoom(-1.2) m=Z a={"Z" "<Key>z"}}
   {"Zoom Out 20%" Zoom(+1.2) m=O a={"Shift-Z" "Shift<Key>z"}}
   {"More zooms and view changes"
    {"Zoom Max" Zoom() m=M a={"V" "<Key>v"}}
    {"Zoom In 2X" Zoom(-2)}
    {"Zoom Out 2X" Zoom(+2)}
    {"Zoom to 0.1mil/px" Zoom(=10)}
    {"Zoom to 0.01mm/px" Zoom(=39.37)}
    {"Zoom to 1mil/px" Zoom(=100)}
    {"Zoom to 0.05mm/px" Zoom(=196.8504)}
    {"Zoom to 2.5mil/px" Zoom(=250)}
    {"Zoom to 0.1mm/px" Zoom(=393.7)}
    {"Zoom to 10mil/px" Zoom(=1000)}
    {"Zoom In 20% and center" Zoom(-1.2) Center() m=Z }
    {"Zoom Out 20% and center" Zoom(+1.2) Center() m=O }
    {"Flip up/down" checked=flip_y SwapSides(V) a={"Tab" "<Key>Tab"}}
    {"Flip left/right" checked=flip_x SwapSides(H) a={"Shift-Tab" "Shift<Key>Tab"}}
    {"Spin 180 degrees" SwapSides(R) a={"Ctrl-Tab" "Ctrl<Key>Tab"}}
    {"Swap Sides" SwapSides() a={"Ctrl-Shift-Tab" "Ctrl Shift<Key>Tab"}}
    {"Center cursor" Center() a={"C" "<Key>c"}}
   }
   -
   {"Shown Layers"
    @layerview
    -
    {"Edit Layer Groups" EditLayerGroups()}
   }
   {"Current Layer"
    @layerpick
    -
    {"Delete current layer" MoveLayer(c,-1)}
    {"Add new layer" MoveLayer(-1,c)}
    {"Move current layer up" MoveLayer(c,up)}
    {"Move current layer down" MoveLayer(c,down)}
   }
  }

#
# Settings menu
#  
  {Settings
   {"'All-direction' lines" checked=alldirection Display(Toggle45Degree) a={"." "<Key>."}}
   {"Auto swap line start angle" checked=swapstartdir Display(ToggleStartDirection)}
   {"Orthogonal moves" checked=orthomove Display(ToggleOrthoMove)}
   {"Crosshair snaps to pins and pads" checked=snappin Display(ToggleSnapPin)}
   {"Crosshair shows DRC clearance" checked=showdrc Display(ToggleShowDRC)}
   {"Auto enforce DRC clearance" checked=autodrc Display(ToggleAutoDRC)}
   {"Lock Names" checked=locknames Display(ToggleLockNames)}
   {"Only Names" checked=onlynames Display(ToggleOnlyNames)}
   {"Hide Names" checked=hidenames Display(ToggleHideNames)}
   -
   {"Rubber band mode" checked=rubberband Display(ToggleRubberBandMode)}
   {"Require unique element names" checked=uniquename Display(ToggleUniqueNames)}
   {"Auto-zero delta measurements" checked=localref Display(ToggleLocalRef)}
   {"New lines, arcs clear polygons" checked=clearnew Display(ToggleClearLine)}
   {"New polygons are full ones" checked=newfullpoly Display(ToggleFullPoly)}
   {"Show autorouter trials" checked=liveroute Display(ToggleLiveRoute)}
   {"Thin draw" checked=thindraw Display(ToggleThindraw) a={"|" "<Key>|"}}
   {"Thin draw poly" checked=thindrawpoly Display(ToggleThindrawPoly) a={"Ctrl-Shift-P" "Ctrl Shift<Key>p"}}
   {"Check polygons" checked=checkplanes Display(ToggleCheckPlanes)}
   -
   {"Vendor drill mapping" ToggleVendor() checked=VendorMapOn}
  }
    
#
# Select menu
#
  {Select 
   {"Select all visible objects" Select(All)}
   {"Select all connected objects" Select(Connection)}
   -
   {"Unselect all objects" Unselect(All)}
   {"unselect all connected objects" Unselect(Connection)}
   -
   {"Select by name"
    {"All objects" Select(ObjectByName) active=have_regex}
    {"Elements" Select(ElementByName) active=have_regex}
    {"Pads" Select(PadByName) active=have_regex}
    {"Pins" Select(PinByName) active=have_regex}
    {"Text" Select(TextByName) active=have_regex}
    {"Vias" Select(ViaByName) active=have_regex}
   }
   -
   {"Auto-place selected elements" AutoPlaceSelected() a={"Ctrl-P" "Ctrl<Key>p"}}
   {"Disperse all elements" DisperseElements(All)}
   {"Disperse selected elements" DisperseElements(Selected)}
   -
   {"Move selected elements to other side" Flip(SelectedElements) a={"Shift-B" "Shift<Key>b"}}
   {"Remove selected objects" RemoveSelected() a={"Shift-Delete" "Shift<Key>Delete"}}
   {"Convert selection to element" Select(Convert)}
   -
   {"Optimize selected rats" DeleteRats(SelectedRats) AddRats(SelectedRats)}
   {"Auto-route selected rats" AutoRoute(SelectedRats) a={"Alt-R" "Alt<Key>r"}}
   {"Rip-up selected auto-routed tracks" RipUp(Selected)}
   -
   {"Change size of selected objects"
    {"Lines -10 mil" ChangeSize(SelectedLines,-10,mil) ChangeSize(SelectedArcs,-10,mil)}
    {"Lines +10 mil" ChangeSize(SelectedLines,+10,mil) ChangeSize(SelectedArcs,+10,mil)}
    {"Pads -10 mil" ChangeSize(SelectedPads,-10,mil)}
    {"Pads +10 mil" ChangeSize(SelectedPads,+10,mil)}
    {"Pins -10 mil" ChangeSize(SelectedPins,-10,mil)}
    {"Pins +10 mil" ChangeSize(SelectedPins,+10,mil)}
    {"Texts -10 mil" ChangeSize(SelectedTexts,-10,mil)}
    {"Texts +10 mil" ChangeSize(SelectedTexts,+10,mil)}
    {"Vias -10 mil" ChangeSize(SelectedVias,-10,mil)}
    {"Vias +10 mil" ChangeSize(SelectedVias,+10,mil)}
   }
   -
   {"Change drilling hole of selected objects"
    {"Vias -10 mil" ChangeDrillSize(SelectedVias,-10,mil)}
    {"Vias +10 mil" ChangeDrillSize(SelectedVias,+10,mil)}
    {"Pins -10 mil" ChangeDrillSize(SelectedPins,-10,mil)}
    {"Pins +10 mil" ChangeDrillSize(SelectedPins,+10,mil)}
   }
   -
   {"Change square-flag of selected objects"
    {"Elements" ChangeSquare(SelectedElements)}
    {"Pins" ChangeSquare(SelectedPins)}
   }
  }

#
# Buffer menu
#
  {Buffer
   {"Cut selection to buffer" GetXY(Click to set the snap point for this buffer)
    PasteBuffer(Clear) PasteBuffer(AddSelected) RemoveSelected() Mode(PasteBuffer)}
   {"Paste buffer to layout" Mode(PasteBuffer)}
   -
   {"Rotate buffer 90 deg CCW" Mode(PasteBuffer) PasteBuffer(Rotate,1)
    a={"Shift-F7" "Shift<Key>F7"}}
   {"Rotate buffer 90 deg CW" Mode(PasteBuffer) PasteBuffer(Rotate,3)}
   {"Mirror buffer (up/down)" Mode(PasteBuffer) PasteBuffer(Mirror)}
   {"Mirror buffer (left/right)" Mode(PasteBuffer) PasteBuffer(Rotate,1)
    PasteBuffer(Mirror) PasteBuffer(Rotate,3)}
   -
   {"Clear buffer" PasteBuffer(Clear)}
   {"Convert buffer to element" PasteBuffer(Convert)}
   {"Break buffer elements to pieces" PasteBuffer(Restore)}
   {"Save buffer elements to file" Save(PasteBuffer)}
   -
   {"Select current buffer" foreground=grey50 sensitive=false}
   {"#1" checked=buffer,1 PasteBuffer(1) a={"Shift-1" "Shift<Key>1"}}
   {"#2" checked=buffer,2 PasteBuffer(2) a={"Shift-2" "Shift<Key>2"}}
   {"#3" checked=buffer,3 PasteBuffer(3) a={"Shift-3" "Shift<Key>3"}}
   {"#4" checked=buffer,4 PasteBuffer(4) a={"Shift-4" "Shift<Key>4"}}
   {"#5" checked=buffer,5 PasteBuffer(5) a={"Shift-5" "Shift<Key>5"}}
  }
    
#
# Connects menu
#
  {Connects 
   {"Lookup connection to object" GetXY(Click on the object) Connection(Find) a={"Ctrl-F" "Ctrl<Key>f"}}
   {"Reset scanned pads/pins/vias" Connection(ResetPinsViasAndPads) Display(Redraw)}
   {"Reset scanned lines/polygons" Connection(ResetLinesAndPolygons) Display(Redraw)}
   {"Reset all connections" Connection(Reset) Display(Redraw) a={"Shift-F" "Shift<Key>f"}}
   -
   {"Optimize rats nest" Atomic(Save) DeleteRats(AllRats)
    Atomic(Restore) AddRats(AllRats) Atomic(Block) a={"O" "<Key>o"}}
   {"Erase rats nest" DeleteRats(AllRats) a={"E" "<Key>e"}}
   {"Erase selected rats" DeleteRats(SelectedRats) a={"Shift-E" "Shift<Key>e"}}
   -
   {"Auto-route selected rats" AutoRoute(Selected)}
   {"Auto-route all rats" AutoRoute(AllRats)}
   {"Rip up all auto-routed tracks" RipUp(All)}
   -
   {"Optimize routed tracks"
    {"Auto-Optimize" djopt(auto)  a={"Shift-=" "Shift<Key>="}}
    {"Debumpify" djopt(debumpify) }
    {"Unjaggy" djopt(unjaggy) }
    {"Vianudge" djopt(vianudge) }
    {"Viatrim" djopt(viatrim) }
    {"Ortho pull" djopt(orthopull) }
    {"Simple optimization" djopt(simple)  a={"=" "<Key>="}}
    {"Miter" djopt(miter) }
    {"Puller" a={"Y" "<Key>y"} Puller() }
    {"Global Puller"
     {"Selected" GlobalPuller(selected) }
     {"Found" GlobalPuller(found) }
     {"All" GlobalPuller() }
    }
    -
    {"Only autorouted nets" OptAutoOnly() checked=optautoonly}
   }
   -
   {"Design Rule Checker" DRC()}
   -
   {"Apply vendor drill mapping" ApplyVendor()}
  }
    
#
# Info Menu
#
  {Info
   {"Generate object report" ReportObject() a={"Ctrl-R" "Ctrl<Key>r"}}
   {"Generate drill summary" Report(DrillReport)}
   {"Report found pins/pads" Report(FoundPins)}
   {"Key Bindings"
    {"Remove" a={"Delete" "<Key>Delete"}
     Mode(Save)
     Mode(Remove)
     Mode(Notify)
     Mode(Restore)
    }
    {"Remove Selected" a={"Backspace" "<Key>BackSpace"}
     RemoveSelected()
    }
    {"Remove Connected" a={"Shift-Backspace" "Shift<Key>BackSpace"}
     Atomic(Save)
     Connection(Reset)
     Atomic(Restore)
     Unselect(All)
     Atomic(Restore)
     Connection(Find)
     Atomic(Restore)
     Select(Connection)
     Atomic(Restore)
     RemoveSelected()
     Atomic(Restore)
     Connection(Reset)
     Atomic(Restore)
     Unselect(All)
     Atomic(Block)
    }
    {"Remove Connected"
     Atomic(Save)
     Connection(Reset)
     Atomic(Restore)
     Unselect(All)
     Atomic(Restore)
     Connection(Find)
     Atomic(Restore)
     Select(Connection)
     Atomic(Restore)
     RemoveSelected()
     Atomic(Restore)
     Connection(Reset)
     Atomic(Restore)
     Unselect(All)
     Atomic(Block)
    }
    {"Set Same" a={"A" "<Key>a"} SetSame()}
    {"Flip Object" a={"B" "<Key>b"} Flip(Object)}
    {"Find Connections" a={"F" "<Key>f"} Connection(Reset) Connection(Find)}
    {"ToggleHideName Object" a={"H" "<Key>h"} ToggleHideName(Object)}
    {"ToggleHideName SelectedElement" a={"Shift-H" "Shift<Key>h"} ToggleHideName(SelectedElements)}
    {"ChangeHole Object" a={"Ctrl-H" "Ctrl<Key>h"} ChangeHole(Object)}
    {"ChangeJoin Object" a={"J" "<Key>j"} ChangeJoin(Object)}
    {"ChangeJoin SelectedObject" a={"Shift-J" "Shift<Key>j"} ChangeJoin(SelectedObjects)}
    {"Clear Object +2 mil" a={"K" "<Key>k"} ChangeClearSize(Object,+2,mil)}
    {"Clear Object -2 mil" a={"Shift-K" "Shift<Key>k"} ChangeClearSize(Object,-2,mil)}
    {"Clear Selected +2 mil" a={"Ctrl-K" "Ctrl<Key>k"} ChangeClearSize(SelectedObjects,+2,mil)}
    {"Clear Selected -2 mil" a={"Shift-Ctrl-K" "Shift Ctrl<Key>k"} ChangeClearSize(SelectedObjects,-2,mil)}
    {"Linesize +5 mil" a={"L" "<Key>l"} SetValue(LineSize,+5,mil)}
    {"Linesize -5 mil" a={"Shift-L" "Shift<Key>l"} SetValue(LineSize,-5,mil)}
    {"MarkCrosshair" a={"Ctrl-M" "Ctrl<Key>m"} MarkCrosshair()}
    {"Select shortest rat" a={"Shift-N" "Shift<Key>n"} AddRats(Close)}
    {"AddRats to selected pins" a={"Shift-O" "Shift<Key>o"}
     Atomic(Save)
     DeleteRats(AllRats)
     Atomic(Restore)
     AddRats(SelectedRats)
     Atomic(Block) }
    {"ChangeOctagon Object" a={"Ctrl-O" "Ctrl<Key>o"} ChangeOctagon(Object)}
    {"Polygon PreviousPoint" a={"P" "<Key>p"} Polygon(PreviousPoint)}
    {"Polygon Close" a={"Shift-P" "Shift<Key>p"} Polygon(Close)}
    {"ChangeSquare Object" a={"Q" "<Key>q"} ChangeSquare(ToggleObject)}
    {"ChangeSize +5 mil" a={"S" "<Key>s"} ChangeSize(Object,+5,mil)}
    {"ChangeSize -5 mil" a={"Shift-S" "Shift<Key>s"} ChangeSize(Object,-5,mil)}
    {"ChangeDrill +5 mil" a={"Alt-S" "Alt<Key>s"} ChangeDrillSize(Object,+5,mil)}
    {"ChangeDrill -5 mil" a={"Alt-Shift-S" "Alt Shift<Key>s"} ChangeDrillSize(Object,-5,mil)}
    {"TextScale +10 mil" a={"T" "<Key>t"} SetValue(TextScale,+10,mil)}
    {"TextScale -10 mil" a={"Shift-T" "Shift<Key>t"} SetValue(TextScale,-10,mil)}
    {"ViaSize +5 mil" a={"Shift-V" "Shift<Key>v"} SetValue(ViaSize,+5,mil)}
    {"ViaSize -5 mil" a={"Shift-Ctrl-V" "Shift Ctrl<Key>v"} SetValue(ViaSize,-5,mil)}
    {"ViaDrill +5 mil" a={"Alt-V" "Alt<Key>v"} SetValue(ViaDrillingHole,+5,mil)}
    {"ViaDrill -5 mil" a={"Alt-Shift-V" "Alt Shift<Key>v"} SetValue(ViaDrillingHole,-5,mil)}
    {"AddRats Selected" a={"Shift-W" "Shift<Key>w"} AddRats(SelectedRats)}
    {"Add All Rats" a={"W" "<Key>w"} AddRats(AllRats)}
    {"Cycle Clip" a={"/" "<Key>/"} Display(CycleClip)}
    {"Arrow Mode" a={"Space" "<Key>space"} Mode(Arrow) checked=arrowmode,1}
    {"Temp Arrow ON" a={"[" "<Key>["} Mode(Save) Mode(Arrow) Mode(Notify)}
    {"Temp Arrow OFF" a={"]" "<Key>]"} Mode(Release) Mode(Restore)}
    -
    {"Step Up" a={"Up" "<Key>Up"} Cursor(Warp,0,1,grid)}
    {"Step Down" a={"Down" "<Key>Down"} Cursor(Warp,0,-1,grid)}
    {"Step Left" a={"Left" "<Key>Left"} Cursor(Warp,-1,0,grid)}
    {"Step Right" a={"Right" "<Key>Right"} Cursor(Warp,1,0,grid)}
    {"Step +Up" a={"Up" "Shift<Key>Up"} Cursor(Pan,0,50,view)}
    {"Step +Down" a={"Down" "Shift<Key>Down"} Cursor(Pan,0,-50,view)}
    {"Step +Left" a={"Left" "Shift<Key>Left"} Cursor(Pan,-50,0,view)}
    {"Step +Right" a={"Right" "Shift<Key>Right"} Cursor(Pan,50,0,view)}
    {'"Click"' a={"Enter" "<Key>Enter"} Mode(Notify) Mode(Release)}
    -
   }
  }
#
# Window Menu
#
  {Window
   {"Library" DoWindows(Library) a={"i" "<Key>i"}}
   {"Message Log" DoWindows(Log)}
   {"DRC Check" DoWindows(DRC)}
   {"Netlist" DoWindows(Netlist)}
   {"Command Entry" Command() a={":" "<Key>:"}}
   {"Pinout" Display(Pinout) a={"Shift-D" "Shift<Key>d"}}
   -
   {"About..." About()}
  }
}

PopupMenus =
  {
    Popup1 =
    {
      {"Operations on selections"
       {"Unselect all objects" Unselect(All)}
       {"Remove selected objects" RemoveSelected()}
       {"Copy selection to buffer" 
	GetXY(Click to set the snap point for this buffer) 
	PasteBuffer(Clear)
	PasteBuffer(AddSelected)
	Mode(PasteBuffer)
       }
       {"Cut selection to buffer"
	GetXY(Click to set the snap point for this buffer) 
	PasteBuffer(Clear)
	PasteBuffer(AddSelected)
	RemoveSelected()
	Mode(PasteBuffer)
       }
       {"Convert selection to element" Select(Convert)}
       {"Auto place selected elements" AutoPlaceSelected()}
       {"Autoroute selected elements" AutoRoute(SelectedRats)}
       {"Rip up selected auto routed tracks" RipUp(Selected)}
      }
      {"Operations on this location"
       {"Generate object report" GetXY(Click on the object) Report(Object)}
      }
      -
      {"Undo last operation" Undo()}
      {"Redo last undone operation" Redo()}
      -
      {Tools
       {"None" checked=nomode,1 Mode(None)}
       {"Via" checked=viamode,1 Mode(Via) a={"F1" "<Key>F1"}}
       {"Line" checked=linemode,1 Mode(Line) a={"F2" "<Key>F2"}}
       {"Arc" checked=arcmode,1 Mode(Arc) a={"F3" "<Key>F3"}}
       {"Text" checked=textmode,1 Mode(Text) a={"F4" "<Key>F4"}}
       {"Rectangle" checked=rectanglemode,1 Mode(Rectangle) a={"F5" "<Key>F5"}}
       {"Polygon" checked=polygonmode,1 Mode(Polygon) a={"F6" "<Key>F6"}}
       {"Buffer" checked=pastebuffermode,1 Mode(PasteBuffer) a={"F7" "<Key>F7"}}
       {"Remove" checked=removemode,1 Mode(Remove) a={"F8" "<Key>F8"}}
       {"Rotate" checked=rotatemode,1 Mode(Rotate) a={"F9" "<Key>F9"}}
       {"Thermal" checked=thermalmode,1 Mode(Thermal) a={"F10" "<Key>F10"}}
       {"Arrow" checked=arrowmode,1 Mode(Arrow)  a={"F11" "<Key>F11"}}
       {"Insert Point" checked=insertpointmode,1 Mode(InsertPoint) a={"Insert" "<Key>Insert"}}
       {"Move" checked=movemode,1 Mode(Move)}
       {"Copy" checked=copymode,1 Mode(Copy)}
       {"Lock" checked=lockmode,1 Mode(Lock)}
       {"Cancel" Mode(Escape) a={"Esc" "<Key>Escape"}}
      }
    }
  }

