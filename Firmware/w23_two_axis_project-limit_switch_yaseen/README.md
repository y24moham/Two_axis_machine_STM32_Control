# W23_Two_Axis_Project
This repo contains the initial code base to get you started with your projects for MTE 325 this term. It has been tested with the Visual Studio Code IDE. Experienced users are welcome to use alternate development environments, however, the teaching team will not be able to assist you if you run into dev environment related issues.

## Uart Driver file Swap
While it is not something I like doing, and I certainly wouldn't encourage it in a long-term project where this change would likely be blown away by any package updates, you will need to swap one hal driver file to get the UART working. The stm32f4xx_hal_uart.c file in this repo will need to replace the file of the same name in your platformIO stm32 framework packages. The install location should be something similar to 
C:\...\.platformio\packages\framework-stm32cubef4\drivers\STM32F4xx_HAL_Driver\Src

If you work with other ST boards in VS code, you may want to make a backup of the original file and replace it when you are done with the project.

## Authors and acknowledgment
The source code for this project was adapted from the Microsteping Motor example for the Nucleo F401RE board provided by STM.
