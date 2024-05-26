/* ============================================================================================================
 *    ёRadio
 * ============================================================================================================
 *    Web-radio based on
 *    ESP32-audioI2S     https://github.com/schreibfaul1/ESP32-audioI2S
 *    or/and
 *    ESP32-vs1053_ext   https://github.com/schreibfaul1/ESP32-vs1053_ext
 *    libraries
 * ============================================================================================================
 *    Project home       https://github.com/e2002/yoradio
 *    Wiki               https://github.com/e2002/yoradio/wiki
 *    Описание на 4PDA   https://4pda.to/forum/index.php?s=&showtopic=1010378&view=findpost&p=112992611
 *    Как это прошить?   https://4pda.to/forum/index.php?act=findpost&pid=112992611&anchor=Spoil-112992611-2
 * ============================================================================================================
 *    Here goes!
 * ============================================================================================================
 */
#include "Arduino.h"
#include "core/options.h"
#include "core/config.h"
#include "core/telnet.h"
#include "core/player.h"
#include "core/display.h"
#include "core/network.h"
#include "core/netserver.h"
#include "core/controls.h"
#include "core/mqtt.h"
#include "core/optionschecker.h"
#ifdef BATT_ADC

#include "driver/adc.h"
#include "esp_adc_cal.h"
#endif
extern __attribute__((weak)) void yoradio_on_setup();
#ifdef BATT_ADC
static esp_adc_cal_characteristics_t *adcChar;

float LastBATVoltage[10] = {0, 0, 0, 0, 0};
int LastBATVoltageCount = 0;
bool BATVoltageReady = false;
float GetBATVoltage()
{
  uint32_t readRaw;
  uint32_t read_voltage;
  float retr;
  readRaw = adc1_get_raw(ADC1_CHANNEL_3);
  read_voltage = esp_adc_cal_raw_to_voltage(readRaw, adcChar);
  retr = read_voltage;
  retr /= 1000;
  retr -= 0.02;
  LastBATVoltage[LastBATVoltageCount++] = retr * 2;
  if (LastBATVoltageCount == 10)
  {
    LastBATVoltageCount = 0;
    BATVoltageReady = true;
  }
  if (!BATVoltageReady)
  {
    return retr * 2;
  }
  else
  {
    retr = 0;
    for (int i = 0; i < 10; i++)
    {
      retr += LastBATVoltage[i];
    }
    retr /= 10;
  }

  return retr;
}
#endif
void setup()
{
  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  setCpuFrequencyMhz(240);
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);
  adcChar = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t cal_mode = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, adcChar);
  if (LED_BUILTIN != 255)
    pinMode(LED_BUILTIN, OUTPUT);
  if (yoradio_on_setup)
    yoradio_on_setup();
  config.init();
  display.init();
  player.init();
  network.begin();
  if (network.status != CONNECTED && network.status != SDREADY)
  {
    netserver.begin();
    initControls();
    display.putRequest(DSP_START);
    while (!display.ready())
      delay(10);
    return;
  }
  if (SDC_CS != 255)
  {
    display.putRequest(WAITFORSD, 0);
    Serial.print("##[BOOT]#\tSD search\t");
  }
  config.initPlaylistMode();
  netserver.begin();
  telnet.begin();
  initControls();
  display.putRequest(DSP_START);
  while (!display.ready())
    delay(10);
#ifdef MQTT_ROOT_TOPIC
  mqttInit();
#endif
  if (config.getMode() == PM_SDCARD)
    player.initHeaders(config.station.url);
  player.lockOutput = false;
  if (config.store.smartstart == 1)
    player.sendCommand({PR_PLAY, config.store.lastStation});
}

void loop()
{
  telnet.loop();
  if (network.status == CONNECTED || network.status == SDREADY)
  {
    player.loop();
    // loopControls();
  }
  loopControls();
  netserver.loop();
}

#include "core/audiohandlers.h"
