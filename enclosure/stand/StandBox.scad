//
//  StandBox.scad
//  SensorClock
//
//  created by yu2924 on 2026-09-21
//

include <BOSL/constants.scad>
use <BOSL/shapes.scad>

//$fn=10;

OffsetX = 25.5214;
BoxWidth = 100;

ConHoleOffsetX = 2.4;
ConHoleOffsetY = 5.6713;
ConHoleWidth = 26;
ConHoleHeight = 10;

BackHoleOffsetX1 = 7.5;
BackHoleOffsetX2 = 72.5;
BackHoleOffsetY = 9;
BackHoleWidth = 20;
BackHoleHeight = 8;

RibThickness = 1.5;
RibOffsetX1 = 34.25;
RibOffsetX2 = 64.25;

module boxrib(x, edge)
{
    translate([x, 0, 0])
      rotate(90, [0, 0, 1])
        rotate(90, [1, 0, 0])
          linear_extrude(height = RibThickness, center = false, convexity = 1)
            translate([-OffsetX, 0, 0])
              if(edge)
                import(file = "sketch.dxf", layer = "boxribedge");
              else
                import(file = "sketch.dxf", layer = "boxrib");
}

module boxoutline()
{
    rotate(90, [0, 0, 1])
      rotate(90, [1, 0, 0])
        linear_extrude(height = BoxWidth, center = false, convexity = 1)
          translate([-OffsetX, 0, 0])
            import(file = "sketch.dxf", layer = "boxoutline");
}

module box()
{
    difference()
    {
        boxoutline();
        // holes
        union()
        {
        translate([ConHoleOffsetX, -5, ConHoleOffsetY])
          cuboid([ConHoleWidth, 10, ConHoleHeight], fillet=1, center=false);
        translate([BackHoleOffsetX1, 20, BackHoleOffsetY])
          cuboid([BackHoleWidth, 10, BackHoleHeight], fillet=1, center=false);
        translate([BackHoleOffsetX2, 20, BackHoleOffsetY])
          cuboid([BackHoleWidth, 10, BackHoleHeight], fillet=1, center=false);
        }
    }
    boxrib(0, edge=true);
    boxrib(RibOffsetX1, edge=false);
    boxrib(RibOffsetX2, edge=false);
    boxrib(BoxWidth - RibThickness, edge=true);
}

box();
