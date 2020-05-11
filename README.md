# BombePi

This is the source code used in my desktop Turing/Welchman Bombe project.

The desktop Bombe makes use of a Raspberry Pi and an Arduino to create hybrid electromechanical version of the Turing/Welchman Bombe develped at Bletchly Park during WW2 to help break Enigma codes.

The original project is described here: [Turing/Welchman Bombe project](https://web.archive.org/web/20190816231400/http://www.asciimation.co.nz/bb/category/turing-welchman-bombe)

A YouTube film of the machine is available here: [turingwelchmanbombe](https://www.youtube.com/watch?v=TmlbNDreLDk)

This IS NOT a running project. It requires very custom, one-off hardware and used external libraries not included here. This code is provided purely for interested parties to examine. It is written in simple C++ and was compiled and run on the Raspberry Pi.

The menu file provided is the same one used at Bletchley Park to demonstrate their recreated Bombe.

Additionally the code in generic BASIC and an explaination is provided in the file [bombBASIC.txt](bombeBASIC.txt)
This BASIC code is written for a home made [6502 computer](https://web.archive.org/web/20190803190500/http://www.asciimation.co.nz/bb/category/6502-computer) using a modified version of the original MS BASIC. It should be similar enough to any generic 80s BASIC implementation to be easy enough to get running (very slowly)!

And a version of the code that will run on an Arduino is available here: [bombecode.ino](/bombecode/bombecode.ino)

## Authors

* **Simon Jansen** - [Asciimation](http://www.asciimation.co.nz)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* The original code makes use of WiringPi available here: [WiringPi](http://wiringpi.com/)

