#include "_Plugin_Helper.h"
#ifdef USES_P025

// #######################################################################################################
// #################################### Plugin 025: ADS1115 I2C 0x48)  ###############################################
// #######################################################################################################


# include "src/PluginStructs/P025_data_struct.h"

# define PLUGIN_025
# define PLUGIN_ID_025 25
# define PLUGIN_NAME_025 "Analog input - ADS1115"
# define PLUGIN_VALUENAME1_025 "Analog"

# define P025_I2C_ADDR    PCONFIG(0)
# define P025_GAIN        PCONFIG(1)
# define P025_MUX         PCONFIG(2)
# define P025_CAL         PCONFIG(3)

# define P025_CAL_ADC1    PCONFIG_LONG(0)
# define P025_CAL_OUT1    PCONFIG_FLOAT(0)
# define P025_CAL_ADC2    PCONFIG_LONG(1)
# define P025_CAL_OUT2    PCONFIG_FLOAT(1)


boolean Plugin_025(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  // static uint8_t portValue = 0;
  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_025;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_025);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_025));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      # define ADS1115_I2C_OPTION 4
      const uint8_t i2cAddressValues[] = { 0x48, 0x49, 0x4A, 0x4B };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), ADS1115_I2C_OPTION, i2cAddressValues, P025_I2C_ADDR);
      } else {
        success = intArrayContains(ADS1115_I2C_OPTION, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P025_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      uint8_t port = CONFIG_PORT;

      if (port > 0) // map old port logic to new gain and mode settings
      {
        P025_GAIN     = PCONFIG(0) / 2;
        P025_I2C_ADDR = 0x48 + ((port - 1) / 4);
        P025_MUX      = ((port - 1) & 3) | 4;
        CONFIG_PORT   = 0;
      }

      addFormSubHeader(F("Input"));

      {
        # define ADS1115_PGA_OPTION 6
        const __FlashStringHelper *pgaOptions[ADS1115_PGA_OPTION] = {
          F("2/3x gain (FS=6.144V)"),
          F("1x gain (FS=4.096V)"),
          F("2x gain (FS=2.048V)"),
          F("4x gain (FS=1.024V)"),
          F("8x gain (FS=0.512V)"),
          F("16x gain (FS=0.256V)")
        };
        addFormSelector(F("Gain"), F("gain"), ADS1115_PGA_OPTION, pgaOptions, nullptr, P025_GAIN);
      }

      {
        # define ADS1115_MUX_OPTION 8
        const __FlashStringHelper *muxOptions[ADS1115_MUX_OPTION] = {
          F("AIN0 - AIN1 (Differential)"),
          F("AIN0 - AIN3 (Differential)"),
          F("AIN1 - AIN3 (Differential)"),
          F("AIN2 - AIN3 (Differential)"),
          F("AIN0 - GND (Single-Ended)"),
          F("AIN1 - GND (Single-Ended)"),
          F("AIN2 - GND (Single-Ended)"),
          F("AIN3 - GND (Single-Ended)"),
        };
        addFormSelector(F("Input Multiplexer"), F("mux"), ADS1115_MUX_OPTION, muxOptions, nullptr, P025_MUX);
      }

      addFormSubHeader(F("Two Point Calibration"));

      addFormCheckBox(F("Calibration Enabled"), F("cal"), P025_CAL);

      addFormNumericBox(F("Point 1"), F("adc1"), P025_CAL_ADC1, -32768, 32767);
      html_add_estimate_symbol();
      addTextBox(F("out1"), toString(P025_CAL_OUT1, 3), 10);

      addFormNumericBox(F("Point 2"), F("adc2"), P025_CAL_ADC2, -32768, 32767);
      html_add_estimate_symbol();
      addTextBox(F("out2"), toString(P025_CAL_OUT2, 3), 10);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P025_I2C_ADDR = getFormItemInt(F("i2c_addr"));

      P025_GAIN = getFormItemInt(F("gain"));

      P025_MUX = getFormItemInt(F("mux"));

      P025_CAL = isFormItemChecked(F("cal"));

      P025_CAL_ADC1 = getFormItemInt(F("adc1"));
      P025_CAL_OUT1 = getFormItemFloat(F("out1"));

      P025_CAL_ADC2 = getFormItemInt(F("adc2"));
      P025_CAL_OUT2 = getFormItemFloat(F("out2"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // int value = 0;
      // uint8_t unit = (CONFIG_PORT - 1) / 4;
      // uint8_t port = CONFIG_PORT - (unit * 4);
      // uint8_t address = 0x48 + unit;

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P025_data_struct(P025_I2C_ADDR, P025_GAIN, P025_MUX));
      P025_data_struct *P025_data = static_cast<P025_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P025_data);
      break;
    }

    case PLUGIN_READ:
    {
      const P025_data_struct *P025_data = static_cast<P025_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P025_data) {
        const int16_t value = P025_data->read();
        UserVar[event->BaseVarIndex] = value;

        # ifndef BUILD_NO_DEBUG
        String log;

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          log  = F("ADS1115 : Analog value: ");
          log += value;
          log += F(" / Channel: ");
          log += P025_MUX;
        }
        # endif // ifndef BUILD_NO_DEBUG

        if (P025_CAL) // Calibration?
        {
          const int adc1   = P025_CAL_ADC1;
          const int adc2   = P025_CAL_ADC2;
          const float out1 = P025_CAL_OUT1;
          const float out2 = P025_CAL_OUT2;

          if (adc1 != adc2)
          {
            const float normalized = static_cast<float>(value - adc1) / static_cast<float>(adc2 - adc1);
            UserVar[event->BaseVarIndex] = normalized * (out2 - out1) + out1;
            # ifndef BUILD_NO_DEBUG

            if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
              log += ' ';
              log += formatUserVarNoCheck(event->TaskIndex, 0);
            }
            # endif // ifndef BUILD_NO_DEBUG
          }
        }

        // TEST log += F(" @0x");
        // TEST log += String(config, 16);
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLogMove(LOG_LEVEL_DEBUG, log);
        }
        # endif // ifndef BUILD_NO_DEBUG
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P025
