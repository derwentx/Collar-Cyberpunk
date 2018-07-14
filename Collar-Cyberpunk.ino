#include<FastLED.h>

#define LED_PIN     12
#define BRIGHTNESS  20
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define DEBUG       1

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

void setup() {
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

void mapRhombiiToLEDsUsingPalette()
{
  for(int y = 0; y < kMatrixWidth; y++) {
    for(int x = 0; x < kMatrixHeight; x++) {
      // (x,y) is coordinate of point
      // theta is angle within color function which repeats (like a sine wave) from 0 to 1
      // real_x is the position in the colour axis


      float real_x = (float)(mirrored(x) - (float)(line_lag * (float)(y)));
      real_x += speed * (float)(millis()) * 1000.0;
      float theta = fmod(real_x, wavelength);

      // TODO: calculate theta from x , y , speed

      uint8_t index = colorFunction(theta);

      CRGB color = ColorFromPalette( currentPalette, index, 255);
      leds[XY(x,y)] = color;
    }
  }
}

void loop() {
  // Periodically choose a new palette, speed, and scale
  changePaletteAndSettingsPeriodically();

  // convert the noise data to colors in the LED array
  // using the current palette
  mapRhombiiToLEDsUsingPalette();

  LEDS.show();
  // delay(10);
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

// This function generates a random palette that's a gradient
// between four different colors.  The first is a dim hue, the second is
// a bright hue, the third is a bright pastel, and the last is
// another bright hue.  This gives some visual bright/dark variation
// which is more interesting than just a gradient of different hues.
void SetupRandomPalette()
{
  currentPalette = CRGBPalette16(
                      CHSV( random8(), 255, 32),
                      CHSV( random8(), 255, 255),
                      CHSV( random8(), 128, 255),
                      CHSV( random8(), 255, 255));
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
  // 'black out' all 16 palette entries...
  fill_solid( currentPalette, PALETTE_LENGTH, CRGB::Black);
  // and set every fourth one to white.
  currentPalette[0] = CRGB::White;
  currentPalette[4] = CRGB::White;
  currentPalette[8] = CRGB::White;
  currentPalette[12] = CRGB::White;

}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
  CRGB purple = CHSV( HUE_PURPLE, 255, 255);
  CRGB green  = CHSV( HUE_GREEN, 255, 255);
  CRGB black  = CRGB::Black;

  currentPalette = CRGBPalette16(
    green,  green,  black,  black,
    purple, purple, black,  black,
    green,  green,  black,  black,
    purple, purple, black,  black );
}

void SetupOrangeAndDarkRedPalette()
{
    fill_solid( currentPalette, PALETTE_LENGTH, CRGB::Orange);
    CRGB dark_red = CHSV( HUE_RED, 255, 10);
    currentPalette[0] = dark_red;
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
