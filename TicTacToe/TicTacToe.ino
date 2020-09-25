//-----------------------------------------------------
// Game "Tic Tac Toe" for Arduino
// Autor:   Joern Weise
// License: GNU GPl 3.0
// Created: 01. Aug 2020
// Update:  08. Aug 2020
//-----------------------------------------------------
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>

//Some defines for later
#define MINPRESSURE 100
#define MAXPRESSURE 1000
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// COPY-PASTE from Serial Terminal TouchScreen_Calibr_native
const int XP=8,XM=A2,YP=A3,YM=9; //240x320 ID=0x9341
const int TS_LEFT=116,TS_RT=915,TS_TOP=84,TS_BOT=894;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

extern uint8_t circle[];
extern uint8_t x_bitmap[];
int pixel_x, pixel_y;     //Touch_getXY() updates global vars
unsigned long previousMillis;
const long interval = 100;
Adafruit_GFX_Button btnEasy, btnHard;

int iSetMark[] = {0,0,0,0,0,0,0,0,0}; //Store the set marks from cpu and human. "0": Not set, "1": CPU, "2": Human
int iWinner = 0; //"0": Draw, "1": CPU, "2": Human
int iMoves = 1;
int iEnableButtons = 1;
bool bPressed = false;

//Returns if there is an touch-action on screen
//Overwrite X and y coordniate and set boolflag
bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed)
    { //Update mapping from calibration
      pixel_x = map(p.y, 88, 899, 0, 320); //.kbv makes sense to me
      pixel_y = map(p.x, 917, 122, 0, 240);
    }
    return pressed;
}

void setup() {
  Serial.begin(9600);
  iEnableButtons = 1;
  uint16_t ID = tft.readID();
  Serial.println("Tic Tac Toe AZ-Delivery by Joern Weise");
  Serial.print("TFT ID = 0x");
  Serial.println(ID, HEX);
  randomSeed(analogRead(0));
  tft.begin(ID);
  tft.setRotation(1); //PORTRAIT
  tft.fillScreen(BLACK);
  //Draw red frame
  drawFrame(10,RED);
  //Set first text
  tft.setCursor(30,40);
  tft.setTextColor(BLUE);
  tft.setTextSize(4);
  tft.print("Tic ");
  tft.setTextColor(WHITE);
  tft.print("Tac ");
  tft.setTextColor(GREEN);
  tft.print("Toe");
  //Set second text
  tft.setCursor(60,90);
  tft.setTextColor(RED);
  tft.setTextSize(3);
  tft.print("Az-Delivery");
  //Set last line
  tft.setTextColor(WHITE);
  tft.setCursor(30,130);
  tft.print("(c)Joern Weise");
  //Init needed buttons
  btnEasy.initButton(&tft, 100, 190, 100, 40, GREEN, BLUE, GREEN, "Easy", 2);
  btnEasy.drawButton();
  btnHard.initButton(&tft, 220, 190, 100, 40, YELLOW, RED, YELLOW, "Hard", 2);
  btnHard.drawButton();
}

//Check in "main-menu" if human select a game
void loop() {
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval )
  {
    previousMillis = currentMillis;
    if(iEnableButtons)
    {
      bPressed = Touch_getXY();
      if(bPressed)
        Serial.println("X: " + String(pixel_x) + " Y: "  + String(pixel_y));
      btnEasy.press(bPressed && btnEasy.contains(pixel_x,pixel_y));
      if (btnEasy.justPressed()){
        //btnEasy.drawButton(true);
        Serial.println("Easy game");
        ResetGame();
        playGame(false);  //Easy Mode
      }
      btnHard.press(bPressed && btnHard.contains(pixel_x,pixel_y));
      if (btnHard.justPressed()){
        //btnHard.drawButton(true);
        Serial.println("Hard game");
        ResetGame();
        playGame(true);  //Hard  Mode
      }
    }
  }
}

//Generate play-screen and reset all vars
//Each element from field can have own color
void ResetGame(){
  iEnableButtons = 0;
  int iCnt = 0;
  for(iCnt = 0; iCnt < 9; iCnt++)
    iSetMark[iCnt] = 0;
  iWinner = 0; //Nobody wins so far :)
  iMoves = 1;
  tft.fillScreen(BLACK);
  //Draw frame
  drawFrame(5, WHITE);
  drawVerticalLine(125, WHITE);
  drawVerticalLine(195, WHITE);
  drawHorizontalLine(80, WHITE);
  drawHorizontalLine(150, WHITE);
  bPressed = false;
}

//Start a loop to play game
void playGame(bool bHardMode)
{
  do
  {
    Serial.println("MOVE: " + String(iMoves));  //Print move
    if(!bHardMode) //Easy-Mode
    {
      if(iMoves % 2 == 1)
      {
        movePlayer(); 
        printBoard();
        checkWinner();  
      }
      else
      {
        moveArduino(false);
        printBoard();
        checkWinner();
      }  
    }
    else //Hard-Mode
    {
      if(iMoves % 2 == 1)
      {
        moveArduino(true);
        printBoard();
        checkWinner();
      }
      else
      {
        movePlayer(); 
        printBoard();
        checkWinner();  
      }
    } 
    iMoves++;
  }
  while(iWinner == 0 && iMoves < 10);
  //End of game, cause somebody win or no moves left
  if(iWinner == 1)
  {
    Serial.println("CPU WINS");
    delay(3000);
    drawGameEndScreen();
  }
  else if(iWinner == 2)
  {
    Serial.println("HUMAN WINS");
    delay(3000);
    drawGameEndScreen();
  }
  else
  {
    Serial.println("DRAW");
    delay(3000);
    drawGameEndScreen();
  }
}

//Function to see if Arduino or human wins
void checkWinner() 
{
  // Player wins?
  if (iSetMark[0]==1 && iSetMark[1]==1 && iSetMark[2]==1)
    iWinner=1; 
  if (iSetMark[3]==1 && iSetMark[4]==1 && iSetMark[5]==1)
    iWinner=1; 
  if (iSetMark[6]==1 && iSetMark[7]==1 && iSetMark[8]==1)
    iWinner=1; 
  if (iSetMark[0]==1 && iSetMark[3]==1 && iSetMark[6]==1)
    iWinner=1; 
  if (iSetMark[1]==1 && iSetMark[4]==1 && iSetMark[7]==1)
    iWinner=1; 
  if (iSetMark[2]==1 && iSetMark[5]==1 && iSetMark[8]==1)
    iWinner=1;  
  if (iSetMark[0]==1 && iSetMark[4]==1 && iSetMark[8]==1)
    iWinner=1; 
  if (iSetMark[2]==1 && iSetMark[4]==1 && iSetMark[6]==1)
    iWinner=1; 
    
  // Arduino wins?
  if (iSetMark[0]==2 && iSetMark[1]==2 && iSetMark[2]==2)
    iWinner=2; 
  if (iSetMark[3]==2 && iSetMark[4]==2 && iSetMark[5]==2)
    iWinner=2; 
  if (iSetMark[6]==2 && iSetMark[7]==2 && iSetMark[8]==2) 
    iWinner=2; 
  if (iSetMark[0]==2 && iSetMark[3]==2 && iSetMark[6]==2) 
    iWinner=2; 
  if (iSetMark[1]==2 && iSetMark[4]==2 && iSetMark[7]==2)
    iWinner=2; 
  if (iSetMark[2]==2 && iSetMark[5]==2 && iSetMark[8]==2)
    iWinner=2; 
  if (iSetMark[0]==2 && iSetMark[4]==2 && iSetMark[8]==2)
    iWinner=2; 
  if (iSetMark[2]==2 && iSetMark[4]==2 && iSetMark[6]==2)
    iWinner=2; 
 
}

//Draw vertical line with given color
void drawVerticalLine(int x, uint16_t color)
{
  int iCnt = 0;
  for(iCnt = 0; iCnt  < 7; iCnt ++)
  {
    tft.drawLine(x+iCnt, 20, x+iCnt, 220, color);
  }
}

//Draw horizontal line with given color
void drawHorizontalLine(int y, uint16_t color)
{
  int iCnt = 0;
  for(iCnt = 0; iCnt < 7; iCnt++)
  {
    tft.drawLine(60, y+iCnt, 270, y+iCnt, color);
  }
}

//Draw frame with given size and color
void drawFrame(int iSize, uint16_t color)
{
  int iCnt;
  for(iCnt = 0; iCnt <= iSize; iCnt++)
    tft.drawRect(0+iCnt,0+iCnt,320-(iCnt*2),240-(iCnt*2),color);
}

//Draw game end screen with winner
//and the buttons to restart a game
void drawGameEndScreen()
{
  tft.fillScreen(BLACK);
  drawFrame(10,RED);
  tft.setCursor(50,30);
  tft.setTextColor(WHITE);
  tft.setTextSize(4);
  tft.print("GAME ENDS");

  if(iWinner == 0)
  {
    //Print "DRAW!" text 
    tft.setCursor(110,100);
    tft.setTextColor(YELLOW);
    tft.setTextSize(4);
    tft.print("DRAW");
  }
  if(iWinner == 1)
  {
    //Print "CPU WINS!" text 
    tft.setCursor(70,100);
    tft.setTextColor(BLUE);
    tft.setTextSize(4);
    tft.print("CPU WINS");
  }

  if(iWinner == 2)
  {
    //Print "HUMAN WINS!" text 
    tft.setCursor(40,100);
    tft.setTextColor(GREEN);
    tft.setTextSize(4);
    tft.print("HUMAN WINS");
  }
  //Draw and enable buttons again
  btnEasy.drawButton();
  btnHard.drawButton();
  iEnableButtons = 1;
  bPressed = false;
}

//Function to show in Serialmonitor the set game moves
void printBoard()
{
  int iCnt = 0;
  Serial.print("Board: [");
  for(iCnt = 0; iCnt < 9; iCnt++)
  {
    if(iCnt != 0)
      Serial.print(",");
    Serial.print(iSetMark[iCnt]);
  }
  Serial.println("]");
}

//Get input from player and check if move is possible
void movePlayer()
{
  bool bValidMove = false;
  Serial.println("Players move");
  do
  {
    bPressed = false;
    bPressed = Touch_getXY();
    if(bPressed)
    {
      Serial.println("X: " + String(pixel_x) + " Y: "  + String(pixel_y));
      if((pixel_x<115)&& (pixel_y>=150)) //6
      {
        if(iSetMark[6]==0)
        {
          Serial.println("Player Move: 6");
          iSetMark[6] = 2;
          bValidMove = true;
          drawPlayerMove(6);  
          Serial.println("Drawing player move");
        }
      }
      else if((pixel_x>0 && pixel_x<115)&& (pixel_y<150 && pixel_y>80)) //3
      {
        if(iSetMark[3]==0)
        {
         Serial.println("Player Move: 3");
          iSetMark[3] = 2;
          bValidMove = true;
          drawPlayerMove(3);  
          Serial.println("Drawing player move");
        }
      }
      else if((pixel_x<125)&& (pixel_y<80)) //0
      {
        if(iSetMark[0]==0)
        {
          Serial.println("Player Move: 0");          
          iSetMark[0] = 2;
          bValidMove = true;
          drawPlayerMove(0);  
        }
      }
      else if((pixel_x>125 && pixel_x<=195)&& (pixel_y<80)) //1
      {
        if(iSetMark[1]==0)
        {
          Serial.println("Player Move: 1");          
          iSetMark[1] = 2;
          bValidMove = true;
          drawPlayerMove(1);  
        }
      }
      else if((pixel_x>195)&& (pixel_y<80)) //2
      {
        if(iSetMark[2]==0)
        {
          Serial.println("Player Move: 2");          
          iSetMark[2] = 2;
          bValidMove = true;
          drawPlayerMove(2);  
        }
      }
      else if((pixel_x>125 && pixel_x<=195)&& (pixel_y<150 && pixel_y>80)) //4
      {
        if(iSetMark[4]==0)
        {
          Serial.println("Player Move: 4");          
          iSetMark[4] = 2;
          bValidMove = true;
          drawPlayerMove(4);  
        }
      }
      else if((pixel_x>195)&& (pixel_y<150 && pixel_y>80)) //5
      {
        if(iSetMark[5]==0)
        {
          Serial.println("Player Move: 5");          
          iSetMark[5] = 2;
          bValidMove = true;
          drawPlayerMove(5);  
        }
      }
      else if((pixel_x>125 && pixel_x<=195)&& (pixel_y>150)) //7
      {
        if(iSetMark[7]==0)
        {
          Serial.println("Player Move: 7");          
          iSetMark[7] = 2;
          bValidMove = true;
          drawPlayerMove(7);  
        }
      }
      else if((pixel_x>195)&& (pixel_y>150)) //8
      {
        if(iSetMark[8]==0)
        {
          Serial.println("Player Move: 8");          
          iSetMark[8] = 2;
          bValidMove = true;
          drawPlayerMove(8);  
        }
      }
    }
  }while(!bValidMove);
}

//Function to let Arduino make a move
void moveArduino(bool bHardMode)
{
  bool bValidMove = false;
  int iRandomMove;
  Serial.println("Arduino move");
  //Hard mode
  if(bHardMode)
  {
    //First move Arduino draw direct to middle or sometimes other pos
    if(iMoves == 1)
    {
      if(millis() % 8 == 0) //Thats why other position is possible
      {
        iRandomMove = random(9);
        iSetMark[iRandomMove] = 1;
        Serial.println("Arduino Move: " + String(iRandomMove));
        drawArduinoMove(iRandomMove);
      }
      else
      {
        iSetMark[4] = 1;
        Serial.println("Arduino Move: " + String(4));
        drawArduinoMove(4);
      }
    }
    else
    {
      int iNextMove = checkPlayerMove();
      int iWinMove = checkPossibleWin();
      Serial.println("Check player: " + String(iWinMove));
      if(iWinMove >= 0)
      {
        delay(1000);
        iSetMark[iWinMove] = 1;
        Serial.println("Arduino Move: " + String(iWinMove));
        drawArduinoMove(iWinMove);
      }
      else
      {
        if(iNextMove >= 0)
        {
          iSetMark[iNextMove] = 1;
          Serial.println("Arduino Move: " + String(iNextMove));
          drawArduinoMove(iNextMove);
        }
        else
        {
          do{
            iRandomMove = random(9);
            if(iSetMark[iRandomMove] == 0)
            {
              delay(1000);
              iSetMark[iRandomMove] = 1;
              Serial.println("Arduino Move: " + String(iRandomMove));
              drawArduinoMove(iRandomMove);
              bValidMove = true;
            }
          }while(!bValidMove);
        }
      }
    } //else
  } //if(bHardMode)
  else  //Easy Mode 
  {
    do{
      iRandomMove = random(9);
      if(iSetMark[iRandomMove] == 0)
      {
        delay(1000);
        iSetMark[iRandomMove] = 1;
        Serial.println("Arduino Move: " + String(iRandomMove));
        drawArduinoMove(iRandomMove);
        bValidMove = true;
      }
    }while(!bValidMove);
  }
}

//Check if player is able to win next move
int checkPlayerMove()
{
  if(((iSetMark[1] == 2 && iSetMark[2] == 2) || (iSetMark[3] == 2 && iSetMark[6] == 2) || (iSetMark[4] == 2 && iSetMark[8] == 2)) && iSetMark[0] == 0 )
  return 0;
  else if(((iSetMark[0] == 2 && iSetMark[2] == 2) || (iSetMark[4] == 2 && iSetMark[7] == 2)) && iSetMark[1] == 0)
  return 1;
  else if(((iSetMark[0] == 2 && iSetMark[1] == 2) || (iSetMark[5] == 2 && iSetMark[8] == 2) || (iSetMark[4] == 2 && iSetMark[6] == 2)) && iSetMark[2] == 0 )
  return 2;
  else if(((iSetMark[0] == 2 && iSetMark[6] == 2) || (iSetMark[4] == 2 && iSetMark[5] == 2)) && iSetMark[3] == 0)
  return 3;
  else if(((iSetMark[1] == 2 && iSetMark[7] == 2) || (iSetMark[3] == 2 && iSetMark[5] == 2) || (iSetMark[0] == 2 && iSetMark[8] == 2) || (iSetMark[2] == 2 && iSetMark[6] == 2)) && iSetMark[4] == 0 )
  return 4;
  else if(((iSetMark[2] == 2 && iSetMark[8] == 2) || (iSetMark[3] == 2 && iSetMark[4] == 2 )) && iSetMark[5] == 0 )
  return 5;
  else if(((iSetMark[0] == 2 && iSetMark[3] == 2) || (iSetMark[7] == 2 && iSetMark[8] == 2) || (iSetMark[4] == 2 && iSetMark[2] == 2)) && iSetMark[6] == 0 )
  return 6;
  else if(((iSetMark[1] == 2 && iSetMark[4] == 2) || (iSetMark[6] == 2 && iSetMark[8] == 8)) && iSetMark[7] == 0 )
  return 7;
  else if(((iSetMark[0] == 2 && iSetMark[4] == 2) || (iSetMark[2] == 2 && iSetMark[5] == 2) || (iSetMark[6] == 2 && iSetMark[7] == 2))  && iSetMark[8] == 0)
  return 8;
  else
  return -1;
}

//Check for arduino if its possible to win with next move
int checkPossibleWin()
{
  if(((iSetMark[1] == 1 && iSetMark[2] == 1) || (iSetMark[3] == 1 && iSetMark[6] == 1) || (iSetMark[4] == 1 && iSetMark[8] == 1)) && iSetMark[0] == 0 )
  return 0;
  else if(((iSetMark[0] == 1 && iSetMark[2] == 1) || (iSetMark[4] == 1 && iSetMark[7] == 1)) && iSetMark[1] == 0)
  return 1;
  else if(((iSetMark[0] == 1 && iSetMark[1] == 1) || (iSetMark[5] == 1 && iSetMark[8] == 1) || (iSetMark[4] == 1 && iSetMark[6] == 1)) && iSetMark[2] == 0 )
  return 2;
  else if(((iSetMark[0] == 1 && iSetMark[6] == 1) || (iSetMark[4] == 1 && iSetMark[5] == 1)) && iSetMark[3] == 0)
  return 3;
  else if(((iSetMark[1] == 1 && iSetMark[7] == 1) || (iSetMark[3] == 1 && iSetMark[5] == 1) || (iSetMark[0] == 1 && iSetMark[8] == 1) || (iSetMark[2] == 1 && iSetMark[6] == 1)) && iSetMark[4] == 0 )
  return 4;
  else if(((iSetMark[2] == 1 && iSetMark[8] == 1) || (iSetMark[3] == 1 && iSetMark[4] == 1 )) && iSetMark[5] == 0 )
  return 5;
  else if(((iSetMark[0] == 1 && iSetMark[3] == 1) || (iSetMark[7] == 1 && iSetMark[8] == 1) || (iSetMark[4] == 1 && iSetMark[2] == 1)) && iSetMark[6] == 0 )
  return 6;
  else if(((iSetMark[1] == 1 && iSetMark[4] == 1) || (iSetMark[6] == 1 && iSetMark[8] == 8)) && iSetMark[7] == 0 )
  return 7;
  else if(((iSetMark[0] == 1 && iSetMark[4] == 1) || (iSetMark[2] == 1 && iSetMark[5] == 1) || (iSetMark[6] == 1 && iSetMark[7] == 1))  && iSetMark[8] == 0)
  return 8;
  else
  return -1;
}

//Draw move from player
void drawPlayerMove(int pos)
{
  switch(pos)
  {
    case 0: drawSign(55,15,circle,RED);    break;
    case 1: drawSign(130,15,circle,RED);   break;
    case 2: drawSign(205,15,circle,RED);   break;
    case 3: drawSign(55,85,circle,RED);    break;
    case 4: drawSign(130,85,circle,RED);   break;
    case 5: drawSign(205,85,circle,RED);   break;
    case 6: drawSign(55,155,circle,RED);   break;
    case 7: drawSign(130,155,circle,RED);  break;
    case 8: drawSign(205,155,circle,RED);  break;
  }
}

//Draw move from CPU
void drawArduinoMove(int pos)
{
  switch(pos)
  {
    case 0: drawSign(55,15,x_bitmap,BLUE);    break;
    case 1: drawSign(130,15,x_bitmap,BLUE);   break;
    case 2: drawSign(205,15,x_bitmap,BLUE);   break;
    case 3: drawSign(55,85,x_bitmap,BLUE);    break;
    case 4: drawSign(130,85,x_bitmap,BLUE);   break;
    case 5: drawSign(205,85,x_bitmap,BLUE);   break;
    case 6: drawSign(55,155,x_bitmap,BLUE);   break;
    case 7: drawSign(130,155,x_bitmap,BLUE);  break;
    case 8: drawSign(205,155,x_bitmap,BLUE);  break;
  }
  
}

//Draw the sign for player and arduino
void drawSign(int x, int y, const uint8_t *sign, uint16_t color)
{
  int iWidth = 65;
  int iHeight = 65;
  int i, j, byteWidth = (iWidth + 7) / 8;
  uint8_t byte;

  for(j = 0; j < iHeight; j++)
  {
    for(i = 0; i < iWidth; i++) 
    {
      if(i & 7)
        byte <<= 1;
      else
        byte   = pgm_read_byte(sign + j * byteWidth + i / 8);
      if(byte & 0x80)
        tft.drawPixel(x+i, y+j, color);
    }
  }
}
