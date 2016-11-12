# Nuvoton ISD9100/9160 BSP

The ISD9160 is a system-on-chip product optimized for low power, audio record and playback with an embedded ARM CortexTM-M0 32-bit microcontroller core.

The ISD9160 embeds a CortexTM-M0 core running up to 50 MHz with 145K-byte of non-volatile flash memory and 12K-byte of embedded SRAM. It also comes equipped with a variety of peripheral devices, such as Timers, Watchdog Timer (WDT), Real-time Clock (RTC), Peripheral Direct Memory Access (PDMA), a variety of serial interfaces (UART, SPI/SSP, I2C, I2S), PWM modulators, GPIO, Analog Comparator, Low Voltage Detector and Brown-out detector.

## Applications

### Voice Recognition

![](http://www.nuvoton.com/export/sites/nuvoton/images/ISD9160vR.png_1851307823.png)

> http://www.nuvoton.com/hq/applications/consumer/voice-recognition/?__locale=en

### Home Scurity Panel

![](http://www.nuvoton.com/export/sites/nuvoton/images/ISD9160-Security-Panel.png_622442009.png)

> http://www.nuvoton.com/hq/applications/consumer/security-panel/?__locale=en

## Hardware

### IC

![](http://www.digikey.com/-/media/Images/Article%20Library/TechZone%20Articles/2014/December/Application-Specific%20MCUs%20Targeting%20Emerging%20Applications/article-2014december-application-specific-mcus-fig1.jpg)

### Board

![](http://www.digikey.com/-/media/Images/Article%20Library/TechZone%20Articles/2014/December/Application-Specific%20MCUs%20Targeting%20Emerging%20Applications/article-2014december-application-specific-mcus-fig2.jpg)

> http://www.digikey.no/en/articles/techzone/2014/dec/application-specific-mcus-targeting-emerging-applications

## Setup

![](Docs/build_tools.png)

### BSP

- http://www.nuvoton.com/hq/resource-download.jsp?tp_GUID=SW0720140910133748

### Environment

- Keil MDK-ARM or IAR Embedded Workbench

![](Docs/build_steps.png)

### Driver

- Nu-Link Keil Driver: http://www.nuvoton.com/opencms/resource-download.jsp?tp_GUID=SW0520101208200142

  This driver is to support Nu-Link to work under Keil RVMDK Development Environment.

- Nu-Link IAR Driver: http://www.nuvoton.com/opencms/resource-download.jsp?tp_GUID=SW0520101208200227

  This driver is to support Nu-Link to work under IAR EWARM Development Environment.

### Programming

- ICP Programming Tool: http://www.nuvoton.com/opencms/resource-download.jsp?tp_GUID=SW0520101208200310
- Nu-Link Command Tool: http://www.nuvoton.com/opencms/resource-download.jsp?tp_GUID=SW0520160317094731

### Debug

- In-Circut Emulation Debug in Keil MDK-ARM with Nuvoton Nu-Link Debugger

  Load program on-line and run/stop/step/..., set breakpoings, check memory/variables/call stack/...

- Log trace with `printf`
  * To output log into UART, remove **DEBUG_ENABLE_SEMIHOST** define in **Project -> Options for Target -> C/C++ -> Preprocessor Symbols**

    UART0: TX - PIN PA.8 , RX - PIN PA.9

  * With **DEBUG_ENABLE_SEMIHOST** defined, check log in Keil MDK in debug mode at **View -> Serial Windows -> UART #1**

## Software

### Architecture

![](Docs/nuvoice_framework.png)

## Reference

- http://www.nuvoton.com/hq/products/application-specific-socs/arm-based-audio/aui-enablers-series/isd9160
- http://www.nuvoton.com/hq/resource-download.jsp?tp_GUID=UG0820130911063812
