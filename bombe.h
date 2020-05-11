MIT License

Copyright (c) 2020 Simon Jansen 
https://www.asciimation.co.nz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

bool ReadSetupFile(const char *fileName);
void ReadInputVoltage(char* buffer);
void ReadTestRegister(char* buffer);
void ReadRotors(char* buffer);
void ReadReflector(char* buffer);
void ReadScramblers(char* buffer);
void ReadConnections(char* buffer, int connectionCount);
void SetupDrums();
char* SetDrumAndOffset(int rotor, int *offset);
void PrintSetupData();
int Scrambler(int value, char *currentScrambler);
int WrapScramblerOffset(int value);
int CalculateScramblerOffset(int value, int currentDrumOffset, int drumCoreOffset);
int ForwardThroughScrambler(int value, char *currentDrum, int currentDrumOffset, int drumCoreOffset);
int BackwardThroughScrambler(int value, char *currentDrum, int currentDrumOffset, int drumCoreOffset);
void IncrementScramblers();
void IncrementScrambler(char **currentScrambler);
void DecrementIndicator();
void DecrementCurrentIndicator();
void ResetDiagonalBoard();
void SetDiagonalBoard(char letter1, char letter2);
void PrintScramblers();
void PrintCorrectedScrambler(char *scrambler);
void PrintDiagonalBoard();
void PrintTestRegister();
void CheckDrumPosition(int iteration);
int CheckRegister(char letter, char *stecker);
void Trace();
void TraceMenuLetterVoltages(int menuLetterIndex, int voltage);
bool ReadButtons(bool *start, bool *stop);
void CheckButtons();
bool WaitForArduino();
int EndsWithMnu(char *string);