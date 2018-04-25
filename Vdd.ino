//Internal Chip temperature an Voltage in stm32duino core
//Credits fpiSTM

uint16_t adc_read(uint32_t channel)
{
  ADC_HandleTypeDef AdcHandle = {};
  ADC_ChannelConfTypeDef  AdcChannelConf = {};
  __IO uint16_t uhADCxConvertedValue = 0;

  AdcHandle.Instance = ADC1;
  AdcHandle.State = HAL_ADC_STATE_RESET;
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;           /* Right-alignment for converted data */
  AdcHandle.Init.ScanConvMode          = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
  AdcHandle.Init.ContinuousConvMode    = DISABLE;                       /* Continuous mode disabled to have only 1 conversion at each conversion trig */
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;            /* Software start to trig the 1st conversion manually, without external event */
  AdcHandle.Init.NbrOfConversion       = 1;                             /* Specifies the number of ranks that will be converted within the regular group sequencer. */
  AdcHandle.Init.NbrOfDiscConversion   = 0;                             /* Parameter discarded because sequencer is disabled */

  if (HAL_ADC_Init(&AdcHandle) != HAL_OK) {
    return 0;
  }

  AdcChannelConf.Channel      = channel;             /* Specifies the channel to configure into ADC */
  AdcChannelConf.Rank         = ADC_REGULAR_RANK_1;               /* Specifies the rank in the regular group sequencer */
  AdcChannelConf.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;                     /* Sampling time value to be set for the selected channel */

  /*##-2- Configure ADC regular channel ######################################*/
  if (HAL_ADC_ConfigChannel(&AdcHandle, &AdcChannelConf) != HAL_OK)
  {
    /* Channel Configuration Error */
    return 0;
  }
  /*##-2.1- Calibrate ADC then Start the conversion process ####################*/
  if (HAL_ADCEx_Calibration_Start(&AdcHandle) !=  HAL_OK) {
    /* ADC Calibration Error */
    return 0;
  }
  /*##-3- Start the conversion process ####################*/
  if (HAL_ADC_Start(&AdcHandle) != HAL_OK) {
    /* Start Conversation Error */
    return 0;
  }
  /*##-4- Wait for the end of conversion #####################################*/
  /*  For simplicity reasons, this example is just waiting till the end of the
      conversion, but application may perform other tasks while conversion
      operation is ongoing. */
  if (HAL_ADC_PollForConversion(&AdcHandle, 10) != HAL_OK) {
    /* End Of Conversion flag not set on time */
    return 0;
  }
  /* Check if the continous conversion of regular channel is finished */
  if ((HAL_ADC_GetState(&AdcHandle) & HAL_ADC_STATE_REG_EOC) == HAL_ADC_STATE_REG_EOC) {
    /*##-5- Get the converted value of regular channel  ########################*/
    uhADCxConvertedValue = HAL_ADC_GetValue(&AdcHandle);
  }

  if (HAL_ADC_Stop(&AdcHandle) != HAL_OK) {
    /* Stop Conversation Error */
    return 0;
  }
  if(HAL_ADC_DeInit(&AdcHandle) != HAL_OK) {
    return 0;
  }
  return uhADCxConvertedValue;
}

void setup() {
  Serial.begin(115000);
  analogRead(A0); // Workaround to init g_current_pin used in HAL_ADC_MspInit
}

void loop() {
  float Vdd = readVdd();
  Serial.print("Vdd=  ");
  Serial.print(Vdd);
  Serial.print(" V Temp= ");
  Serial.print(readTempSensor(Vdd));
  Serial.println(" °C");
  delay(500);
}

static float readVdd()
{
  return (1.20 * 4096.0 / adc_read(ADC_CHANNEL_VREFINT)); // ADC sample to V
  //return (1200 * 4096 / adc_read(ADC_CHANNEL_VREFINT)); // ADC sample to mV
}

static float readTempSensor(float Vdd)
{
  return ((1.43 - (Vdd / 4096.0 * adc_read(ADC_CHANNEL_TEMPSENSOR))) / 0.0043 + 25.0);
}
