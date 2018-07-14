# Collar-Cyberpunk
Firmware for displaying a pattern based on cyberpunk 2077 on an LED matrix

# How to use
from the code
```c
// Generates a rhombus stripe pattern that transitions between colors in a 16 color palette.
// stripe is symmetrical pattern like this
//
//                  ----------- Plane of symmetry
//                /     /<->/ - pulse_width * wavelength
// -y -+++++-----|-----+++++-
// ^  --+++++----|----+++++--
// |  ---+++++---|---+++++---
//    ----+++++--|--+++++----
// +                      -> x
//            <-   ->         Rhombus move direction
// Angle of rhombus determined by line_lag, how much x lags as you move in y.
//
// for a color function that looks like this
//
//  ^ c
//  | ----          -           - peak
//         \      /             - rise and fall
//          -----               - trough
//  +               -> th
//  |<->|                       - pulse width determines width of peak
//      |<->|                   - falling width determined by pw_fall
//              |<->|           - rising width determined by pw_fall
//  |<------------->|           - sum <= 1.0
```
