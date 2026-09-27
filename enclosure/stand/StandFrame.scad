//
//  StandFrame.scad
//  SensorClock
//
//  created by yu2924 on 2026-09-21
//

$fn=100;

Thickness = 4;

module frame()
{
    linear_extrude(height = Thickness, center = false, convexity = 10)
        import(file = "sketch.dxf", layer = "frameoutline");
}

frame();
