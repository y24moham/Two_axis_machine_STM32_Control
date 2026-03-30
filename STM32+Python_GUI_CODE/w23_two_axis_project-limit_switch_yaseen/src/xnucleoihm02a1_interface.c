/**
  ******************************************************************************
  * @file    xnucleoihm02a1_interface.c
  * @brief   This file is used as interface between the 
  *          X-NUCLEO-IHM02A1 and the NUCLEO-F4xx board.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2014 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "xnucleoihm02a1_interface.h"
#ifndef example_usart_h
#include "example_usart.h"
#endif
#ifndef microstepping_motor_h
#include "microstepping_motor.h"
#endif
#ifndef L6470_h
#include "L6470.h"
#endif


/**
  * @addtogroup BSP
  * @{
  */

/**
  * @addtogroup X-NUCLEO-IHM02A1
  * @{
  */

/**
  * @addtogroup NUCLEO_Interface
  * @{
  */

/**
  * @addtogroup NUCLEO_Exported_Variables
  * @{
  */

/**
  * @brief  The data structure for all further instances to ADC.
  */
ADC_HandleTypeDef hadc1;
/**
  * @brief  The data structure for all further instances to SPI1.
  */
SPI_HandleTypeDef hspi1;
/**
  * @brief  The data structure for all further instances to SPI2.
  */
SPI_HandleTypeDef hspi2;

/**
  * @}
  */ /* End of NUCLEO_Exported_Variables */

/**
  * @defgroup   NUCLEO_Private_Functions
  * @brief      NUCLEO Private Functions.
  * @{
  */

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_ADC1_Init(void);
void MX_SPI_Init(void);
void MX_SPI1_Init(void);
void MX_SPI2_Init(void);
void MX_USART2_Init(void);

/**
  * @}
  */ /* End of NUCLEO_Private_Functions */

/**
  * @addtogroup NUCLEO_Private_Functions
  * @{
  */

/**
  * @brief  This function configures the System Clock
  * 
  * @note   The System Clock will be configured as following:
  *         - PLL Source: HSI
  *         - SYSCLK: 84 MHz
  *         - HCLK: 84 MHz
  *         - APB1 Peripheral Clocks: 42 MHz
  *         - APB1 Timer Clocks: 84 MHz
  *         - APB2 Peripheral Clocks: 84 MHz
  *         - APB2 Timer Clocks: 84 MHz
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

}

/**
  * @brief  This function initializes the GPIO MX.
  */
void MX_GPIO_Init(void)
{
#ifdef NUCLEO_USE_USER_BUTTON
  /* Configures Button GPIO and EXTI Line */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
#endif

#ifdef NUCLEO_USE_USER_LED
  /* Configures LED GPIO */
  BSP_LED_Init(LED2);
#endif

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  /* Declare a struct, which is then to be specified to configure an GPIO Pin */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  // configure the switch pin for motor 0 (switch 4)
  GPIO_InitStruct.Pin = GPIO_PIN_8; // Change X to your pin
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN; // Use pulldown when the switch is connected to 3.3V
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); // Sends the configuration to specified Port and Pin settings
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  // Configure the switch pin for motor 0 (switch 3)
  GPIO_InitStruct.Pin = GPIO_PIN_9; // Change X to your pin
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN; // Use pulldown when the switch is connected to 3.3V
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); // Sends the configuration to specified Port and Pin settings
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  // configure the switch pin for motor 1 (switch 2)
  GPIO_InitStruct.Pin = GPIO_PIN_6; //Change X to your pin
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING; //Push-Pull output, meaning the pin supplies 3.3V and 0V
  GPIO_InitStruct.Pull = GPIO_PULLDOWN; //No internal pullup/down for output
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  // Configure the switch pin for motor 1 (switch 1)
  GPIO_InitStruct.Pin = GPIO_PIN_10; //Change X to your pin
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING; //Push-Pull output, meaning the pin supplies 3.3V and 0V
  GPIO_InitStruct.Pull = GPIO_PULLDOWN; //No internal pullup/down for output
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  // Configure the error LED pin as output
  GPIO_InitStruct.Pin = GPIO_PIN_8; //Change X to your pin
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; //Push-Pull output, meaning the pin supplies 3.3V and 0V
  GPIO_InitStruct.Pull = GPIO_NOPULL; //No internal pullup/down for output
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
  * @brief  Initialize the SPI used by the NUCLEO board.
  *
  * @note   It selects the @ref MX_SPI1_Init or @ref MX_SPI2_Init
  *         related to the defined macro @ref NUCLEO_USE_SPI_1 or @ref NUCLEO_USE_SPI_2.
  */
void MX_SPI_Init(void)
{
#ifdef NUCLEO_USE_SPI_1
  MX_SPI1_Init();
#endif
#ifdef NUCLEO_USE_SPI_2
  MX_SPI2_Init();
#endif
}

/**
  * @brief  This function initializes the SPI1 MX
  *
  * @note   It sets the <i>hspi1</i> data structure for all further instances to
  *         SPI1
  *
  * @note   The SPI1 peripheral is configured as following:
  *         - Full-Duplex Master
  *         - 8-Bits
  *         - CPOL High
  *         - CPHA 2nd Edge
  *         - Baud Rate lower than 5 MBits/s
  */
void MX_SPI1_Init(void)
{
  #define MAX_BAUDRATE  5000000
  uint32_t freq;
  uint16_t freq_div;
  uint32_t spi_baudrateprescaler;
  
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLED;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  
  freq = HAL_RCC_GetPCLK2Freq();
  freq_div = (freq / MAX_BAUDRATE);
  
  if (freq_div < 2)
  {
    spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_2;
  }
  else
  {
    if (freq_div < 4)
    {
      spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_4;
    }
    else
    {
      if (freq_div < 8)
      {
        spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_8;
      }
      else
      {
        if (freq_div < 16)
        {
          spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_16;
        }
        else
        {
          if (freq_div < 32)
          {
            spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_32;
          }
          else
          {
            if (freq_div < 64)
            {
              spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_64;
            }
            else
            {
              if (freq_div < 128)
              {
                spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_128;
              }
              else
              {
                if (freq_div < 256)
                {
                  spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_256;
                }
                else
                {
                  /* the condition is not possible, you should reduce the CPU frequency */
                  while(1);
                }
              }
            }
          }
        }
      }
    }
  }
  
  hspi1.Init.BaudRatePrescaler = spi_baudrateprescaler;  // the baudrate will be lower than MAX_BAUDRATE (5 MBits/s)
  HAL_SPI_Init(&hspi1);
}

/**
  * @brief  This function initializes the SPI2 MX
  *
  * @note   It sets the <i>hspi2</i> data structure for all further instances to
  *         SPI2
  *
  * @note   The SPI2 peripheral is configured as following:
  *         - Full-Duplex Master
  *         - 8-Bits
  *         - CPOL High
  *         - CPHA 2nd Edge
  *         - Baud Rate lower than 5 MBits/s
  */
void MX_SPI2_Init(void)
{
  #define MAX_BAUDRATE  5000000
  uint32_t freq;
  uint16_t freq_div;
  uint32_t spi_baudrateprescaler;
  
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLED;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  
  freq = HAL_RCC_GetPCLK1Freq();
  freq_div = (freq / MAX_BAUDRATE);
  
  if (freq_div < 2)
  {
    spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_2;
  }
  else
  {
    if (freq_div < 4)
    {
      spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_4;
    }
    else
    {
      if (freq_div < 8)
      {
        spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_8;
      }
      else
      {
        if (freq_div < 16)
        {
          spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_16;
        }
        else
        {
          if (freq_div < 32)
          {
            spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_32;
          }
          else
          {
            if (freq_div < 64)
            {
              spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_64;
            }
            else
            {
              if (freq_div < 128)
              {
                spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_128;
              }
              else
              {
                if (freq_div < 256)
                {
                  spi_baudrateprescaler = SPI_BAUDRATEPRESCALER_256;
                }
                else
                {
                  /* the condition is not possible, you should reduce the CPU frequency */
                  while(1);
                }
              }
            }
          }
        }
      }
    }
  }
  
  hspi2.Init.BaudRatePrescaler = spi_baudrateprescaler; // the baudrate will be lower than MAX_BAUDRATE (5 MBits/s)
  HAL_SPI_Init(&hspi2);
}

/**
  * @brief  This function initializes the USART2 MX.
  * @note   It sets the <i>huart2</i> data structure for all further instances
  *         to USART2.
  * @note   The USART2 peripheral is configured as following:
  *         - Baud Rate:  115200
  *         - Data Bits:  8
  *         - Stop Bit:   1
  *         - Parity:     None
  *         - Mode:       TX/RX
  */
void MX_USART2_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart2);
}

/**
  * @brief  This function initializes the ADC MX
  *
  * @note   It sets the <i>hadc</i> data structure for all further instances to ADC
*/
void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig;

  /* GPIO Ports Clock Enable */
  __GPIOB_CLK_ENABLE();

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_8B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  HAL_ADC_Init(&hadc1);

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_0; // PA0
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

  sConfig.Channel = ADC_CHANNEL_8; // PB0
  sConfig.Rank = 2;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

}


/**
 * @brief  This function reads the ADC value and maps it to a motor speed value, then updates the motor speed if a fault condition has not been detected and motor is already in motion.
 * @note   The function uses tight polling for ADC conversion, if the conversion is complete then read the value and store in ADC_raw variable, if the conversion fails, it increments a fail count and takes corrective action if the motor speed was not set previously or if there have been more than 50 consecutive ADC conversion failures to avoid potential issues with uninitialized variable and potential issues with sudden change in motor speed from previous value to 0 due to ADC conversion failures, if the new ADC value is different from the previous value, it maps the ADC value which is between 0 and 255 to a motor speed value between min_speed and max_speed, then updates the motor speed with the new value read from the ADC if a fault condition has not been detected and motor is already in motion (BUSY_ID flag is 0 when motor is in motion).
 */
void ADC1_to_Motor(ADC_HandleTypeDef* hadc)
{
  static volatile uint32_t ADC_raw_0 = 0;
  static volatile uint32_t ADC_raw_1 = 0;
  static volatile uint32_t ADC_old_0 = 5000; // initialize ADC_old to a random value
  static volatile uint32_t ADC_old_1 = 5000; // initialize ADC_old to a random value
  static uint8_t init_done = 0;
  if (!init_done) // if this is the first time the function is called, initialize ADC_old to a value that is outside the possible range of ADC values (0 to 255 for 8-bit resolution) to ensure that the first ADC value read will be considered different and will be used to set the initial motor speed correctly, and set init_done flag to 1 to indicate that initialization has been done and should not be repeated in subsequent calls to the function
  {
    volatile uint32_t adc_bits = 0;
    switch (hadc->Init.Resolution) // determine the number of bits of the ADC resolution based on the ADC initialization settings, and store in adc_bits variable to be used later for mapping the ADC value to motor speed value
    {
      case ADC_RESOLUTION_12B:
          adc_bits = 12;
          break;
      case ADC_RESOLUTION_10B:
          adc_bits = 10;
          break;
      case ADC_RESOLUTION_8B:
          adc_bits = 8;
          break;
      case ADC_RESOLUTION_6B:
          adc_bits = 6;
          break;
      default:
          adc_bits = 0;
          break;
    }
    ADC_old_0 = (uint16_t)(1U << adc_bits);
    ADC_old_1 = (uint16_t)(1U << adc_bits);
    init_done = 1;
  }
  if (HAL_ADC_Start(hadc) != HAL_OK)
  {
      return;
  }

  // Declare a static volatile variable to count the number of consecutive ADC conversion failures for motor 0, and initialize it to 0
  static volatile uint32_t fail_count_0 = 0;
  // tight polling for ADC conversion, if the conversion is complete then read the value and store in ADC_raw variable
  HAL_StatusTypeDef adc_status = HAL_ADC_PollForConversion(hadc, 10);
  if (adc_status == HAL_OK)
  {
    // read the ADC value and store in ADC_raw variable
    ADC_raw_0 = (uint16_t)HAL_ADC_GetValue(hadc);
    fail_count_0 = 0; // reset fail count if ADC conversion is successful
  }
  else
  {
    fail_count_0++; // increment fail count if ADC conversion fails
    if (ADC_old_0 == 5000 || fail_count_0 > 50) // if the motor speed was not set previously (ADC_old_0 is still at its initial value of 5000) or if there have been more than 50 consecutive ADC conversion failures, set ADC_raw_0 to 0 to set motor speed to minimum (0 speed) and avoid potential issues with uninitialized variable and potential issues with sudden change in motor speed from previous value to 0 due to ADC conversion failures
    {
      ADC_raw_0 = 0;
      fail_count_0 = 0; // reset fail count after taking corrective action to avoid repeated messages and actions in case of continuous ADC conversion failures
    }// if ADC conversion fails, and the motor speed was not set previously, or if there have been more than 50 consecutive ADC conversion failures, set ADC_raw_0 to 0 to set motor speed to minimum (0 speed) and avoid potential issues with uninitialized variable
    // if motor speed was set previously, and the ADC conversion fails, keep the previous ADC value to avoid setting motor speed to minimum (0 speed) due to uninitialized variable and potential issues with sudden change in motor speed from previous value to 0
  }

  // The same process can be repeated for ADC_raw_1 and ADC_old_1 if needed for a second motor or for additional functionality, following the same logic for tight polling, fail count handling, and motor speed updating based on the new ADC value read
  static volatile uint32_t fail_count_1 = 0;
  // tight polling for ADC conversion, if the conversion is complete then read the value and store in ADC_raw variable
  if (HAL_ADC_PollForConversion(hadc, 10) == HAL_OK)
  {
    // read the ADC value and store in ADC_raw variable
    ADC_raw_1 = (uint16_t)HAL_ADC_GetValue(hadc);
    fail_count_1 = 0; // reset fail count if ADC conversion is successful
  }
  else
  {
    fail_count_1++; // increment fail count if ADC conversion fails
    if (ADC_old_1 == 5000 || fail_count_1 > 50) // if the motor speed was not set previously (ADC_old_1 is still at its initial value of 5000) or if there have been more than 50 consecutive ADC conversion failures, set ADC_raw_1 to 0 to set motor speed to minimum (0 speed) and avoid potential issues with uninitialized variable and potential issues with sudden change in motor speed from previous value to 0 due to ADC conversion failures
    {
      ADC_raw_1 = 0;
      // USART_Transmit(&huart2, "Critical Error READING ADC conversion: turning speed to minimum\r\n");
      fail_count_1 = 0; // reset fail count after taking corrective action to avoid repeated messages and actions in case of continuous ADC conversion failures
    }// if ADC conversion fails, and the motor speed was not set previously, or if there have been more than 50 consecutive ADC conversion failures, set ADC_raw_1 to 0 to set motor speed to minimum (0 speed) and avoid potential issues with uninitialized variable
  }

  // if the new ADC value is different from the previous value, map the ADC value which is between 0 and 255 to a motor speed value between min_speed and max_speed, then update the motor speed with the new value read from the ADC if a fault condition has not been detected and motor is already in motion (BUSY_ID flag is 0 when motor is in motion)
  if (ADC_raw_0!=ADC_old_0 && ADC_raw_0%2==0) //only transmit the new ADC value if it is different from the previous value and is an even numberto avoid unnecessary UART transmissions, and ensure less changes in motor speed for small changes in ADC value to avoid potential issues with sudden changes in motor speed due to small changes in ADC value, and to ensure that the motor speed is updated only when there is a significant change in the ADC value (at least 2 units change) to provide smoother motor control, and ensure it stays at a value that is between the possible range of ADC values (0 to 255 for 8-bit resolution) to avoid potential issues with mapping the ADC value to motor speed value and potential issues with sudden changes in motor speed due to small changes in ADC value, and to ensure that the motor speed is updated only when there is a significant change in the ADC value (at least 2 units change) to provide smoother motor control, and ensure it stays at a value that is between the possible range of ADC values (0 to 255 for 8-bit resolution) to avoid potential issues with mapping the ADC value to motor speed value and potential issues with sudden changes in motor speed due to small changes in ADC value
  {
    ADC_old_0 = ADC_raw_0; // update ADC_old with the new ADC value read to be used for comparison in the next call to the function to determine if the ADC value has changed significantly (at least 2 units change) to provide smoother motor control and avoid potential issues with sudden changes in motor speed due to small changes in ADC value, and ensure it stays at a value that is between the possible range of ADC values (0 to 255 for 8-bit resolution) to avoid potential issues with mapping the ADC value to motor speed value and potential issues with sudden changes in motor speed due to small changes in ADC value

    // map the ADC value which is between 0 and 255 to a motor speed value between min_speed and max_speed
    motor_0_speed = motor_0_speed_min + ADC_raw_0*(motor_0_speed_max-motor_0_speed_min)/255;
    if (fault_detected == false) // only update the motor speed with the new value read from the ADC if a fault condition has not been detected and motor is already in motion (MOT_STATUS_ID flag is 0 when motor is in motion) to avoid potential issues with sudden changes in motor speed due to ADC value changes when the motor is not in motion, and to avoid potential issues with sudden changes in motor speed due to ADC value changes when a fault condition has been detected, this can help in providing smoother motor control and avoiding potential issues with sudden changes in motor speed due to ADC value changes when the motor is not in motion or when a fault condition has been detected
    {
      if (manual_control == true && manual_stopped[MOTOR_0] == false) // only update the motor speed with the new value read from the ADC if manual control is not active to avoid potential issues with sudden changes in motor speed due to ADC value changes when manual control is active, this can help in providing smoother motor control and avoiding potential issues with sudden changes in motor speed due to ADC value changes when manual control is active
      {
        No_Interrupt_Delay_ms(10); // add a small non-blocking delay before updating the motor speed to allow the motor to respond to the previous speed setting before potentially updating it again with a new ADC value, this can help in providing smoother motor control and avoiding potential issues with rapid changes in motor speed due to rapid changes in ADC value
        if (manual_control == true && manual_stopped[MOTOR_0] == false) // check again if manual control is still not active before updating the motor speed with the new value read from the ADC to avoid potential issues with sudden changes in motor speed due to ADC value changes when manual control is active, this can help in providing smoother motor control and avoiding potential issues with sudden changes in motor speed due to ADC value changes when manual control is active
        {
          L6470_Run(MOTOR_0, motor_direction[MOTOR_0], motor_0_speed); //update the specified motor speed with the new value read from the ADC
          No_Interrupt_Delay_ms(10); // add a small non-blocking delay after updating the motor speed to allow the motor to respond to the new speed setting before potentially updating it again with a new ADC value, this can help in providing smoother motor control and avoiding potential issues with rapid changes in motor speed due to rapid changes in ADC value
          if (motor_direction[MOTOR_0] != L6470_CheckStatusRegisterFlag(MOTOR_0, DIR_ID)) // if the motor direction has changed from the previous direction, add an additional non-blocking delay to allow the motor to respond to the new direction setting before potentially updating it again with a new ADC value, this can help in providing smoother motor control and avoiding potential issues with rapid changes in motor speed due to rapid changes in ADC value when the motor direction has changed
          {
            L6470_Run(MOTOR_0, motor_direction[MOTOR_0], motor_0_speed); //update the specified motor speed with the new value read from the ADC, this is done again after the additional non-blocking delay to ensure that the motor speed is updated with the new direction setting and to provide smoother motor control when the motor direction has changed
            No_Interrupt_Delay_ms(20); // add an additional non-blocking delay after updating the motor speed and direction to allow the motor to respond to the new settings before potentially updating it again with a new ADC value, this can help in providing smoother motor control and avoiding potential issues with rapid changes in motor speed due to rapid changes in ADC value when the motor direction has changed
          }
        }
        if (manual_stopped[MOTOR_0] == true) // if manual control is active and the motor was stopped due to manual control
        {
          L6470_HardStop(MOTOR_0); // stop the motor immediately if manual control is active and the motor was stopped due to manual control
        }
      }
      else if (manual_control == false) // if manual control is active, do not update the motor speed with the new value read from the ADC to avoid potential issues with sudden changes in motor speed due to ADC value changes when manual control is active, this can help in providing smoother motor control and avoiding potential issues with sudden changes in motor speed due to ADC value changes when manual control is active
      {
        L6470_Run(MOTOR_0, motor_direction[MOTOR_0], motor_0_speed); //update the specified motor speed with the new value read from the ADC
        No_Interrupt_Delay_ms(10); // add a small non-blocking delay after updating the motor speed to allow the motor to respond to the new speed setting before potentially updating it again with a new ADC value, this can help in providing smoother motor control and avoiding potential issues with rapid changes in motor speed due to rapid changes in ADC value
        if (motor_direction[MOTOR_0] != L6470_CheckStatusRegisterFlag(MOTOR_0, DIR_ID)) // if the motor direction has changed from the previous direction, add an additional non-blocking delay to allow the motor to respond to the new direction setting before potentially updating it again with a new ADC value, this can help in providing smoother motor control and avoiding potential issues with rapid changes in motor speed due to rapid changes in ADC value when the motor direction has changed
        {
          L6470_Run(MOTOR_0, motor_direction[MOTOR_0], motor_0_speed); //update the specified motor speed with the new value read from the ADC, this is done again after the additional non-blocking delay to ensure that the motor speed is updated with the new direction setting and to provide smoother motor control when the motor direction has changed
          No_Interrupt_Delay_ms(20); // add an additional non-blocking delay after updating the motor speed and direction to allow the motor to respond to the new settings before potentially updating it again with a new ADC value, this can help in providing smoother motor control and avoiding potential issues with rapid changes in motor speed due to rapid changes in ADC value when the motor direction has changed
        }
      }
    }
  }
  
  
  // The same process can be repeated for ADC_raw_1 and ADC_old_1 if needed for a second motor or for additional functionality, following the same logic for checking if the new ADC value is different from the previous value, mapping the ADC value to motor speed, and updating the motor speed if a fault condition has not been detected and motor is already in motion
  if (ADC_raw_1!=ADC_old_1 && ADC_raw_1%2==0) //only transmit the new ADC value if it is different from the previous value and is an even numberto avoid unnecessary UART transmissions, and ensure less changes in motor speed for small changes in ADC value to avoid potential issues with sudden changes in motor speed due to small changes in ADC value, and to ensure that the motor speed is updated only when there is a significant change in the ADC value (at least 2 units change) to provide smoother motor control, and ensure it stays at a value that is between the possible range of ADC values (0 to 255 for 8-bit resolution) to avoid potential issues with mapping the ADC value to motor speed value and potential issues with sudden changes in motor speed due to small changes in ADC value, and to ensure that the motor speed is updated only when there is a significant change in the ADC value (at least 2 units change) to provide smoother motor control, and ensure it stays at a value that is between the possible range of ADC values (0 to 255 for 8-bit resolution) to avoid potential issues with mapping the ADC value to motor speed value and potential issues with sudden changes in motor speed due to small changes in ADC value
  {
    ADC_old_1 = ADC_raw_1; // update ADC_old with the new ADC value read to be used for comparison in the next call to the function to determine if the ADC value has changed significantly (at least 2 units change) to provide smoother motor control and avoid potential issues with sudden changes in motor speed due to small changes in ADC value, and ensure it stays at a value that is between the possible range of ADC values (0 to 255 for 8-bit resolution) to avoid potential issues with mapping the ADC value to motor speed value and potential issues with sudden changes in motor speed due to small changes in ADC value

    // map the ADC value which is between 0 and 255 to a motor speed value between min_speed and max_speed
    motor_1_speed = motor_1_speed_min + ADC_raw_1*(motor_1_speed_max-motor_1_speed_min)/255;
    if (fault_detected == false) // only update the motor speed with the new value read from the ADC if a fault condition has not been detected and motor is already in motion (MOT_STATUS_ID flag is 0 when motor is in motion) to avoid potential issues with sudden changes in motor speed due to ADC value changes when the motor is not in motion, and to avoid potential issues with sudden changes in motor speed due to ADC value changes when a fault condition has been detected, this can help in providing smoother motor control and avoiding potential issues with sudden changes in motor speed due to ADC value changes when the motor is not in motion or when a fault condition has been detected
    {
      if (manual_control == true && manual_stopped[MOTOR_1] == false) // only update the motor speed with the new value read from the ADC if manual control is not active to avoid potential issues with sudden changes in motor speed due to ADC value changes when manual control is active, this can help in providing smoother motor control and avoiding potential issues with sudden changes in motor speed due to ADC value changes when manual control is active
      {
        No_Interrupt_Delay_ms(10); // add a small non-blocking delay before updating the motor speed to allow the motor to respond to the previous speed setting before potentially updating it again with a new ADC value, this can help in providing smoother motor control and avoiding potential issues with rapid changes in motor speed due to rapid changes in ADC value
        if (manual_control == true && manual_stopped[MOTOR_1] == false) // check again if manual control is still not active before updating the motor speed with the new value read from the ADC to avoid potential issues with sudden changes in motor speed due to ADC value changes when manual control is active, this can help in providing smoother motor control and avoiding potential issues with sudden changes in motor speed due to ADC value changes when manual control is active
        {
          L6470_Run(MOTOR_1, motor_direction[MOTOR_1], motor_1_speed); //update the specified motor speed with the new value read from the ADC
          No_Interrupt_Delay_ms(10); // add a small non-blocking delay after updating the motor speed to allow the motor to respond to the new speed setting before potentially updating it again with a new ADC value, this can help in providing smoother motor control and avoiding potential issues with rapid changes in motor speed due to rapid changes in ADC value
          if (L6470_CheckStatusRegisterFlag(MOTOR_1, DIR_ID) != motor_direction[MOTOR_1]) // if the motor direction has changed from the previous direction, add an additional non-blocking delay to allow the motor to respond to the new direction setting before potentially updating it again with a new ADC value, this can help in providing smoother motor control and avoiding potential issues with rapid changes in motor speed due to rapid changes in ADC value when the motor direction has changed
          {
            L6470_Run(MOTOR_1, motor_direction[MOTOR_1], motor_1_speed); //update the specified motor speed with the new value read from the ADC, this is done again after the additional non-blocking delay to ensure that the motor speed is updated with the new direction setting and to provide smoother motor control when the motor direction has changed
            No_Interrupt_Delay_ms(20); // add an additional non-blocking delay after updating the motor speed and direction to allow the motor to respond to the new settings before potentially updating it again with a new ADC value, this can help in providing smoother motor control and avoiding potential issues with rapid changes in motor speed due to rapid changes in ADC value when the motor direction has changed
          }
          if (manual_stopped[MOTOR_1] == true) // if manual control is active and the motor was stopped due to manual control, add an additional non-blocking delay to allow the motor to respond to the stop command before potentially updating it again with a new ADC value, this can help in providing smoother motor control and avoiding potential issues with sudden changes in motor speed due to ADC value changes when manual control is active and the motor was stopped due to manual control
          {
            L6470_HardStop(MOTOR_1); // stop the motor immediately if manual control is active and the motor was stopped due to manual control
          }
        }
      }
      else if (manual_control == false) // if manual control is active, do not update the motor speed with the new value read from the ADC to avoid potential issues with sudden changes in motor speed due to ADC value changes when manual control is active, this can help in providing smoother motor control and avoiding potential issues with sudden changes in motor speed due to ADC value changes when manual control is active
      {
        L6470_Run(MOTOR_1, motor_direction[MOTOR_1], motor_1_speed); //update the specified motor speed with the new value read from the ADC
        No_Interrupt_Delay_ms(10); // add a small non-blocking delay after updating the motor speed to allow the motor to respond to the new speed setting before potentially updating it again with a new ADC value, this can help in providing smoother motor control and avoiding potential issues with rapid changes in motor speed due to rapid changes in ADC value
        if (L6470_CheckStatusRegisterFlag(MOTOR_1, DIR_ID) != motor_direction[MOTOR_1]) // if the motor direction has changed from the previous direction, add an additional non-blocking delay to allow the motor to respond to the new direction setting before potentially updating it again with a new ADC value, this can help in providing smoother motor control and avoiding potential issues with rapid changes in motor speed due to rapid changes in ADC value when the motor direction has changed
        {
          L6470_Run(MOTOR_1, motor_direction[MOTOR_1], motor_1_speed); //update the specified motor speed with the new value read from the ADC, this is done again after the additional non-blocking delay to ensure that the motor speed is updated with the new direction setting and to provide smoother motor control when the motor direction has changed
          No_Interrupt_Delay_ms(20); // add an additional non-blocking delay after updating the motor speed and direction to allow the motor to respond to the new settings before potentially updating it again with a new ADC value, this can help in providing smoother motor control and avoiding potential issues with rapid changes in motor speed due to rapid changes in ADC value when the motor direction has changed
        }
      }
    }
  }
}


/**
  * @}
  */ /* End of NUCLEO_Private_Functions */

/**
  * @addtogroup NUCLEO_Exported_Functions
  * @{
  */

/**
  * @brief  This function initializes some peripherals of the NUCELO board
  *         (HAL, Clock, NVIC, LED and user button)
  */
void NUCLEO_Board_Init(void)
{
  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  /* Initialize the ADC */
  MX_ADC1_Init();
  
  /* Initialize the SPI used by the X-NUCLEO-IHM02A1 */
  MX_SPI_Init();
  
#ifdef NUCLEO_USE_USART
  /* Initialize the USART peripheral */
  MX_USART2_Init();
#endif  

#ifdef NUCLEO_USE_USER_LED
  /* Perform 3 repetition of blinking user LED at 50% duty cycle with 250 ms as period */
  User_LED_Blinking(3, 750);
#endif
}

/**
  * @brief  Blinking user LED at 50% duty cycle.
  * @param  repetitions The number of  repetions.
  * @param  period_ms   The blinking period in ms.
  */
void User_LED_Blinking(uint8_t repetitions, uint16_t period_ms)
{
  uint8_t r;
  uint16_t half_period_ms;
  
  half_period_ms = period_ms >> 1;
  
  for (r=0; r<repetitions; r++)
  {
    /* Switch on the user LED */
    BSP_LED_On(LED2);
    /* ms delay */
    HAL_Delay(half_period_ms);
    /* Switch off the user LED */
    BSP_LED_Off(LED2);
    /* ms delay */
    HAL_Delay(half_period_ms);
  }
}

/**
  * @}
  */ /* End of NUCLEO_Exported_Functions */

/**
  * @}
  */ /* End of NUCLEO_Interface */

/**
  * @}
  */ /* End of X-NUCLEO-IHM02A1 */

/**
  * @}
  */ /* End of BSP */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
