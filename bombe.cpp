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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <wiringPi.h>
#include <lcd.h>
#include "bombe.h"

#define LCD_E       10
#define LCD_RS      11
#define LCD_D4      6
#define LCD_D5      12
#define LCD_D6      13
#define LCD_D7      14

#define ARD_RESET   0
#define ARD_STOP    3

#define START_BUTTON    4
#define STOP_BUTTON    5

#define ARD_STEP    21
      
#define MAX_PATH    256
      
// ENIGMA rotors.
//                 ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ
char rotor1[54] = "EKMFLGDQVZNTOWYHXUSPAIBRCJ UWYGADFPVZBECKMTHXSLRINQOJ";
char rotor2[54] = "AJDKSIRUXBLHWTMCQGZNPYFVOE AJPCZWRLFBDKOTYUQGENHXMIVS";
char rotor3[54] = "BDFHJLCPRTXVZNYEIWGAKMUSQO TAGBPCSDQEUFVNZHYIXJWLRKOM";
char rotor4[54] = "ESOVPZJAYQUIRHXLNFTGKDCMWB HZWVARTNLGUPXQCEJMBSKDYOIF";
char rotor5[54] = "VZBRGITYUPSDNHLXAWMJQOFECK QCYLXWENFTZOSMVJUDKGIARPHB";

// Enigma reflectors.
char reflectorB[27] = "YRUHQSLDPXNGOKMIEBFZCWVJAT";
char reflectorC[27] = "FVPJIAOYEDRZXWGCTKUQSBNMHL";

// Set up Bombe drums.
char *drum1;
char *drum2;
char *drum3;

// Bombe drum offsets from Enigma rotors.
int drum1CoreOffset;
int drum2CoreOffset;
int drum3CoreOffset;
const int ROTOR1COREOFFSET = 1;
const int ROTOR2COREOFFSET = 1;
const int ROTOR3COREOFFSET = 1;
const int ROTOR4COREOFFSET = 2;
const int ROTOR5COREOFFSET = 3;

// Drums.
int drums[3];

// Menu connections.
const int MAXNUMBERCONNECTIONS = 6;
int menuConnections[36][MAXNUMBERCONNECTIONS];

// Scramblers.
int numberOfScramblers = 0;
char scramblerOffsets[36][4];
char scramblerConnections[36][3];

// Menu letters.
int numberMenuLetters = 0;
char menuLetters[36];

// Test input voltage.
char inputLetter;

// Test register.
char testMenuLetter;

// Ring setting for all drums.
// 25 = Z.
const int RINGSETTING = 25;

// Set up Bombe reflector.
char *reflector;

// Diagonal board.
int diagonalBoard[26][26];
char diagonalBoardLetter;

// Initial indicator drum positions.
char indicatorDrums[4] = "ZZZ";

// Current indicator drum positions.
char currentDrums[4] = "ZZZ";

// Number of untraced voltages.
int untraced = 0;

// Number of stops.
int numStops = 0;

// Number of iterations.
int numIterations = 0;

// Number of steps the Arduino has done.
int numArduinoSteps = 0;

// States.
bool allIterationsDone = false;
bool allArduinoStepsDone = false;
bool arduinoRunning = false;
bool stopFound = false;
bool runComplete = false;
bool displayingStop = false;

// Debugging.
bool debugDiagonal = false;
bool debugOther = false;
bool debugEnigma = false;
bool debugDrumPositions = false;

// LCD display.
int display = 0;

// ----------------------------------------------------------------------------
// Read the set up information from a .mnu file.
// ----------------------------------------------------------------------------
bool ReadSetupFile(const char *fileName)
{
	FILE *menuFile;
	const char *mode = "r";
	const int bufferSize = 280;
	char buffer[bufferSize];
	int lineCount = 0;

	// Open the file for reading.
	menuFile = fopen(fileName, mode);
	if (menuFile == NULL) 
	{
		return false;
	}

	printf("Menu file opened.\n");
	
	// Read each line.
	while (fgets(buffer, bufferSize, menuFile) != NULL)
	{	

        // Ignore comments.
        if ( buffer[0] != '*' )
        {
            
            lineCount++;

            if (lineCount == 1)
            {
                ReadRotors(buffer);
            }
            else if (lineCount == 2)
            {
                ReadReflector(buffer);
            }
            else if (lineCount == 3)
            {
                ReadTestRegister(buffer);
            }
            else if (lineCount == 4)
            {
                ReadInputVoltage(buffer); 
            }
            else if (lineCount == 5)
            {
                ReadScramblers(buffer);
            }
            else if (lineCount > 6)
            {
                ReadConnections(buffer, lineCount - 7);
            }		
        }
    }

	// Close the file.
	fclose(menuFile);

	return true;
}

// ----------------------------------------------------------------------------
// Read test register.
// ----------------------------------------------------------------------------
void ReadTestRegister(char* buffer)
{
	testMenuLetter = buffer[15];
}

// ----------------------------------------------------------------------------
// Read input voltage.
// ----------------------------------------------------------------------------
void ReadInputVoltage(char* buffer)
{
	inputLetter = buffer[15];
}

// ----------------------------------------------------------------------------
// Read rotors.
// ----------------------------------------------------------------------------
void ReadRotors(char* buffer)
{
	drums[0] = buffer[8] - '0';
	drums[1] = buffer[11] - '0';
	drums[2] = buffer[14] - '0';
}

// ----------------------------------------------------------------------------
// Read reflector.
// ----------------------------------------------------------------------------
void ReadReflector(char* buffer)
{
	if (buffer[11] == 'B')
	{
		reflector = reflectorB;
	}
	if (buffer[11] == 'C')
	{
		reflector = reflectorC;
	}
}

// ----------------------------------------------------------------------------
// Read scramblers.
// ----------------------------------------------------------------------------
void ReadScramblers(char* buffer)
{
	// Work out how many scramblers are being used.
	numberOfScramblers = ((strlen(buffer) - 7) / 5) + 1;

	// Copy the scramblers.
	for (int i = 0; i < numberOfScramblers; i++)
	{
		strncpy(&scramblerOffsets[i][0], &buffer[7+i*5], 3);
	}
}

// ----------------------------------------------------------------------------
// Read connections.
// ----------------------------------------------------------------------------
void ReadConnections(char* buffer, int connectionCount)
{
	int tokenCount = 0;
	const char delim[2] = ",";
	char *token;
	int tokenInt = 0;
	char menuLetter = '\0';

	// Set the menu letter.
	menuLetter = buffer[0];
	menuLetters[connectionCount] = menuLetter;
	numberMenuLetters++;

	// Get the first token.
	token = strtok(&buffer[3], delim);

	// Loop through the tokens.
	while (token != NULL)
	{		
		tokenCount++;
		// Remove the 'i' or 'o'.
		token[strlen(token) - 1] = '\0';
		// Store the connection.
		tokenInt = atoi(token);
		menuConnections[connectionCount][tokenCount - 1] = tokenInt;
		// Store the menu letter against the correct scrambler.
		strncat(&scramblerConnections[tokenInt - 1][0], &menuLetter, 1);	
		// Get the next token.
		token = strtok(NULL, delim);
	}
}

// ----------------------------------------------------------------------------
// Setup the drums.
// ----------------------------------------------------------------------------
void SetupDrums()
{
	drum1 = SetDrumAndOffset(drums[0], &drum1CoreOffset);
	drum2 = SetDrumAndOffset(drums[1], &drum2CoreOffset);
	drum3 = SetDrumAndOffset(drums[2], &drum3CoreOffset);
}
// ----------------------------------------------------------------------------
// Set the correct drum and core wiring offset.
// ----------------------------------------------------------------------------
char* SetDrumAndOffset(int rotor, int *offset)
{
	char *drum;

	switch (rotor)
	{
	case 1:
	{
		drum = rotor1;
		*offset = ROTOR1COREOFFSET;
		break;
	}
	case 2:
	{
		drum = rotor2;
		*offset = ROTOR2COREOFFSET;
		break;
	}
	case 3:
	{
		drum = rotor3;
		*offset = ROTOR3COREOFFSET;
		break;
	}
	case 4:
	{
		drum = rotor4;
		*offset = ROTOR4COREOFFSET;
		break;
	}
	case 5:
	{
		drum = rotor5;
		*offset = ROTOR5COREOFFSET;
		break;
	}
	default:
		printf("Unknown rotor specified.\n");
	}
	return drum;
}

// ----------------------------------------------------------------------------
// Print the setup data to the screen.
// ----------------------------------------------------------------------------
void PrintSetupData()
{
	printf("Top drum: %i\n", drums[0]);
	printf("Middle drum: %i\n", drums[1]);
	printf("Bottom drum: %i\n", drums[2]);
	if (reflector[0] == 'Y')
	{
		printf("Reflector: B\n");
	}
	if (reflector[0] == 'F')
	{
		printf("Reflector: C\n");
	}
	printf("Test register: %c\n", testMenuLetter);
	printf("Test voltage: %c\n", inputLetter);
	printf("Number of scramblers: %i\n", numberOfScramblers - 1);
	PrintScramblers();
	printf("Number of menu letters: %i\n", numberMenuLetters);
	printf("Menu connections:\n");
	for (int i = 0; i < numberMenuLetters; i++)
	{
		printf("%c:", menuLetters[i]);
		for (int j = 0; j < MAXNUMBERCONNECTIONS; j++)
		{
			if (menuConnections[i][j] != 0)
			{
				printf("%3i ", menuConnections[i][j]);
			}
		}
		printf("\n");
	}
}

// ----------------------------------------------------------------------------
// Pass a value through the given scrambler.
// ----------------------------------------------------------------------------
int Scrambler(int value, char *currentScrambler)
{
	char *currentDrum;
	int currentDrumOffset = 0;
	int drumCoreOffset = 0;

	// Through drum 3.
	currentDrum = drum3;
	currentDrumOffset = currentScrambler[2] - 'A';
	drumCoreOffset = drum3CoreOffset;
	value = CalculateScramblerOffset(value, currentDrumOffset, drumCoreOffset);
	value = ForwardThroughScrambler(value, currentDrum, currentDrumOffset, drumCoreOffset);

	// Through drum 2.
	currentDrum = drum2;
	currentDrumOffset = currentScrambler[1] - 'A';
	drumCoreOffset = drum2CoreOffset;
	value = CalculateScramblerOffset(value, currentDrumOffset, drumCoreOffset);
	value = ForwardThroughScrambler(value, currentDrum, currentDrumOffset, drumCoreOffset);

	// Through drum 1.
	currentDrum = drum1;
	currentDrumOffset = currentScrambler[0] - 'A';
	drumCoreOffset = drum1CoreOffset;
	value = CalculateScramblerOffset(value, currentDrumOffset, drumCoreOffset);
	value = ForwardThroughScrambler(value, currentDrum, currentDrumOffset, drumCoreOffset);

	// Through reflector.
	value = reflector[value-1] - 64;
	value = WrapScramblerOffset(value);
	if (debugEnigma) printf("%c", value + 64);

	// Back through drum 1.
	currentDrum = drum1;
	currentDrumOffset = currentScrambler[0] - 'A';
	drumCoreOffset = drum1CoreOffset;
	value = CalculateScramblerOffset(value, currentDrumOffset, drumCoreOffset);
	value = BackwardThroughScrambler(value, currentDrum, currentDrumOffset, drumCoreOffset);

	// Back through drum 2.
	currentDrum = drum2;
	currentDrumOffset = currentScrambler[1] - 'A';
	drumCoreOffset = drum2CoreOffset;
	value = CalculateScramblerOffset(value, currentDrumOffset, drumCoreOffset);
	value = BackwardThroughScrambler(value, currentDrum, currentDrumOffset, drumCoreOffset);

	// Back through drum 3.
	currentDrum = drum3;
	currentDrumOffset = currentScrambler[2] - 'A';
	drumCoreOffset = drum3CoreOffset;
	value = CalculateScramblerOffset(value, currentDrumOffset, drumCoreOffset);
	value = BackwardThroughScrambler(value, currentDrum, currentDrumOffset, drumCoreOffset);

	return value;
}

// ----------------------------------------------------------------------------
// Wrap the scrambler value so it is in the range 1-26.
// ----------------------------------------------------------------------------
int WrapScramblerOffset(int value)
{
	while (value < 1)
	{
		value = value + 26;
	}
	while (value > 26)
	{
		value = value - 26;
	}
	return value;
}

// ----------------------------------------------------------------------------
// Calculate the current scrambler offset.
// We take the current drum offset, the ring value (Z) and the Bombe core
// wiring offset into account.
// ----------------------------------------------------------------------------
int CalculateScramblerOffset(int value, int currentDrumOffset, int drumCoreOffset)
{
	int returnValue;
	returnValue = value + currentDrumOffset - RINGSETTING - drumCoreOffset;
	returnValue = WrapScramblerOffset(returnValue);
	return returnValue;
}

// ----------------------------------------------------------------------------
// Pass the value forwards through the given scrambler.
// ----------------------------------------------------------------------------
int ForwardThroughScrambler(int value, char *currentDrum, int currentDrumOffset, int drumCoreOffset)
{
	int currentValue;
	currentValue = currentDrum[(value-1)] - 64;
	currentValue = currentValue - currentDrumOffset + RINGSETTING + drumCoreOffset;
	currentValue = WrapScramblerOffset(currentValue);
	if (debugEnigma) printf("%c", currentValue + 64);
	return currentValue;
}

// ----------------------------------------------------------------------------
// Pass the value backwards through the given scrambler.
// ----------------------------------------------------------------------------
int BackwardThroughScrambler(int value, char *currentDrum, int currentDrumOffset, int drumCoreOffset)
{
	int currentValue;
	currentValue = currentDrum[(value-1) + 27] - 64;
	currentValue = currentValue - currentDrumOffset + RINGSETTING + drumCoreOffset;
	currentValue = WrapScramblerOffset(currentValue);
	if (debugEnigma) printf("%c", currentValue + 64);
	return currentValue;
}

// ----------------------------------------------------------------------------
// Increment all the scramblers.
// ----------------------------------------------------------------------------
void IncrementScramblers()
{
	for (int i = 0; i < numberOfScramblers; i++)
	{
		char *currentScrambler = &scramblerOffsets[i][0];
		IncrementScrambler(&currentScrambler);
	}
}

// ----------------------------------------------------------------------------
// Increment the scrambler.
// ----------------------------------------------------------------------------
void IncrementScrambler(char **currentScrambler)
{
	// Always increment the top drum.
	// Top drum.
	*currentScrambler[0] = *currentScrambler[0] + 1;
	if ((int)*currentScrambler[0] > 'Z')
	{
		*currentScrambler[0] = 'A';
	}
	
	// The middle one increments after the top one has done 1 and a half turns
	// or 39 steps (26 + 13).
	if ((numIterations) % 39 == 0)
	{
		// Middle drum.
		*(currentScrambler[0] + 1) = *(currentScrambler[0] + 1) + 1;
		if ((int)*(currentScrambler[0] + 1) > 'Z')
		{
			*(currentScrambler[0] + 1) = 'A';						
		}		
	}
	
	// Bottom drum increments when the second drum has done one whole turn.
	if ((numIterations) % (39 * 26) == 0)
	*(currentScrambler[0] + 2) = *(currentScrambler[0] + 2) + 1;
	if ((int)*(currentScrambler[0] + 2) > 'Z')
	{
		*(currentScrambler[0] + 2) = 'A';
	}
}

// ----------------------------------------------------------------------------
// Decrement the indicator.
// ----------------------------------------------------------------------------
void DecrementIndicator()
{
	// Top indicator drum always decrements.
	indicatorDrums[0] = indicatorDrums[0] - 1;
	if ((int)indicatorDrums[0] < 'A')
	{
		indicatorDrums[0] = 'Z';
	}

	// The middle one decrements after the top one has done 1 and a half turns
	// or 39 steps (26 + 13).
	if ((numIterations) % 39 == 0)
	{
		indicatorDrums[1] = indicatorDrums[1] - 1;
		if (indicatorDrums[1] < 'A')
		{
			indicatorDrums[1] = 'Z';

			// The bottom one decrements if the middle one has just finished one turn.
			indicatorDrums[2] = indicatorDrums[2] - 1;
			if (indicatorDrums[2] < 'A')
			{
				indicatorDrums[2] = 'Z';
				// When this happens we are done.
				allIterationsDone = true;
                printf("\nAll stops found.\n");
                // Once we have a stop the Bombe is allowed to start.    
                if ( displayingStop == false )
                {
                    lcdClear(display);
                    lcdPosition(display,0,0);
                    lcdPuts(display, "All stops found.");                            
                    if ( arduinoRunning == false )
                    {
                        lcdPosition(display,0,1);
                        lcdPuts(display, "Press START to run.");            
                    }
                }
			}
		}
	}	
}

// ----------------------------------------------------------------------------
// Decrement the current indicator.
// ----------------------------------------------------------------------------
void DecrementCurrentIndicator()
{
	// Top indicator drum always decrements.
	currentDrums[0] = currentDrums[0] - 1;
	if ((int)currentDrums[0] < 'A')
	{
		currentDrums[0] = 'Z';
	}

	// The middle one decrements after the top one has done 1 and a half turns
	// or 39 steps (26 + 13).
	if ((numArduinoSteps) % 39 == 0)
	{
		currentDrums[1] = currentDrums[1] - 1;
		if (currentDrums[1] < 'A')
		{
			currentDrums[1] = 'Z';

			// The bottom one decrements if the middle one has just finished one turn.
			currentDrums[2] = currentDrums[2] - 1;
			if (currentDrums[2] < 'A')
			{
				currentDrums[2] = 'Z';
			}
		}
	}	
}
// ----------------------------------------------------------------------------
// Reset the diagonal board to all zero.
// ----------------------------------------------------------------------------
void ResetDiagonalBoard()
{
	if (debugDiagonal) printf("Resetting diagonal board.\n");
	// Set all voltages to 0.
	for (int i = 0; i < 26; i++)
	{
		for (int j = 0; j < 26; j++)
		{
			diagonalBoard[i][j] = 0;
		}
	}
	// Set the untraced count to 0.
	untraced = 0;
}

// ----------------------------------------------------------------------------
// Set voltages on the diagonal board.
// ----------------------------------------------------------------------------
void SetDiagonalBoard(char menuLetter, char letter)
{	
	if (debugOther) printf("Setting diagonal: %c:%c\n", menuLetter, letter);
	if (diagonalBoard[menuLetter - 'A'][letter - 'A'] == 0)
	{
		// Always update the menu letter.
		diagonalBoard[menuLetter - 'A'][letter - 'A'] = -1;
		// Update the count of untraced voltages.
		untraced++;	
	}

	// If it's not the same letter in the pair set the diagonal.
	if (menuLetter != letter)
	{
		if (diagonalBoard[letter - 'A'][menuLetter - 'A'] == 0)
		{
			// Update the untraced count if this is a menu letter.
			if (strchr(menuLetters, letter))
			{
				diagonalBoard[letter - 'A'][menuLetter - 'A'] = -1;
				untraced++;
			}
			else
			{
				diagonalBoard[letter - 'A'][menuLetter - 'A'] = 2;
			}
		}
	}
}

// ----------------------------------------------------------------------------
// Print out the scramblers.
// ----------------------------------------------------------------------------
void PrintScramblers()
{
	int scramblerNumber = 0;

	printf("Scramblers:\n");
	for (int i = 0; i < numberOfScramblers - 1; i++)
	{
		printf("%2i:%s-%s ", i+1, scramblerOffsets[i], scramblerConnections[i]);
		scramblerNumber++;
		if (scramblerNumber % 6 == 0)
		{
			printf("\n");
		}
	}
	printf("\n");
}

// ----------------------------------------------------------------------------
// Print out a scrambler corrected to match Enigma.
// ----------------------------------------------------------------------------
void PrintCorrectedScrambler( char *scrambler )
{
	char drum;
	int drumValue;

	drum = scrambler[0];
	drumValue = (int)drum - 64 - drum1CoreOffset;
	drumValue = WrapScramblerOffset(drumValue);
	printf("%c", drumValue + 64);
	drum = scrambler[1];
	drumValue = (int)drum - 64 - drum2CoreOffset;
	drumValue = WrapScramblerOffset(drumValue);
	printf("%c", drumValue + 64);
	drum = scrambler[2];
	drumValue = (int)drum - 64 - drum3CoreOffset;
	drumValue = WrapScramblerOffset(drumValue);
	printf("%c", drumValue + 64);
	
}
// ----------------------------------------------------------------------------
// Print out the diagonal board.
// ----------------------------------------------------------------------------
void PrintDiagonalBoard()
{
	printf("\n ");
	for (int i = 0; i < 26; i++)
	{
		printf("%2c", 'A' + i);
	}
	printf("\n");
	for (int i = 0; i < 26; i++)
	{
		for (int j = 0; j < 26; j++)
		{
			// Print the rows.
			if (j == 0)
			{
				// If this is the test register print a ?
				if (char(i + 'A') == testMenuLetter)
				{
					printf("?");
				}
				else
				{
					printf("%c", i + 'A');								
				}
			}
			// Print the board.
			if (diagonalBoard[i][j] == -1 ) printf(" x");
			else if (diagonalBoard[i][j] == 1 ) printf(" |");
			else if (diagonalBoard[i][j] == 2 ) printf(" o");
			else printf("  ");
		}
		printf("\n");
	}
	printf("\n");
}

// ----------------------------------------------------------------------------
// Print out the test register.
// ----------------------------------------------------------------------------
void PrintTestRegister()
{
	for (int i = 0; i < 26; i++)
	{
		printf("%c", 'A' + i);
	}
	printf("\n");
	for (int i = 0; i < 26; i++)
	{
		if (diagonalBoard[int(testMenuLetter - 'A')][i] == 1)
		{
			printf("%c", ' ');
		}
		else
		{
			printf("%c", '|');
		}
	}
	printf("\n");
}

// ----------------------------------------------------------------------------
// Check position function.
// ----------------------------------------------------------------------------
void CheckDrumPosition(int iteration)
{
	int numberVoltages = 0;
	char potentialStecker = 0;

	// Clear the diagonal board.
	ResetDiagonalBoard();

	// Set up the initial letters.
	SetDiagonalBoard(testMenuLetter, inputLetter);

	// Trace all voltages.
	Trace();

	// If we've traced all voltages check if we have a stop.
	numberVoltages = CheckRegister(testMenuLetter, &potentialStecker);

	// If not all letters have a voltage we have a stop.
	if (numberVoltages < 26)
	{	
        stopFound = true;
        printf("Stop %i found.\n", ++numStops);
        printf("Indicator: %s Iteration: %i\n", indicatorDrums, iteration);
        PrintTestRegister();                                               
        
        // Once we have a stop the Bombe is allowed to start.    
        if ( displayingStop == false )
        {            
            lcdPosition(display,0,1);
            lcdPuts(display, "Press START to run.");
        }
   
    }
}

// ----------------------------------------------------------------------------
// Check a register to see if we have traced all voltages.
// ----------------------------------------------------------------------------
int CheckRegister(char letter, char *stecker)
{
	int traceCount = 0;
	char potentialStecker1 = '\0';
	char potentialStecker25 = '\0';

	// If we've traced all voltages check for a stop.

	// Check each voltage.
	for (int i = 0; i < 26; i++)
	{
		// Decrement count for each set voltage.
		if (diagonalBoard[letter - 'A'][i] != 0)
		{
			traceCount++;
		}
	
		// Store the potential stecker.
		if (diagonalBoard[letter - 'A'][i] == 1)
		{
			// Either the single voltage set.
			potentialStecker1 = i + 'A';
		}
		else if (diagonalBoard[letter - 'A'][i] == 0)
		{
			// Or the single voltage unset.
			potentialStecker25 = i + 'A';
		}
	}	

	// Return the correct stecker.
	if (traceCount == 1)
	{
		// If one set return that one.
		*stecker = potentialStecker1;
	}
	else if (traceCount == 25)
	{
		// If 25 set return the one unset.
		*stecker = potentialStecker25;
	}
	else
	{
		// Else return null.
		*stecker = '\0';
	}

	return traceCount;
}

// ----------------------------------------------------------------------------
// Trace voltages.
// ----------------------------------------------------------------------------
void Trace()
{
	// Loop until we've finished tracing every voltage.
	while (untraced)
	{
		// For each menu letter.
		for (int j = 0; j < numberMenuLetters; j++)
		{
			if (debugDiagonal) PrintDiagonalBoard();		
			if (debugOther) printf("Indicator: %s\n", indicatorDrums);
			if (debugOther) printf("Checking letter: %c\n", menuLetters[j]);
			if (debugOther) printf("Total untraced: %i\n", untraced);

			// Trace the voltage through the menu.
			// For each input voltage on this menu letter.
			for (int k = 0; k < 26; k++)
			{
				if (diagonalBoard[menuLetters[j] - 'A'][k] == -1)
				{
					// If there is a -1 we trace it through.
					TraceMenuLetterVoltages(j, k);
				}
			}
			if (!untraced) break;
		}
	}
}

// ----------------------------------------------------------------------------
// Trace voltages for this menu letter.
// ----------------------------------------------------------------------------
void TraceMenuLetterVoltages(int menuLetterIndex, int voltage)
{
	char *currentScrambler = 0;
	char *currentConnections = 0;
	char *currentScramblerLetters = 0;
	int output = 0;
	int connectionCount = 0;

	// Set this voltage as traced.
	diagonalBoard[menuLetters[menuLetterIndex] - 'A'][voltage] = 1;
	untraced--;

	if (debugOther) printf("Tracing voltage on letter %c\n", voltage + 'A');
	// For each connected scrambler.
	for (int i = 0; i < MAXNUMBERCONNECTIONS; i++)
	{
		// If we checked all scramblers we are done tracing.
		if (menuConnections[menuLetterIndex][i] == 0)
		{
			break;
		}

		// Else send the voltage through this scrambler.
		currentScrambler = &scramblerOffsets[(menuConnections[menuLetterIndex][i]) - 1][0];
		currentScramblerLetters = &scramblerConnections[(menuConnections[menuLetterIndex][i]) - 1][0];
		if (debugOther)
		{
			printf("%2i:", menuConnections[menuLetterIndex][i]);
			PrintCorrectedScrambler(currentScrambler);
			printf(" %c>", voltage + 65);
		}
		output = Scrambler(voltage + 1, currentScrambler);
		if (debugOther)
		{
			printf("%c\n", output + 64);
		}

		// Set a -1 on the other end of each connected scrambler.
		char letter = (char)output + 64;		
		for (int j = 0; j < 2; j++)
		{
			// Each scrambler is between two letters.
			if (scramblerConnections[(menuConnections[menuLetterIndex][i]) - 1][j] != menuLetters[menuLetterIndex])
			{
				// If its not this menu letter it's the opposite end.
				SetDiagonalBoard(scramblerConnections[(menuConnections[menuLetterIndex][i]) - 1][j], letter);
			}
		}
	}
}

// ----------------------------------------------------------------------------
// Read button states.
// ----------------------------------------------------------------------------
bool ReadButtons(bool *start, bool *stop)
{
    bool buttonPressed = false;
    *start = false;
    *stop = false;
	if ( digitalRead(START_BUTTON) == 1 )
    {        
        *start = true;
        buttonPressed = true;
    }
    if ( digitalRead(STOP_BUTTON) == 1 )
    {        
        *stop = true;
        buttonPressed = true;
    }
    
    // If both buttons are pressed return immediately.
    // Otherwise wait till the button is released.
    if ( (*start == false) || (*stop == false) )
    {
        // Start button was pressed.
        if (*start == true)
        {
            // Check if the stop button is pressed also.
            if ( digitalRead(STOP_BUTTON) == 1 )                   
            {        
                *stop = true;
                return buttonPressed;
            }                                   
        }

        // Stop was pressed.
        if (*stop == true)
        {
            // Check if the start button is pressed also.
            if ( digitalRead(START_BUTTON) == 1 )                   
            {        
                *start = true;
                return buttonPressed;
            }                
        }
    }

    return buttonPressed;
}

// ----------------------------------------------------------------------------
// Check the buttons then decide what to do based on their state
// and the current state of the machine.
// ----------------------------------------------------------------------------
void CheckButtons()
{
    bool startButton = false;
    bool stopButton = false;

    // Which button was pressed?
    if (ReadButtons (&startButton, &stopButton) )
    {
        // If both buttons are pressed we exit to the command line
        // immediately.    
        if ( (startButton == true) && (stopButton == true) )
        {            
            // Stop the arduino running.
            digitalWrite(ARD_STOP, HIGH);                 
            // Set the Arduino to stopped.
            arduinoRunning = false;    
            printf("Exiting to command line...\n");
            lcdClear(display);            
            lcdPosition(display,0,0);
            lcdPuts(display, "Exiting to command line..."); 
            // Return 0 on exit.
            exit (0);             
        }
        
        // Start button.
        // If we are stopped, the Arduino isn't running, 
        // the run isn't complete and we have a stop we can start running.    
        if ( (startButton == true) 
          && (arduinoRunning == false) 
          && (runComplete == false) 
          && (stopFound == true) )
        {               
            printf("START button pressed.\n");                                
            // Reset the indicator.
            lcdClear(display);            
            lcdPosition(display,6,1);
            lcdPuts(display, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");                       
            // Start the arduino running.                
            digitalWrite(ARD_STOP, LOW);   
            arduinoRunning = true;                                      
        }
        
        // Start button.
        // If we are stopped, the Arduino isn't running, 
        // the run isn't complete and we don't have stop we need to wait
        // until the next stop is found.    
        if ( (startButton == true) 
          && (arduinoRunning == false) 
          && (runComplete == false)
          && (allIterationsDone == false)  
          && (stopFound == false) )
        {               
            lcdClear(display);            
            lcdPosition(display,0,0);
            lcdPuts(display, "Processing...");    
            printf("START button pressed. Processing...\n");                                
            displayingStop = false;                           
        }
        
        // Start button.
        // If we are stopped, the Arduino isn't running, 
        // and all iterations are done we can start running.    
        if ( (startButton == true) 
          && (arduinoRunning == false) 
          && (allIterationsDone == true)
          && (runComplete == false) )
        {               
            printf("START button pressed.\n");                                
            // Reset the indicator.
            lcdClear(display);
            lcdPosition(display,0,0);
            lcdPuts(display, "All stops found.");            
            lcdPosition(display,0,1);
            lcdPuts(display, "Running to completion.");                       
            // Start the arduino running.                
            digitalWrite(ARD_STOP, LOW);   
            arduinoRunning = true;                                      
        }
        
        // Stop button.
        // If we are running and not complete we stop.
        if ( (stopButton == true) && (arduinoRunning == true) && (runComplete == false) )
        {   
            // Stop the arduino running.
            digitalWrite(ARD_STOP, HIGH);                 
            // Set the Arduino to stopped.
            arduinoRunning = false;                                  
            
            printf("\nSTOP button pressed. Press START to run.\n");    
            lcdClear(display);
            lcdPosition(display,0,0);
            lcdPuts(display, "STOP button pressed.");  
            lcdPosition(display,0,1);
            lcdPuts(display, "Press START to run.");  
        } 

        // Stop button.
        // If we are finished running (so stopped) 
        // the stop button will shut down the Bombe down.
        if ( (stopButton == true) && (runComplete == true) )
        {                     
            printf("STOP button pressed. Shutting down.\n");      
            lcdClear(display);            
            lcdPosition(display,0,0);
            lcdPuts(display, "STOP button pressed. Shutting down."); 
            // Return 1 on exit.
            exit (1);                                         
        }
    }
}

// ----------------------------------------------------------------------------
// Count the pulses coming back from the Arduino if it is running
// until we reach the correct iteration. 
// ----------------------------------------------------------------------------
bool WaitForArduino()
{       
    // If the Arduino is running check for pulses.    
    if ( (arduinoRunning == true) && (numArduinoSteps < numIterations) )
    {   
        // Wait for the next pulse.
        while (digitalRead(ARD_STEP) == 0)              
        {
            // Do nothing.
        } 
        // Wait for it to change again.
        while (digitalRead(ARD_STEP) == 1)
        {
            // Do nothing.
        }                
        // We have a step.
        numArduinoSteps++;
        
        if (debugDrumPositions == true)
        {
            // Decrement the current indicator drums.
            DecrementCurrentIndicator();
            printf("%s\n", currentDrums);            
        }
        else
        {
            if ( (numArduinoSteps % 39) == 0 )
            {
                printf(".");
            }
        }            
    }
    
    // Have we caught up yet?
    if ( numArduinoSteps == numIterations )
    {
        return true;        
    }    
    else
    {
        return false;
    }
}

// ----------------------------------------------------------------------------
// Check if a given string (filename) ends in .mnu.
// ----------------------------------------------------------------------------     
int EndsWithMnu( char *string )
{
    string = strrchr(string, '.');

    if( string != NULL )
    {
        if ( strcmp(string, ".mnu") == 0)
        {
            return(1);
        }
    }
    return(0);
}
     
// ----------------------------------------------------------------------------
// Main program.
// ----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	bool fileRead = false;
    const char* menuDir = "../../../media/usb";
    static char menuFullFilePath[MAX_PATH];
    bool startButton = false;
    bool stopButton = false;    
    
    // Disable printf buffering.
    setbuf(stdout, NULL);

	// WiringPi set-up.        
    // GPIO 0 = Arduino reset.
    // GPIO 2 = Unused.
    // GPIO 3 = Arduino stop.
    // GPIO 4 = Switch 1.
    // GPIO 5 = Switch 2.    
    // GPIO 6 = LCD D4.
    // GPIO 10 = LCD E.
    // GPIO 11 = LCD RS.    
    // GPIO 12 = LCD D5.
    // GPIO 13 = LCD D6.
    // GPIO 14 = LCD D7.
    // GPIO 21 = Arduino step.
    wiringPiSetup();
        
    // Outputs.
    pinMode(ARD_RESET, OUTPUT);
	pinMode(ARD_STOP, OUTPUT);
    digitalWrite(ARD_RESET, LOW);
    digitalWrite(ARD_STOP, HIGH);

    // Reset Arduino.
    printf("Resetting Arduino...\n");	
    digitalWrite(ARD_RESET, HIGH);
    delayMicroseconds(200);
    digitalWrite(ARD_RESET, LOW);

    // Inputs.
    printf("Setting inputs...\n");
    pinMode(START_BUTTON, INPUT);
	pinMode(STOP_BUTTON, INPUT);
    pinMode(ARD_STEP, INPUT);
    
    // LCD initialisation.
    // Rows, Cols, Bits, RS, E, D0-D7.
    printf("Initialising LCD...\n");
    display = lcdInit (2, 40, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);
    lcdClear(display);
    lcdDisplay(display, 1);
    lcdCursor(display, 0);
    lcdCursorBlink(display, 0);
    lcdClear(display);                  
    
    // Look for menu files.
    DIR *dir;
    struct dirent *dirent;
    
    bool menuSelected = false;
    // We have to loop around displaying the files until one is selected.
    while (menuSelected == false)
    {
        dir = opendir(menuDir);
        // Read the directory.
        while ((dirent = readdir(dir)) != NULL)
        {
            // If it's a .mnu file show it.
            if ( EndsWithMnu(dirent->d_name) )
            {
               lcdClear(display);            
               lcdPosition(display,0,0);
               lcdPuts(display, "Press START to select or STOP to skip.");
               lcdPosition(display,0,1);
               lcdPuts(display, dirent->d_name);

               // Wait for button press.
               while (ReadButtons (&startButton, &stopButton) == false ){};           
               if ( startButton )
               {
                    //printf("START button pressed.\n");
                    menuSelected = true;
               }
               else
               {
                   //printf("STOP button pressed.\n");
               }
               // Delay.
               delay(100);
               // Wait for button release.
               while (ReadButtons (&startButton, &stopButton) == true ){};
               // If start button we exit and use this menu file. If not we go to the next file.
               if ( menuSelected )
               {
                    lcdClear(display);            
                    lcdPosition(display,0,0);
                    lcdPuts(display, "Using menu: ");                    
                    break;
               }               
            }
        }
        (void)closedir(dir);
    }
    
    // Read the selected menu file.
    sprintf(menuFullFilePath, "%s//%s", menuDir, dirent->d_name); 
    printf("Reading menu file: %s\n", menuFullFilePath);
    lcdClear(display); 
    lcdPosition(display,0,0);
    lcdPuts(display, "Reading menu file: ");
    lcdPuts(display, dirent->d_name);    	
    // Open and read the file.
    if (!(fileRead = ReadSetupFile(menuFullFilePath)))
	{
		printf("Could not open input menu file.\n");
        printf("Press STOP to shutdown.\n");		
        lcdClear(display);            
        lcdPosition(display,0,0);
        lcdPuts(display, "Could not open: ");
        lcdPuts(display, dirent->d_name);
        lcdPosition(display,0,1);
        lcdPuts(display, "Press STOP to shutdown.");
        // Check buttons and exit as appropriate.
        runComplete = true;
        while (1)
        {
            CheckButtons();
        }
	}
    
    // Set up the drums and print out the configuration.
	SetupDrums();
	PrintSetupData();

    // Wait until a stop is found then we can start.
    lcdClear(display);            
    lcdPosition(display,0,0);
    lcdPuts(display, "Menu loaded. Processing...");
    printf("Menu loaded.\n");    
    printf("Bombe finding next stop...\n");       
	arduinoRunning = false;
    
	// Main processing loop.
	while (1)
	{	
        // Check the buttons.
        CheckButtons();
        
        // If we are still calculating.
        if ( (allIterationsDone == false) && (stopFound == false) )    
        {                
            // Next iteration.
            numIterations++;                
            // Increment all the scramblers.	
            IncrementScramblers();                
            // Decrement the indicator drums.
            DecrementIndicator();
            // Don't check the half cycles.
            if ((numIterations % 39) >= 13)
            {	// Check this position.
                CheckDrumPosition(numIterations);
                if (debugOther)
                {
                    printf("Iteration: %i\tIndicator: %s\t%i *\n", numIterations, indicatorDrums, numIterations % 39);
                    PrintScramblers();
                }				
            }
            else
            {
                if (debugOther)
                {
                    printf("Iteration: %i\tIndicator: %s\t%i\n", numIterations, indicatorDrums, numIterations % 39);
                    PrintScramblers();
                }				
            }
        }

        // If a stop was found and the Arduino is running wait for it
        // to catch up.
        if ( (allIterationsDone == false) && (stopFound == true) )
        {
            
            // Wait for the Arduino to catch up.
            if ( WaitForArduino() )
            {                
                // Stop the Arduino.
                printf("\nStopping Arduino at iteration %i.\n", numArduinoSteps);
                digitalWrite(ARD_STOP, HIGH);
                // Set the states.
                arduinoRunning = false;
                stopFound = false;
                displayingStop = true;
                // Output to the LCD.
                lcdClear(display);            
                lcdPosition(display,6,1);
                lcdPuts(display, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
                lcdPosition(display,6,0);
                for (int i = 0; i < 26; i++)
                {
                    if (diagonalBoard[int(testMenuLetter - 'A')][i] == 1)
                    {
                        lcdPutchar(display, ' ');
                    }
                    else
                    {
                        lcdPutchar(display, '|');
                    }
                }
            }
        }                  
        
        // If we've found all stops and the Arduino is running wait for it
        // to finish.
        if ( (allIterationsDone == true) && (runComplete == false) )
        {   
            // We just need to wait for the Arduino to finish.                        
            if ( WaitForArduino() )
            {                            
                // Stop the Arduino.
                printf("Stopping Arduino at iteration %i.\n", numArduinoSteps);
                digitalWrite(ARD_STOP, HIGH);
                // Set the states.
                arduinoRunning = false;
                runComplete = true;	
                // Complete.
                printf("Bombe run complete.\n");
                printf("Press STOP to shutdown.\n");                  
                lcdClear(display);            
                lcdPosition(display,0,0);
                lcdPuts(display, "Bombe run complete.");   
                lcdPosition(display,0,1);
                lcdPuts(display, "Press STOP to shutdown.");
            }                
        }
    } // while (1)
	exit 0;
}

