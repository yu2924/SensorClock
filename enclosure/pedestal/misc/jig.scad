//
//  jig.scad
//  SensorClock.enclosure.misc
//
//  created by yu2924 on 2026-10-03
//

include <BOSL/constants.scad>
use <BOSL/shapes.scad>

$fn=30;

CmpXY = 0;
CmpZ = 0;
Width = 101 - CmpXY;
Depth = 19 - CmpXY;
Height = 22 - CmpZ;

module block(x, t)
{
    translate([x, 0, 0])
    {
        difference()
        {
            cube([t, Depth, Height], center=false);
            union()
            {
                translate([-1, Depth, Height/2])
                    cyl(l=t+2, d=Height/2, center=false, orient=ORIENT_X);
                translate([-1, Depth/2, Height])
                    cyl(l=t+2, d=Depth/2, center=false, orient=ORIENT_X);
            }
        }
    }
}

// left block
block(0, 4);
// center block
block(Width/2-1.5, 3);
// right block
block(Width-4, 4);
// bottom beam
cube([Width, 10, 3], center=false);
// vert beam
cube([Width, 3, 15], center=false);

// left stop
translate([-3, -4, 0]) cube([7, 4, Height + 3], center=false);
// right stop
translate([Width-4, -4, 0]) cube([7, 4, Height + 3], center=false);
