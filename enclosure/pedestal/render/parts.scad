//
//  parts.scad
//  SensorClock.enclosure.stand.render
//
//  created by yu2924 on 2026-10-03
//

$fn=100;

module frame()
{
    linear_extrude(height = 3, center = false, convexity = 10)
        import(file = "frame.dxf", layer = "render");
}

module beamv()
{
    linear_extrude(height = 3, center = false, convexity = 10)
        import(file = "beamv.dxf", layer = "render");
}

module beamh()
{
    linear_extrude(height = 3, center = false, convexity = 10)
        import(file = "beamh.dxf", layer = "render");
}

frame();
translate([60,0,0]) frame();
translate([0,-30,0]) beamv();
translate([0,-60,0]) beamh();

