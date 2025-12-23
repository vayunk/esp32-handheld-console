#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- CONFIGURATION ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 

// Pins from your Schematic
#define PIN_SDA 8
#define PIN_SCL 9
#define BTN_JUMP 14 // Key 2
#define BTN_DUCK 13 // Key 1

// Game Physics & Dimensions
#define DINO_X 10
#define GROUND_Y (SCREEN_HEIGHT - 10)
#define DINO_W 10
#define DINO_H 10
#define DINO_DUCK_H 5

// Acceleration Settings
#define START_SPEED 3.0
#define MAX_SPEED 8.0
#define ACCELERATION 0.002 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Game Variables
int dinoY = GROUND_Y - DINO_H;
float velocityY = 0;
float gravity = 0.6;
float jumpStrength = -6.5; 

// Obstacle Variables
float obstacleX = SCREEN_WIDTH;
int obstacleY = GROUND_Y - 12; // Y position of current obstacle
int obstacleW = 6;             // Width of current obstacle
int obstacleH = 12;            // Height of current obstacle

float gameSpeed = START_SPEED;
int score = 0;
bool gameOver = false;

void setup() {
  Serial.begin(115200);

  // Initialize Custom I2C Pins
  Wire.begin(PIN_SDA, PIN_SCL);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  // Initialize Buttons
  pinMode(BTN_JUMP, INPUT);
  pinMode(BTN_DUCK, INPUT);

  // Start Screen
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(5, 20);
  display.println("PRESS KEY1 TO START");
  display.display();
  
  // Wait for start
  while(digitalRead(BTN_JUMP) == LOW) { delay(10); }
}

void loop() {
  if (gameOver) {
    display.clearDisplay();
    display.setCursor(30, 20);
    display.println("GAME OVER");
    display.setCursor(30, 40);
    display.print("Score: "); display.println(score);
    display.display();
    
    // Restart on Jump Button
    if (digitalRead(BTN_JUMP) == HIGH) {
      gameOver = false;
      score = 0;
      obstacleX = SCREEN_WIDTH;
      dinoY = GROUND_Y - DINO_H;
      velocityY = 0;
      gameSpeed = START_SPEED;
      spawnObstacle(); // Reset first obstacle
      delay(500); 
    }
    return;
  }

  // --- INPUT HANDLING ---
  bool isDucking = (digitalRead(BTN_DUCK) == HIGH);
  bool isJumping = (digitalRead(BTN_JUMP) == HIGH);

  // Jump Logic
  if (isJumping && dinoY >= GROUND_Y - DINO_H) {
     velocityY = jumpStrength;
  }

  // Physics (Gravity)
  dinoY += velocityY;
  velocityY += gravity;

  // Floor Collision
  if (dinoY >= GROUND_Y - DINO_H) {
    dinoY = GROUND_Y - DINO_H;
    velocityY = 0;
  }

  // --- MOVEMENT ---
  gameSpeed += ACCELERATION; 
  if (gameSpeed > MAX_SPEED) gameSpeed = MAX_SPEED;

  obstacleX -= gameSpeed;
  
  // Reset Obstacle
  if (obstacleX < -15) {
    obstacleX = SCREEN_WIDTH;
    score += 10;
    spawnObstacle(); // Pick new type/height
  }

  // --- DRAWING ---
  display.clearDisplay();

  // Draw Ground
  display.drawLine(0, GROUND_Y, SCREEN_WIDTH, GROUND_Y, WHITE);

  // Calculate Dino Dimensions for this frame
  int currentDinoH = isDucking ? DINO_DUCK_H : DINO_H;
  int currentDinoY = isDucking ? (dinoY + (DINO_H - DINO_DUCK_H)) : dinoY;
  
  // Draw Dino
  display.fillRect(DINO_X, currentDinoY, DINO_W, currentDinoH, WHITE);

  // Draw Obstacle
  display.fillRect((int)obstacleX, obstacleY, obstacleW, obstacleH, WHITE);

  // Draw Score
  display.setCursor(100, 0);
  display.print(score);

  // --- BOX COLLISION (AABB) ---
  // 1. Define Dino Box
  int dinoLeft = DINO_X;
  int dinoRight = DINO_X + DINO_W;
  int dinoTop = currentDinoY;
  int dinoBottom = currentDinoY + currentDinoH;

  // 2. Define Obstacle Box
  int obsLeft = (int)obstacleX;
  int obsRight = (int)obstacleX + obstacleW;
  int obsTop = obstacleY;
  int obsBottom = obstacleY + obstacleH;

  // 3. Check Overlap
  // If ALL these are true, the boxes are touching
  if (dinoRight > obsLeft && 
      dinoLeft < obsRight && 
      dinoBottom > obsTop && 
      dinoTop < obsBottom) {
       gameOver = true;
  }

  display.display();
  delay(10); 
}

// Function to randomly pick a new obstacle
void spawnObstacle() {
  int type = random(0, 100);

  if (type < 50) {
    // 50% Chance: CACTUS (Ground level, tall)
    obstacleW = 6; 
    obstacleH = 12;
    obstacleY = GROUND_Y - 12;
  } 
  else if (type < 75) {
    // 25% Chance: LOW BIRD (Must Jump)
    obstacleW = 10; 
    obstacleH = 6;
    obstacleY = GROUND_Y - 10; // Slightly off ground
  } 
  else {
    // 25% Chance: HIGH BIRD (Must Duck)
    obstacleW = 10; 
    obstacleH = 6;
    obstacleY = GROUND_Y - 22; // High enough to walk under if ducking
  }
}