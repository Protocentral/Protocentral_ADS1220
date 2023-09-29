#include <iostream>

#include <wiringPi.h>

#include "ads1220.h"

#define CS_PIN 18
#define DRDY_PIN 13

using namespace std;

int main(void) {
  int err = wiringPiSetupPhys();
  if (err != 0) {
    cerr << "Couldn't setup wiringPi: " << err << endl;
    exit(err);
  }

  Ads1220 ads(CS_PIN, DRDY_PIN);

  uint32_t previousValue = 0;
  while (true) {
    uint32_t value = ads.readUntilNewValue(Ads1220::MultiplexerConfiguration::AIN0_SINGLE_ENDED, previousValue);
    cout << "AIN0: " << value << endl;
    previousValue = value;
  }
}
