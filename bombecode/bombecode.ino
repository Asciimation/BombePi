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

#include <avr/pgmspace.h>

// ENIGMA rotors.
// ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ
//char rotor1[54] = "EKMFLGDQVZNTOWYHXUSPAIBRCJ UWYGADFPVZBECKMTHXSLRINQOJ";
//char rotor2[54] = "AJDKSIRUXBLHWTMCQGZNPYFVOE AJPCZWRLFBDKOTYUQGENHXMIVS";
//char rotor3[54] = "BDFHJLCPRTXVZNYEIWGAKMUSQO TAGBPCSDQEUFVNZHYIXJWLRKOM";
//char rotor4[54] = "ESOVPZJAYQUIRHXLNFTGKDCMWB HZWVARTNLGUPXQCEJMBSKDYOIF";
//char rotor5[54] = "VZBRGITYUPSDNHLXAWMJQOFECK QCYLXWENFTZOSMVJUDKGIARPHB";

// Enigma reflectors.
//char reflectorB[27] = "YRUHQSLDPXNGOKMIEBFZCWVJAT";
//char reflectorC[27] = "FVPJIAOYEDRZXWGCTKUQSBNMHL";

// Set up Bombe drums.
const char drum1[54] PROGMEM = "AJDKSIRUXBLHWTMCQGZNPYFVOE AJPCZWRLFBDKOTYUQGENHXMIVS";
const char drum2[54] PROGMEM = "VZBRGITYUPSDNHLXAWMJQOFECK QCYLXWENFTZOSMVJUDKGIARPHB";
const char drum3[54] PROGMEM = "BDFHJLCPRTXVZNYEIWGAKMUSQO TAGBPCSDQEUFVNZHYIXJWLRKOM";
int drum1CoreOffset = 1;
int drum2CoreOffset = 3;
int drum3CoreOffset = 1;

// Set up Bombe reflector.
const char reflector[27] = "YRUHQSLDPXNGOKMIEBFZCWVJAT";

//const int ROTOR1COREOFFSET = 1;
//const int ROTOR2COREOFFSET = 1;
//const int ROTOR3COREOFFSET = 1;
//const int ROTOR4COREOFFSET = 2;
//const int ROTOR5COREOFFSET = 3;

// Drums.
int drums[] = {2, 5, 3};

// Scramblers.
const int numberOfScramblers = 13;
char scramblerOffsets[numberOfScramblers][4] = {
  "ZZK", 
  "ZZE", 
  "ZZF", 
  "ZZN", 
  "ZZM",
  "ZZG",
  "ZZP",
  "ZZB",
  "ZZJ",
  "ZZI",
  "ZZL",
  "ZZO",
  "ZZA"
};

const char scramblerConnections[numberOfScramblers][3] PROGMEM = {
  "UE",
  "EG",
  "GR", 
  "RA",
  "AS",
  "SV",
  "EV",
  "EN",
  "HZ",
  "RZ",
  "GR",
  "GL",
  "SW"
};
 
// Menu letters.
const int numberMenuLetters = 12;
const char menuLetters[numberMenuLetters] PROGMEM = {'U', 'E', 'G', 'R', 'A', 'S', 'V', 'N', 'H', 'Z', 'L', 'W'};

// Menu connections.
const int MAXNUMBERCONNECTIONS = 4;
const int menuConnections[numberMenuLetters][MAXNUMBERCONNECTIONS] PROGMEM = {
  {1,0,0,0},
  {1,2,7,8},
  {2,3,11,12},
  {3,4,10,11},
  {4,5,0,0},
  {5,6,13,0},
  {6,7,0,0},
  {8,0,0,0},
  {9,0,0,0},
  {9,10,0,0},
  {12,0,0,0},
  {13,0,0,0}
};

// Test input voltage.
char inputLetter = 'A';

// Test register.
char testMenuLetter = 'G';

// Ring setting for all drums.
// 25 = Z.
const int RINGSETTING = 25;

//int diagonalBoard[26][26];
char diagonalBoardLetter = '\0';

// Initial indicator drum positions.
char indicatorDrums[4] = "ZZZ";

// Number of untraced voltages.
int untraced = 0;

// Number of stops.
int numStops = 0;

// Number of iterations.
int numIterations = 0;

// Debugging.
bool debugDiagonal = false;
bool debugOther = true;
bool debugEnigma = false;
bool jumbo = false;

// ----------------------------------------------------------------------------
// Setup.
// ----------------------------------------------------------------------------
void setup() 
{
  Serial.begin(9600); 
}

// ----------------------------------------------------------------------------
// Print the setup data to the screen.
// ----------------------------------------------------------------------------
void PrintSetupData()
{
  Serial.print("Top drum: "); 
  Serial.println(drums[0]);
  Serial.print("Middle drum: "); 
  Serial.println(drums[1]);
  Serial.print("Bottom drum: "); 
  Serial.println(drums[2]);

  if (reflector[0] == 'Y')
  {
    Serial.println("Reflector: B");
  }
  if (reflector[0] == 'F')
  {
    Serial.println("Reflector: C");
  }

  Serial.print("Test register: ");
  Serial.println(testMenuLetter);
  Serial.print("Test voltage: ");
  Serial.println(inputLetter);
  Serial.print("Number of scramblers: "); 
  Serial.println(numberOfScramblers);
  PrintScramblers();
  Serial.print("Number of menu letters: "); 
  Serial.println(numberMenuLetters);
  Serial.println("Menu connections:");
  for (int i = 0; i < numberMenuLetters; i++)
  {
    Serial.print(pgm_read_word_near(menuLetters[i]));
    Serial.print(" ");
    for (int j = 0; j < MAXNUMBERCONNECTIONS; j++)
    {
      if (pgm_read_word_near(menuConnections[i][j] != 0))
      {
        Serial.print(pgm_read_word_near(menuConnections[i][j]));
        Serial.print(" ");
      }
    }
    Serial.println();
  }
}

// ---------------------------------------------------------------------------
// Print out the scramblers.
// ----------------------------------------------------------------------------
void PrintScramblers()
{
  Serial.println("Scramblers:");
  for (int i = 0; i < numberOfScramblers; i++)
  {
    Serial.print(i + 1); 
    Serial.print(" "); 
    Serial.print(scramblerOffsets[i]); 
    Serial.print(" - ");
    Serial.print((const char*)pgm_read_word(scramblerConnections[i]));
    Serial.println();    
  }
}

// ----------------------------------------------------------------------------
// Pass a value through the given scrambler.
// ----------------------------------------------------------------------------
int Scrambler(int value, char *currentScrambler)
{
  const char *currentDrum;
  int currentDrumOffset = 0;
  int drumCoreOffset = 0;
  // Through drum 3.
  currentDrum = (const char*)pgm_read_byte_near(drum3);
  currentDrumOffset = currentScrambler[2] - 'A';
  drumCoreOffset = drum3CoreOffset;
  value = CalculateScramblerOffset(value, currentDrumOffset, drumCoreOffset);
  value = ForwardThroughScrambler(value, currentDrum, currentDrumOffset, drumCoreOffset);
  // Through drum 2.
  currentDrum = (const char*)pgm_read_byte_near(drum2);
  currentDrumOffset = currentScrambler[1] - 'A';
  drumCoreOffset = drum2CoreOffset;
  value = CalculateScramblerOffset(value, currentDrumOffset, drumCoreOffset);
  value = ForwardThroughScrambler(value, currentDrum, currentDrumOffset, drumCoreOffset);
  // Through drum 1.
  currentDrum = (const char*)pgm_read_byte_near(drum1);
  currentDrumOffset = currentScrambler[0] - 'A';
  drumCoreOffset = drum1CoreOffset;
  value = CalculateScramblerOffset(value, currentDrumOffset, drumCoreOffset);
  value = ForwardThroughScrambler(value, currentDrum, currentDrumOffset, drumCoreOffset);
  // Through reflector.
  value = (int)((const char*)pgm_read_byte_near(reflector[value-1])) - 64;
  value = WrapScramblerOffset(value);
  if (debugEnigma) Serial.println(value + 64);
  // Back through drum 1.
  currentDrum = (const char*)pgm_read_byte_near(drum1);
  currentDrumOffset = currentScrambler[0] - 'A';
  drumCoreOffset = drum1CoreOffset;
  value = CalculateScramblerOffset(value, currentDrumOffset, drumCoreOffset);
  value = BackwardThroughScrambler(value, currentDrum, currentDrumOffset, drumCoreOffset);
  // Back through drum 2.
  currentDrum = (const char*)pgm_read_byte_near(drum2);
  currentDrumOffset = currentScrambler[1] - 'A';
  drumCoreOffset = drum2CoreOffset;
  value = CalculateScramblerOffset(value, currentDrumOffset, drumCoreOffset);
  value = BackwardThroughScrambler(value, currentDrum, currentDrumOffset, drumCoreOffset);
  // Back through drum 3.
  currentDrum = (const char*)pgm_read_byte_near(drum3);
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
int ForwardThroughScrambler(int value, const char *currentDrum, int currentDrumOffset, int drumCoreOffset)
{
  int currentValue;
  currentValue = currentDrum[(value-1)] - 64;
  currentValue = currentValue - currentDrumOffset + RINGSETTING + drumCoreOffset;
  currentValue = WrapScramblerOffset(currentValue);
  if (debugEnigma) Serial.println(currentValue + 64);
  return currentValue;
}

// ----------------------------------------------------------------------------
// Pass the value backwards through the given scrambler.
// ----------------------------------------------------------------------------
int BackwardThroughScrambler(int value, const char *currentDrum, int currentDrumOffset, int drumCoreOffset)
{
  int currentValue;
  currentValue = currentDrum[(value-1) + 27] - 64;
  currentValue = currentValue - currentDrumOffset + RINGSETTING + drumCoreOffset;
  currentValue = WrapScramblerOffset(currentValue);
  if (debugEnigma) Serial.println(currentValue + 64);
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
  // The middle one iterates after the top one has done 26 turns.
  if (numIterations % 26 == 0)
  {
    // Middle drum.
    *(currentScrambler[0] + 1) = *(currentScrambler[0] + 1) + 1;
    if ((int)*(currentScrambler[0] + 1) > 'Z')
    {
      *(currentScrambler[0] + 1) = 'A';
    }
  }
  // The bottom drum iterates when we've done 26 * 26 turns.
  if (numIterations % 676 == 0)
  {
    // Bottom drum.
    *(currentScrambler[0] + 2) = *(currentScrambler[0] + 2) + 1;
    if ((int)*(currentScrambler[0] + 2) > 'Z')
    {
      *(currentScrambler[0] + 2) = 'A';
    }
  }
}

// ----------------------------------------------------------------------------
// Decrement the indicator.
// ----------------------------------------------------------------------------
void DecrementIndicator()
{
  // Top drum.
  indicatorDrums[0] = indicatorDrums[0] - 1;
  if ((int)indicatorDrums[0] < 'A')
  {
    indicatorDrums[0] = 'Z';
  }
  if ((int)indicatorDrums[0] != 'Z') return;
  // Middle drum.
  indicatorDrums[1] = indicatorDrums[1] - 1;
  if (indicatorDrums[1] < 'A')
  {
    indicatorDrums[1] = 'Z';
  }
  if (indicatorDrums[1] != 'Z') return;
  // Bottom drum.
  indicatorDrums[2] = indicatorDrums[2] - 1;
  if (indicatorDrums[2] < 'A')
  {
    indicatorDrums[2] = 'Z';
  }
  return;
}

// ----------------------------------------------------------------------------
// Reset the diagonal board to all zero.
// ----------------------------------------------------------------------------
void ResetDiagonalBoard(int diagonalBoard[26][26])
{
  if (debugDiagonal) Serial.println("Resetting diagonal board.");
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
void SetDiagonalBoard(int diagonalBoard[26][26], char menuLetter, char letter)
{
  if (debugOther) Serial.print("Setting diagonal: "); Serial.println(menuLetter, letter);
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
// Print out a scrambler corrected to match Enigma.
// ----------------------------------------------------------------------------
void PrintCorrectedScrambler( char *scrambler )
{
  char drum;
  int drumValue;
  drum = scrambler[0];
  drumValue = (int)drum - 64 - drum1CoreOffset;
  drumValue = WrapScramblerOffset(drumValue);
  Serial.println(drumValue + 64);
  drum = scrambler[1];
  drumValue = (int)drum - 64 - drum2CoreOffset;
  drumValue = WrapScramblerOffset(drumValue);
  Serial.println(drumValue + 64);
  drum = scrambler[2];
  drumValue = (int)drum - 64 - drum3CoreOffset;
  drumValue = WrapScramblerOffset(drumValue);
  Serial.println(drumValue + 64);
}

// ----------------------------------------------------------------------------
// Print out the diagonal board.
// ----------------------------------------------------------------------------
void PrintDiagonalBoard(int diagonalBoard[26][26])
{
  Serial.println("\n ");
  for (int i = 0; i < 26; i++)
  {
    Serial.println('A' + i);
  }
  Serial.println();
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
        Serial.println("?");
      }
      else
      {
        Serial.println(i + 'A');
      }
    }
    // Print the board.
    if (diagonalBoard[i][j] == -1 ) Serial.println(" x");
    else if (diagonalBoard[i][j] == 1 ) Serial.println(" |");
    else if (diagonalBoard[i][j] == 2 ) Serial.println(" o");
    else Serial.println(" ");
    }
    Serial.println();
  }
  Serial.println();
}

// ----------------------------------------------------------------------------
// Print out the test register.
// ----------------------------------------------------------------------------
void PrintTestRegister(int diagonalBoard[26][26])
{
  for (int i = 0; i < 26; i++)
  {
    Serial.println('A' + i);
  }
  Serial.println("\n");
  for (int i = 0; i < 26; i++)
  {
    if (diagonalBoard[int(testMenuLetter - 'A')][i] == 1)
    {
      Serial.println("|");
    }
    else
    {
      Serial.println();
    }
  }
  Serial.println();
}

// ----------------------------------------------------------------------------
// Check position function.
// ----------------------------------------------------------------------------
void CheckDrumPosition(int diagonalBoard[26][26])
{
  int numberVoltages = 0;
  bool stop = false;
  char potentialStecker = 0;
  // Clear the diagonal board.
  ResetDiagonalBoard(diagonalBoard);
  // Set up the initial letters.
  SetDiagonalBoard(diagonalBoard, testMenuLetter, inputLetter);
  // Next iteration.
  numIterations++;
  // Increment all the scramblers.
  IncrementScramblers();
  if (debugOther)
  {
    Serial.print("Iteration: "); Serial.println(numIterations);
    PrintScramblers();
  }
  // Decrement the indicator drums.
  DecrementIndicator();
  if (debugOther) Serial.print("Indicator: "); Serial.println(indicatorDrums);
  // Trace all voltages.
  Trace(diagonalBoard);
  // If we've traced all voltages check if we have a stop.
  numberVoltages = CheckRegister(diagonalBoard, testMenuLetter, &potentialStecker);
  // If not all letters have a voltage we have a potential stop.
  if (numberVoltages < 26)
  {
    // We might have a stop.
    stop = true;
  }
  else
  {
    stop = false;
  }
  // Is it a real stop.
  if (stop == true)
  {
    // If we are a Jumbo Bombe do the extra tests.
    if (jumbo)
    {
      if (debugOther) Serial.println("Potential stop. Doing Jumbo checks.");
      // If 25 were set check the one unset one for consistency.
      if (numberVoltages == 25)
      {
        stop = CheckStraight(diagonalBoard, potentialStecker);
        // If still consistent check the diagonal board.
        if (stop == true)
        {
        stop = CheckDiagonal(diagonalBoard, potentialStecker);
        }
      }
    }
    // If it's still a real stop report it.
    if (stop == true)
    {
      Serial.print(++numStops); Serial.print(indicatorDrums);
      PrintTestRegister(diagonalBoard);
    }
  }
}

// ----------------------------------------------------------------------------
// Check a potential straight.
// ----------------------------------------------------------------------------
bool CheckStraight(int diagonalBoard[26][26], char letter)
{
  int numberVoltages = 0;
  bool stop = true;
  char potentialStecker = '\0';
  // For each input letter.
  for (int i = 0; i < 26; i++)
  {
    // Clear the diagonal board.
    ResetDiagonalBoard(diagonalBoard);
    // Set up the initial letters.
    SetDiagonalBoard(diagonalBoard, testMenuLetter, i + 'A');
    // Trace all voltages.
    Trace(diagonalBoard);
    // If we've traced all voltages check it.
    numberVoltages = CheckRegister(diagonalBoard, testMenuLetter, &potentialStecker);
    if ((numberVoltages != 1) && (numberVoltages != 25))
    {
      // If more than one is set it's not a stop.
      stop = false;
      break;
    }
  }
  return stop;
}
// ----------------------------------------------------------------------------
// Check a register to see if we have traced all voltages.
// ----------------------------------------------------------------------------
int CheckRegister(int diagonalBoard[26][26], char letter, char *stecker)
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
// Check the diagonal board for inconsistencies.
// ----------------------------------------------------------------------------
bool CheckDiagonal(int diagonalBoard[26][26], char letter)
{
  bool stop = true;
  int numberSteckers = 0;
  // Clear the diagonal board.
  ResetDiagonalBoard(diagonalBoard);
  // Set up the initial letters.
  SetDiagonalBoard(diagonalBoard, testMenuLetter, letter);
  // Trace all voltages.
  Trace(diagonalBoard);
  // For each row of the diagonal board in the menu
  // check we don't have the same letter twice.
  for (int i = 0; i < 26; i++)
  {
    for (int j = 0; j < 26; j++)
    {
      if (diagonalBoard[i][j] != 0)
      {
        numberSteckers++;
        if (numberSteckers > 1)
        {
          // If it's more than one we have a inconsistency
          // and this is a false stop.
          stop = false;
          break;
          }
        }
      }
      // Did we find an inconsistency?
      if (numberSteckers > 1)
      {
        break;
      }
    // Reset the count.
    numberSteckers = 0;
  }
  return stop;
}

// ----------------------------------------------------------------------------
// Trace voltages.
// ----------------------------------------------------------------------------
void Trace(int diagonalBoard[26][26])
{
  // Loop until we've finished tracing every voltage.
  while (untraced)
  {
    // For each menu letter.
    for (int j = 0; j < numberMenuLetters; j++)
    {
      if (debugDiagonal) PrintDiagonalBoard(diagonalBoard);
      if (debugOther) Serial.print("Indicator: "); Serial.println(indicatorDrums);
      if (debugOther) Serial.print("Checking letter: "); Serial.println(menuLetters[j]);
      if (debugOther) Serial.print("Total untraced: "); Serial.println(untraced);
      // Trace the voltage through the menu.
      // For each input voltage on this menu letter.
      for (int k = 0; k < 26; k++)
      {
        if (diagonalBoard[menuLetters[j] - 'A'][k] == -1)
        {
          // If there is a -1 we trace it through.
          TraceMenuLetterVoltages(diagonalBoard, j, k);
        }
      }
      if (!untraced) break;
    }
  }
}

// ----------------------------------------------------------------------------
// Trace voltages for this menu letter.
// ----------------------------------------------------------------------------
void TraceMenuLetterVoltages(int diagonalBoard[26][26], int menuLetterIndex, int voltage)
{
  char *currentScrambler = 0;
  char *currentConnections = 0;
  char *currentScramblerLetters = 0;
  int output = 0;
  int connectionCount = 0;
  
  // Set this voltage as traced.
  diagonalBoard[menuLetters[menuLetterIndex] - 'A'][voltage] = 1;
  untraced--;
  if (debugOther) Serial.println("Tracing voltage on letter "); Serial.println(voltage + 'A');
  // For each connected scrambler.
  for (int i = 0; i < MAXNUMBERCONNECTIONS; i++)
  {
    // If we checked all scramblers we are done tracing.
    if (menuConnections[menuLetterIndex][i] == 0)
    {
      break;
    }
    // Else send the voltage through this scrambler.
    currentScrambler = &scramblerOffsets[ (pgm_read_byte_near(menuConnections[menuLetterIndex][i])) - 1 ][0];
    currentScramblerLetters = (char*)pgm_read_word(&scramblerConnections[ (pgm_read_byte_near(menuConnections[menuLetterIndex][i])) - 1 ][0]);
    if (debugOther)
    {
      Serial.println(menuConnections[menuLetterIndex][i]);
      PrintCorrectedScrambler(currentScrambler);
      Serial.println(voltage + 65);
    }
    output = Scrambler(voltage + 1, currentScrambler);
    if (debugOther)
    {
      Serial.println(output + 64);
    }
    // Set a -1 on the other end of each connected scrambler.
    char letter = (char)output + 64;
    for (int j = 0; j < 2; j++)
    {
      // Each scrambler is between two letters.
      if (scramblerConnections[(menuConnections[menuLetterIndex][i]) - 1][j] != menuLetters[menuLetterIndex])
      {
        // If its not this menu letter it's the opposite end.
        SetDiagonalBoard(diagonalBoard, scramblerConnections[(menuConnections[menuLetterIndex][i]) - 1][j], letter);
      }
    }
  }
}

// ----------------------------------------------------------------------------
// Main program.
// ----------------------------------------------------------------------------
void loop() 
{
  bool runComplete = false;
  int diagonalBoard[26][26];

  Serial.println("Doing Bombe setup...");
  
  PrintSetupData();

  Serial.println("Bombe running...");
  // Check positions.
  while (!runComplete)
  {
    // Check this position.
    CheckDrumPosition(diagonalBoard);
    // Has the run finished?
    if (strcmp(indicatorDrums, "ZZZ") == 0)
    {
      Serial.println("Bombe run finished.");
      runComplete = true;
      while(1);
    }
  }
}
