//
//  hubholder.scad
//  SensorClock.enclosure.misc
//
//  created by yu2924 on 2026-10-11
//

ThWall = 1.5;
HubWidth = 10.5;
HubHeight = 8.5;
OuterWidth = HubWidth + ThWall * 2;
OuterHeight = HubHeight + ThWall * 2;
CutoutWidth = HubWidth - 3;
BaseDepth = 20;

difference()
{
    cube([OuterWidth, OuterHeight, BaseDepth], center=false);
    union()
    {
        translate([ThWall, ThWall, -1])
            cube([HubWidth, HubHeight, BaseDepth + 2], center=false);
        translate([(OuterWidth - CutoutWidth) / 2, OuterHeight - ThWall - 1, -1])
            cube([CutoutWidth, ThWall + 2, BaseDepth + 2], center=false);
    }
}
