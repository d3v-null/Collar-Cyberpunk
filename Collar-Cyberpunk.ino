#include<FastLED.h>

// Serial Baud rate
#define SERIAL_BAUD 115200

/**
*  Debug stuff
*/
#define DEBUG       1

// Length of various buffer
#define BUFFLEN_MSG 300

// Serial out Buffer
char msg_buffer[BUFFLEN_MSG];

#define SERIAL_OBJ Serial

// snprintf to output buffer
#define SNPRINTF_MSG(...) \
snprintf(msg_buffer, BUFFLEN_MSG, __VA_ARGS__);

// snprintf to output buffer then println to serial
#define SER_SNPRINTF_MSG(...)  \
SNPRINTF_MSG(__VA_ARGS__); \
SERIAL_OBJ.println(msg_buffer);

/**
* LED stuff
*/

#define LED_PIN     12
#define BRIGHTNESS  20
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB

const uint8_t kMatrixWidth  = 28;
const uint8_t kMatrixHeight = 4;
const bool    kMatrixSerpentineLayout = true;

// Generates a rhombus stripe pattern that transitions between colors in a 16 color palette.
// stripe is symmetrical pattern like this
//
//                  ----------- Plane of symmetry
//                /     /<->/ - pulse_width * wavelength
// -y -+++++-----|-----+++++-
// ^  --+++++----|----+++++--
// |  ---+++++---|---+++++---
//    ----+++++--|--+++++----
// +                      -> x (width)
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

float pulse_width = 0.5;
float pw_rise = 0.0;
float pw_fall = 0.0;
float line_lag = 1.0;
float wavelength = 5;

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
#define MAX_DIMENSION ((kMatrixWidth>kMatrixHeight) ? kMatrixWidth : kMatrixHeight)
#define PALETTE_LENGTH 16

// The leds
CRGB leds[kMatrixWidth * kMatrixHeight];

// At speed = 1.0, a wave will travel one pixel in one second.
// speed is set dynamically once we've started up
float speed = 1.0;

CRGBPalette16 currentPalette( PartyColors_p );

uint16_t start_time;

uint16_t delta(){
    return millis() - start_time;
}

void setup() {
    // initialize serial
    SERIAL_OBJ.begin(SERIAL_BAUD);

    start_time = millis();

    #if DEBUG
    SER_SNPRINTF_MSG("SETUP");
    #endif

    delay(3000);

    LEDS.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,NUM_LEDS);
    LEDS.setBrightness(BRIGHTNESS);
}

uint8_t colorFunction(float theta) {
    // Generates a palette index from an angle theta
    // TODO: calculate theta colour from theta, PALETTE_LENGTH
    if( 0 <= theta && theta < pulse_width ) {
        return PALETTE_LENGTH - 1;
    } else {
        return 0;
    }
    // TODO: implement pw_fall and pw_rise sections
}

float mirrored( int x ){
    // distance of x from centre of collar
    return abs( float(x) - (float(kMatrixWidth) / 2.0));
}

//
// Mark's xy coordinate mapping code.  See the XYMatrix for more information on it.
//
uint16_t XY( uint8_t x, uint8_t y)
{
    uint16_t i;
    if( kMatrixSerpentineLayout == false) {
        i = (y * kMatrixWidth) + x;
    }
    if( kMatrixSerpentineLayout == true) {
        if( y & 0x01) {
            // Odd rows run backwards
            uint8_t reverseX = (kMatrixWidth - 1) - x;
            i = (y * kMatrixWidth) + reverseX;
        } else {
            // Even rows run forwards
            i = (y * kMatrixWidth) + x;
        }
    }
    return i;
}

void mapRhombiiToLEDsUsingPalette()
{
    for(int y = 0; y < kMatrixHeight; y++) {
        for(int x = 0; x < kMatrixWidth; x++) {
            // (x,y) is coordinate of point
            // theta is angle within color function which repeats (like a sine wave) from 0 to 1
            // real_x is the position in the colour axis
            float real_x = (float)(mirrored(x) - (float)(line_lag * (float)(y)));
            real_x += speed * (float)(delta()) / 1000.0;
            float theta = fmod(real_x, wavelength);

            uint16_t index = colorFunction(theta);

            CRGB color = ColorFromPalette( currentPalette, index, 255);
            uint8_t led_number = XY(x,y);
            #if DEBUG
            SER_SNPRINTF_MSG("real_x %s, wavelength %s", dtostrf(real_x), dtostrf(wavelength));
            SER_SNPRINTF_MSG("theta %s, index %d", dtostrf(theta), index);
            SER_SNPRINTF_MSG("(x, y) = (%d, %d) led_number %d", x, y, led_number);
            #endif

            leds[led_number] = color;
        }
    }
}

// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.

// 1 = 5 sec per palette
// 2 = 10 sec per palette
// etc
#define HOLD_PALETTES_X_TIMES_AS_LONG 1

void changePaletteAndSettingsPeriodically()
{
    if(1) { SetupOrangeAndDarkRedPalette();           speed = 1.0; wavelength =   5; pulse_width = 0.1; }
}

void SetupOrangeAndDarkRedPalette()
{
    fill_solid( currentPalette, PALETTE_LENGTH, CRGB::Orange);
    CRGB dark_red = CHSV( HUE_RED, 255, 10);
    currentPalette[0] = dark_red;
}

void loop() {
    #if DEBUG
    SER_SNPRINTF_MSG("LOOP");
    SER_SNPRINTF_MSG("time is %d", (delta() / 1000.0));
    #endif
    // Periodically choose a new palette, speed, and scale
    changePaletteAndSettingsPeriodically();

    // convert the noise data to colors in the LED array
    // using the current palette
    mapRhombiiToLEDsUsingPalette();

    LEDS.show();
    // delay(10);
}
