//
//  enclosure.scad
//  SensorClock
//
//  created by yu2924 on 2026-05-04
//

include <BOSL/constants.scad>
use <BOSL/shapes.scad>

$fn=30;

// terminology
// x: width
// y: height
// z: depth

Epsilon         = 0.01;

ShellThickness  = 1.5;
BoardWidth      = 164;
BoardHeight     = 97;
PanelWidth      = 164.5;
PanelHeight     = 99;
PanelDepth      = 9;
MountPostDepth  = 6;
PartsDepth      = 10.5;
PanelMargin     = 0.5;
Fillet          = 1.5;

EnclosureWidth      = PanelWidth  + (ShellThickness + PanelMargin) * 2; // 168.5
EnclosureHeight     = PanelHeight + (ShellThickness + PanelMargin) * 2; // 103
EnclosureDepth      = PanelDepth + PartsDepth + ShellThickness; // 21
echo("EncrosureWidth=", EnclosureWidth);
echo("EnclosureHeight=", EnclosureHeight);
echo("EnclosureDepth=", EnclosureDepth);

// --------------------------------------------------------------------------------
// box shell

module boxshell()
{
    w = EnclosureWidth;
    h = EnclosureHeight;
    d = EnclosureDepth;
    th = ShellThickness;
    f = Fillet;
    // cut in half
    difference()
    {
        // rounded hollow shell
        difference()
        {
            cuboid([w, h, d * 2], fillet=f);
            cuboid([w - th * 2, h - th * 2, d * 2 - th * 2], fillet = f - 1);
        }
        downcube([w + 0.1, h + 0.1, d + 0.1]);
    }
}

// --------------------------------------------------------------------------------
// punch holes

module rightholes()
{
    translate([EnclosureWidth/2, -EnclosureHeight/2, 0])
      rotate([0, 0, 90])
        rotate([90, 0, 0])
          translate([0, 0, -ShellThickness * 3])
            linear_extrude(height = ShellThickness * 4, center = false, convexity = 10)
              import(file = "face-right.dxf", layer = "holes");
}

module leftholes()
{
    translate([-EnclosureWidth / 2, EnclosureHeight / 2, 0])
      rotate([0, 0, -90])
        rotate([90, 0, 0])
          translate([0, 0, -ShellThickness * 3])
            linear_extrude(height = ShellThickness * 4, center = false, convexity = 10)
              import(file = "face-left.dxf", layer = "holes");
}

module bottomholes()
{
    translate([-EnclosureWidth / 2, -EnclosureHeight / 2, 0])
      rotate([90, 0, 0])
        translate([0, 0, -ShellThickness * 3])
          linear_extrude(height = ShellThickness * 4, center = false, convexity = 10)
            import(file = "face-bottom.dxf", layer = "holes");
}

module backholes()
{
    translate([-EnclosureWidth / 2, -EnclosureHeight / 2, EnclosureDepth])
      translate([0, 0, -ShellThickness * 3])
        linear_extrude(height = ShellThickness * 4, center = false, convexity = 10)
          import(file = "face-back.dxf", layer = "holes");
}

module allholes()
{
    leftholes();
    rightholes();
    bottomholes();
    backholes();
}

// --------------------------------------------------------------------------------
// mounting posts

module mountingboss(x, y)
{
    dpost = EnclosureDepth - PanelDepth - MountPostDepth - ShellThickness + Epsilon;
    echo("MountingPostDepth=", dpost);
    diapost = 8;
    lrib = 4; // x|y length of ribs
    thrib = 1.5; // thickness of ribs
    translate([x, y, PanelDepth + MountPostDepth])
    {
      tube(dpost, od=diapost, id=3);
      translate([diapost / 2, 0, 0])
        upcube([lrib, thrib, dpost]);
      translate([-diapost / 2, 0, 0])
        upcube([lrib, thrib, dpost]);
      translate([0, diapost / 2, 0])
        upcube([thrib, lrib, dpost]);
      translate([0, -diapost / 2, 0])
        upcube([thrib, lrib, dpost]);
    }
}

module allmoutingbosses()
{
    ScrewHoleOffset = 4;
    x = BoardWidth / 2 - ScrewHoleOffset;
    y = BoardHeight / 2 - ScrewHoleOffset;
    mountingboss(-x,  y);
    mountingboss( x,  y);
    mountingboss( x, -y);
    mountingboss(-x, -y);
}

module counterbore(x, y)
{
    screwheaddepth = 2;
    screwheaddia = 5;
    translate([x, y, EnclosureDepth])
      cyl(d=screwheaddia, h=screwheaddepth * 2);
}

module allcounterbore()
{
    ScrewHoleOffset = 4;
    x = BoardWidth / 2 - ScrewHoleOffset;
    y = BoardHeight / 2 - ScrewHoleOffset;
    counterbore(-x,  y);
    counterbore( x,  y);
    counterbore( x, -y);
    counterbore(-x, -y);
}

// --------------------------------------------------------------------------------
// ribs

module rib(x, y, angle)
{
    // NOTE: angle=0 means along the x-axis, increase in the ccw direction
    lrib = 10; // x|y length
    drib = 7; // z depth
    thrib = 1.5; // thickness
    translate([x, y, EnclosureDepth - drib - ShellThickness + Epsilon])
      rotate([0, 0, angle])
        cuboid([lrib, thrib, drib], fillet=3, edges=EDGE_BOT_RT, align=[1,0,1]);
}

module allribs()
{
    // top
    ytop = EnclosureHeight / 2 - ShellThickness + Epsilon;
    for(x = [-55,-20,20,60])
    {
        rib(x, ytop, -90);
    }
    // bottom
    ybottom = -EnclosureHeight / 2 + ShellThickness - Epsilon;
    for(x = [-55,-20,20,60])
    {
        rib(x, ybottom, 90);
    }
    // left
    xl = -EnclosureWidth / 2 + ShellThickness - Epsilon;
    for(y = [-39,23])
    {
        rib(xl, y, 0);
    }
    // right
    xr = EnclosureWidth / 2 - ShellThickness + Epsilon;
    for(y = [-27.25,-12,10,25])
    {
        rib(xr, y, 180);
    }
}

// --------------------------------------------------------------------------------
// skids

module skid(x)
{
    translate([x, -EnclosureHeight / 2, EnclosureDepth / 2])
      cyl(l=EnclosureDepth, r=1.5);
}

module allskids()
{
    skid(-52);
    skid( 52);
}

// --------------------------------------------------------------------------------
// assembly

difference()
{
    union()
    {
        boxshell();
        allmoutingbosses();
        allribs();
        allskids();
    }
    union()
    {
        allholes();
        allcounterbore();
    }
}
