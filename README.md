
# DSduino
DSduino transforms your Nintendo DS (NDS) to be a handheld computer which can manipulate sensors or control electronic items via BASIC language.

![DSduino](/img/4.jpeg)

## Intro
DSduino is one of my projects originally open sourced on SourceForge back in 2014, I'm now move it to my github for better maintainance.

DSduino is an NDS + Arduino + BASIC extension via slot 1 SPI interface. DSduino allows you to directly write BASIC programs on your NDS, such programs will be interpreted and executed on NDS via NDSBasic (included in this project). Additionally, these programs can be saved to NDS's flash card (e.g., R4 or SuperCard) or loaded from the card. Considering the functionalities of NDS (WIFI, 2 ARM CPUs, touch screen, 2 TFT screens, Microphone), DSduino can extend to a wide range of interesting applications.

## Requirement

- An NDS which can run homebrew programs (flashme, passme, etc)

- A flash cartriage, can be R4, SuperCard, etc. I used SuperCard.

## Hardware

Nintendo DS (Slot 1) <----SPI----> Arduino <----GPIO/I2C, etc----> Peripherals

The Arduino is a minimal system which does not require any electrical components. In this project, I used an Atmel 328p from an Arduino UNO board and reflashed the bootloader to allow it running with its internal oscillator under 3.3V rather than 5V, as NDS's SPI is running on 3.3V. Technically, you are totally fine to use any 3.3V Arduino board, e.g., Arduino mini.  

## Software

- NDSBasic, a tiny BASIC programming language interpreter I ported to NDS. I also added a few new commands in order to draw (lines, circles, rectangles) and read/write peripherals.

- Arduino firmware, in folder Arduino, this firmware makes the MCU to receive any commands from SPI (which is NDS) and interpret it and then send response back via SPI.

- libspi(modified), the libspi library is an open source library I adopted and modified in order to communicate bwteen NDS and Arduino via SPI. I have built the code into .a file, so you may use it directly in your project.

## Documents /  Articles

Back to 2014, I have written 7 articles to document the whole design and progress of the project and eventually won the Best Techincal Award in 2nd Atmel Open Source Competition. Check the doc folder for these articles that were also published on a Chinese Journal, called "无线电". However, all these articles were written in Chinese, and I don't have a plan to translate them into other languages. Well, you may use AI to do the translation for you as they do such work very well. Or, you may contact me via email too if you have any questions.

Note: most of the documents/articles are in .rtfd format, which is fine on Mac OS, so if you are on Windows or Linux, just open the TXT.rtf file in the folder.

## Demo Video
[Check this link for video demo](https://www.youtube.com/watch?v=gw4SA426inc). This is re-editted video with English captions and uploaded to youtube.

The original video for the aforementioned compitetion is
[Original video](https://v.youku.com/v_show/id_XNzg1ODg4NzEy.html?spm=a2hkm.8166622.PhoneSokuUgc_1.dscreenshot&playMode=pugv&frommaciku=1). Be aware of long and annoying ad.