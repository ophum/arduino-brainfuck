#include <Key.h>
#include <Keypad.h>

#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27, 16, 4);

// Keypad
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  { '+', '-', '<', '>' },
  { '[', ']', '.', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' },
};

byte rowPins[ROWS] = { 10, 9, 8, 7 };
byte colPins[COLS] = { 6, 5, 4, 3 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// lcd buf
char screen[2][17] = {};

void writeScreen(int row, int col, String s) {
  for (int i = 0; i < 16 && i < s.length(); i++) {
    screen[row][col + i] = s[i];
  }
}

void resetScreen() {
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 16; j++) {
      screen[i][j] = ' ';
    }
  }
}

void flushScreen() {
  for (int i = 0; i < 2; i++) {
    lcd.setCursor(0, i);
    lcd.print(screen[i]);
  }
}

// brainf**k
char program[1024] = {};
int programIndex = 0;

const byte MEMCOUNT = 3;
byte mem[MEMCOUNT] = {};
byte pointer = 0;

const byte OUTPUTSIZE = 5;
char output[OUTPUTSIZE] = "";
int outputIndex = 0;

int runIndex = 0;

const int DELAY = 0;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  writeScreen(0, 0, "START BRAINF**K");
  flushScreen();
  delay(1000);
  resetScreen();
  flushScreen();
  digitalWrite(2, HIGH);
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    switch (key) {
      case '#':
        run();
        return;
      case '*':
        reset();
        return;
      default:
        program[programIndex++] = key;
    }

    displayProgram();
  }
}

void displayProgram() {
  int windowLeft = 0;
  if (programIndex > 16) {
    windowLeft = programIndex - 16;
  }
  String s = "";
  for (int i = 0 + windowLeft; i < programIndex; i++) {
    s += program[i];
  }

  resetScreen();
  writeScreen(0, 0, s);
  writeScreen(1, 0, "LEN" + String(programIndex, DEC));
  flushScreen();
}

void run() {
  debug();
  for (runIndex; runIndex < programIndex; runIndex++) {
    switch (program[runIndex]) {
      case '+':
        mem[pointer]++;
        break;
      case '-':
        mem[pointer]--;
        break;
      case '<':
        pointer--;
        if (pointer == -1) {
          fatal();
        }
        break;
      case '>':
        pointer++;
        if (pointer == MEMCOUNT) {
          fatal();
        }
        break;
      case '.':
        output[outputIndex++] = char(mem[pointer]);
        break;
      case '[':
        break;
      case ']':
        if (mem[pointer] == 0) break;
        int count = 0;
        for (int j = runIndex; j >= 0; j--) {
          if (program[j] == ']') {
            count++;
          }
          if (program[j] == '[') {
            count--;
            if (count == 0) {
              runIndex = j;
              break;
            }
          }
        }
    }
    resetScreen();
    writeScreen(0, 0, displayMemoryString());
    writeScreen(1, 0, displayOutputString(i));
    flushScreen();
    delay(DELAY);
  }
}

String displayMemoryString() {
  String s = "";
  for (int i = 0; i < MEMCOUNT; i++) {
    s += String(i, DEC) + ":";
    if (mem[i] < 0x10) {
      s += "0";
    }
    s += String(mem[i], HEX) + " ";
  }
  return s;
}

String displayOutputString(int n) {
  String s = "";
  String nn = String(n, DEC);
  for (int i = 0; i < 4 - nn.length(); i++) {
    s += "0";
  }
  return s + String(n, DEC) + program[n] + "OUT> " + String(output);
}

void reset() {
  while (programIndex >= 0) {
    program[programIndex--] = 0;
  }
  programIndex = 0;
  runIndex = 0;
  for (int i = 0; i < MEMCOUNT; i++) {
    mem[i] = 0;
  }
  resetScreen();
  writeScreen(0, 0, "reset: done");
  flushScreen();
}

void fatal() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fatal");
  exit(1);
}

void debug() {
  for (int i = 0; i < MEMCOUNT; i++) {
    Serial.print("mem");
    Serial.print(i);
    Serial.print(":");
    Serial.print(mem[i]);
    Serial.print(" ");
  }
  Serial.println("");
  Serial.println("runIndex:" + String(runIndex, DEC));
}