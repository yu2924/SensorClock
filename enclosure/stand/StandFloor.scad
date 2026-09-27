//
//  StandFLoor.scad
//  SensorClock
//
//  created by yu2924 on 2026-09-21
//

$fn=100;

BoxWidth = 100;
FloorHeight = 21.48;
FloorThickness = 1.0;
ClawOffsetY = 9.5;
ClawDia = 2;
ClawDiaSmall = 1.5;
ClawAdjust = (ClawDia - ClawDiaSmall) / 2;

module boxfloor()
{
    // plate
    cube([BoxWidth, FloorHeight, FloorThickness]);
    // edges
    translate([0, 4, FloorThickness])
      cube([1.5, FloorHeight - 8, 2 - ClawAdjust]);
    translate([BoxWidth-1.5, 4, FloorThickness])
      cube([1.5, FloorHeight - 8, 2 - ClawAdjust]);
    // snapfit claws
    translate([-1, ClawOffsetY + ClawAdjust, FloorThickness + ClawAdjust])
      cube([2, ClawDiaSmall, ClawDiaSmall]);
    translate([BoxWidth-1, ClawOffsetY + ClawAdjust, FloorThickness + ClawAdjust])
      cube([2, ClawDiaSmall, ClawDiaSmall]);
}

boxfloor();
