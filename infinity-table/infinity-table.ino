#include <FastLED.h>

#define NUM_LEDS 260
#define DATA_PIN 9
#define MAX_BRIGHT 30


static CRGB leds[NUM_LEDS];


class SquaredVal {
public:
	SquaredVal() {
		for (uint32_t i=0; i<sizeof(values)/sizeof(values[0]); ++i) {
			float f = (float) i / 255.0f;
			values[i] = (uint8_t) (f * f * 255.0f);
		}
	}
	uint8_t operator [] (unsigned int const idx) const {
		if (idx > sizeof(values)/sizeof(values[0])) {
			Serial.print("SquaredVal["); Serial.print(idx); Serial.println("] called\n");
			while (1);
		}
		return values[idx];
	}

private:
	uint8_t values[256];
};


static CRGB CrbgInit(uint8_t const r, uint8_t const g, uint8_t const b)
{
	return {.red = (uint8_t) ((float) r / 255.0f * (float) MAX_BRIGHT), 
		.green = (uint8_t) ((float) g / 255.0f * (float) MAX_BRIGHT), 
		.blue = (uint8_t) ((float) b / 255.0f * (float) MAX_BRIGHT)};
}


class ColorRange {
public:
	ColorRange(CRGB const &high, CRGB const &low) : 
		high_(high), low_(low),
		redInc_((float) (high.red - low.red) / 100.0f), 
		greenInc_((float) (high.green - low.green) / 100.0f),
		blueInc_((float) (high.blue - low.blue) / 100.0f) {}

	CRGB percent(float const pc) const {
		return {(uint8_t) (low_.red + (uint8_t) (redInc_ * pc)), (uint8_t) (low_.green + (uint8_t) (greenInc_ * pc)), (uint8_t) (low_.blue + (uint8_t) (blueInc_ * pc))};
	}

private:
	CRGB const high_;
	CRGB const low_;
	float const redInc_;
	float const greenInc_;
	float const blueInc_;
};


static SquaredVal const squaredVal;

static CRGB const black = {.red = 0, .green = 0, .blue = 0};
static ColorRange const pink = {CrbgInit(255, 110, 199), CrbgInit(26, 11, 20)};
// static ColorRange const forest = {CrbgInit(57, 255, 20), CrbgInit(6, 26, 2)};
static ColorRange const forest = {CrbgInit(0x49, 0xE2, 0x0E), CrbgInit(0x55, 0x10, 0x33)};
static ColorRange const ocean = {CrbgInit(0, 180, 255), CrbgInit(0, 18, 26)};
static ColorRange const autumn = {CrbgInit(255, 0, 0), CrbgInit(255, 255, 0)};

static int8_t const colorInc = 2;


static bool stop = false;


class LedState {
public:
	LedState() : percent_(0), direction_(0) {}
	LedState(int8_t const percent, int8_t const direction) : percent_(percent), direction_(direction) {}

	void copy(CRGB &rgb, ColorRange const &range) const {
		rgb = range.percent(percent_);
	}
	void advance(void) {
		int8_t const res = percent_ + colorInc * direction_;
		if (res < 0) {
			direction_ = 1;
		}
		else if (res > 100) {
			direction_ = -1;
		}
		else {
			percent_ = res;
			return;
		}
		advance();
	}

private:
	int8_t percent_;
	int8_t direction_;
};

static LedState ledState[NUM_LEDS];
static unsigned long const changeWindow = 5000;
static unsigned long nextChange;

void setup() {
	Serial.begin(115200);

	FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);

	for (int i=0; i<NUM_LEDS; ++i) {
		ledState[i] = {0, 1};
	}
	for (int i=0; i<min(NUM_LEDS, 8); ++i) {
		ledState[i] = {(int8_t) (i * 10), 1};
	}

	nextChange = millis() + changeWindow;
}


void loop() {
	static uint8_t c = 0;
	static ColorRange const *colors[] = {&forest, &autumn, &ocean, &pink};

	for (int i=0; i<NUM_LEDS; ++i) {
		ledState[i].advance();
		ledState[i].copy(leds[i], *colors[c]);
	}

	FastLED.show();
	delay(10);

	if ((int) (nextChange - millis()) <= 0) {
	Serial.println("Change");
		nextChange += changeWindow;
		if (++c >= (uint8_t) (sizeof(colors) / sizeof(colors[0]))) {
			c = 0;
		}
	}

	if (0) {
		for (int i=0; i<NUM_LEDS; ++i) {
			Serial.print(i);
			Serial.print(": ");
			Serial.print(leds[i].red);
			Serial.print(", ");
			Serial.print(leds[i].green);
			Serial.print(", ");
			Serial.println(leds[i].blue);
		}
		while (1);
	}
}
