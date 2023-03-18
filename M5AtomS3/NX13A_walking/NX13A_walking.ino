#include <M5AtomS3.h>
#include <DabbleESP32.h>

#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

const uint8_t Srv0 = 5; //GPIO Right Arm
const uint8_t Srv1 = 6; //GPIO Right Leg
const uint8_t Srv2 = 7; //GPIO Right Foot
const uint8_t Srv3 = 8; //GPIO Left Foot
const uint8_t Srv4 = 38; //GPIO Left Leg
const uint8_t Srv5 = 39; //GPIO Left Arm

const uint8_t srv_CH0 = 0, srv_CH1 = 1, srv_CH2 = 2, srv_CH3 = 3, srv_CH4 = 4, srv_CH5 = 5; //チャンネル
const double PWM_Hz = 50;   //PWM周波数
const uint8_t PWM_level = 14; //PWM 14bit(0～16384)

int pulseMIN = 410;  //0deg 500μsec 50Hz 14bit : PWM周波数(Hz) x 2^14(bit) x PWM時間(μs) / 10^6
int pulseMAX = 2048;  //180deg 2500μsec 50Hz 14bit : PWM周波数(Hz) x 2^14(bit) x PWM時間(μs) / 10^6

int cont_min = 0;
int cont_max = 180;

int angZero[] = {39, 87, 80, 89, 81, 139}; //Trimming
int ang0[6];
int ang1[6];
int ang_b[6];
char ang_c[6];
float ts=140;  //160msごとに次のステップに移る
float td=10;   //20回で分割

// Forward Step
int f_s[19][6]={
  {0,0,0,0,0,0},
  {0,0,-15,-10,0,0},
  {0,0,-15,-15,0,0},
  {-20,15,-15,-15,15,-20},
  {-20,15,0,0,15,-20},
  {-20,15,10,15,15,-20},
  {-20,15,15,15,15,-20},
  {0,0,15,15,0,0},
  {20,-15,15,15,-15,20},
  {20,-15,0,0,-15,20},
  {20,-15,-15,-10,-15,20},
  {20,-15,-15,-15,-15,20},
  {0,0,-15,-15,0,0},
  {-20,15,-15,-15,15,-20},
  {-20,15,0,0,15,-20},
  {-20,15,10,15,15,-20},
  {-20,15,15,15,15,-20},
  {0,0,15,15,0,0},
  {0,0,0,0,0,0}};

// Back Step
int b_s[19][6]={
  {0,0,0,0,0,0},
  {0,0,-20,-15,0,0},
  {0,0,-15,-15,0,0},
  {20,-15,-15,-15,-15,20},
  {20,-15,0,0,-15,20},
  {20,-15,15,20,-15,20},
  {20,-15,15,15,-15,20},
  {0,0,15,15,0,0},
  {-20,15,15,15,15,-20},
  {-20,15,0,0,15,-20},
  {-20,15,-20,-15,15,-20},
  {-20,15,-15,-15,15,-20},
  {0,0,-15,-15,0,0},
  {20,-15,-15,-15,-15,20},
  {20,-15,0,0,-15,20},
  {20,-15,15,20,-15,20},
  {20,-15,15,15,-15,20},
  {0,0,15,15,0,0},
  {0,0,0,0,0,0}};

// Left Turn_Step
int l_s[9][6]={
  {0,0,0,0,0,0},
  {0,0,10,15,0,0},
  {0,0,15,15,0,0},
  {-20,15,15,15,-15,-20},
  {-20,15,0,0,-15,-20},
  {-20,15,-15,-10,-15,-20},
  {-20,15,-15,-15,-15,-20},
  {0,0,-15,-15,0,0},
  {0,0,0,0,0,0}};

// Right Turn Step
int r_s[9][6]={
  {0,0,0,0,0,0},
  {0,0,-15,-10,0,0},
  {0,0,-15,-15,0,0},
  {-20,15,-15,-15,-15,-20},
  {-20,15,0,0,-15,-20},
  {-20,15,10,15,-15,-20},
  {-20,15,15,15,-15,-20},
  {0,0,15,15,0,0},
  {0,0,0,0,0,0}};

// Right Arm
int r_a[6]={80,0,0,0,0,0};

// Left Arm
int l_a[6]={0,0,0,0,0,-80};

// Both Arm
int b_a[6]={80,0,0,0,0,-80};

// Home Position
int h_p[6]={0,0,0,0,0,0};

int arm_state = 0;
int angry_state = 0;

void Initial_Value(){  //initial servo angle
  int cn = 50;
  for (int j=0; j <=5; j++){
    Srv_drive(j, angZero[j]);
    ang0[j] = angZero[j];
    ang1[j] = angZero[j];
    delay(cn);
  }
}

void face_clear(){
  M5.Lcd.fillScreen(M5.Lcd.color565(0, 150, 255)); //b, g, r   Yellow
  
}

void face_center_eye(){
  face_clear();
  M5.Lcd.fillCircle(39,49,10,0x7800); //Blue
  M5.Lcd.fillCircle(89,49,10,0x7800); //Blue
  M5.Lcd.fillRect(39,83,50,10,0x001F); //Red
  delay(1500);
  face_clear();
  M5.Lcd.fillRect(34,44,10,7,0x7800); //Blue
  M5.Lcd.fillRect(84,44,10,7,0x7800); //Blue
  M5.Lcd.fillRect(39,83,50,10,0x001F); //Red
  delay(100);
}

void face_center(){
  face_clear();
  M5.Lcd.fillCircle(39,49,10,0x7800); //Blue
  M5.Lcd.fillCircle(89,49,10,0x7800); //Blue
  M5.Lcd.fillRect(39,83,50,10,0x001F); //Red
}

void face_angry(){
  M5.Lcd.fillScreen(M5.Lcd.color565(0, 0, 255)); //b, g, r
  M5.Lcd.fillTriangle(14,24,54,54,14,44,0x7800); //Blue
  M5.Lcd.fillTriangle(74,54,114,24,114,44,0x7800); //Blue
  M5.Lcd.fillRect(39,73,50,40,0x0000); //Black
}

void face_right(){
  face_clear();
  M5.Lcd.fillCircle(19,49,10,0x7800); //Blue
  M5.Lcd.fillCircle(69,49,10,0x7800); //Blue
  M5.Lcd.fillRect(19,78,50,20,0x001F); //Red
}

void face_left(){
  face_clear();
  M5.Lcd.fillCircle(59,49,10,0x7800); //Blue
  M5.Lcd.fillCircle(109,49,10,0x7800); //Blue
  M5.Lcd.fillRect(59,78,50,20,0x001F); //Red
}

void Srv_drive(int srv_CH,int SrvAng){
  SrvAng = map(SrvAng, cont_min, cont_max, pulseMIN, pulseMAX);
  ledcWrite(srv_CH, SrvAng);
}

void forward_step()
{
  face_center();
  for (int i=0; i <=18 ; i++){
    for (int j=0; j <=5 ; j++){
      ang1[j] = angZero[j] + f_s[i][j];
    }
  servo_set();
  }
}

void back_step()
{
  face_center();
  for (int i=0; i <=18 ; i++){
    for (int j=0; j <=5 ; j++){
      ang1[j] = angZero[j] + b_s[i][j];
    }
  servo_set();
  }
}

void right_step()
{
  face_right();
  for (int i=0; i <=8 ; i++){
    for (int j=0; j <=5 ; j++){
      ang1[j] = angZero[j] + r_s[i][j];
    }
  servo_set();
  }
  face_clear();
}

void left_step()
{
  face_left();
  for (int i=0; i <=8 ; i++){
    for (int j=0; j <=5 ; j++){
      ang1[j] = angZero[j] + l_s[i][j];
    }
  servo_set();
  }
  face_clear();
}

void right_arm()
{
  face_right();
  for (int j=0; j <=5 ; j++){
    ang1[j] = angZero[j] + r_a[j];
  }
  servo_set();
  delay(500);
  face_clear();
}

void left_arm()
{
  face_left();
  for (int j=0; j <=5 ; j++){
    ang1[j] = angZero[j] + l_a[j];
  }
  servo_set();
  delay(500);
  face_clear();
}

void both_arm()
{
  face_angry();
  for (int j=0; j <=5 ; j++){
    ang1[j] = angZero[j] + b_a[j];
  }
  servo_set();
  delay(500);
}

void home_position()
{
  face_center();
  for (int j=0; j <=5 ; j++){
    ang1[j] = angZero[j] + h_p[j];
  }
  servo_set();
  face_clear();
}

void servo_set(){
  int a[6],b[6];
  
  for (int j=0; j <=5 ; j++){
      a[j] = ang1[j] - ang0[j];
      b[j] = ang0[j];
      ang0[j] = ang1[j];
  }

  for (int k=0; k <=td ; k++){

      Srv_drive(srv_CH0, a[0]*float(k)/td+b[0]);
      Srv_drive(srv_CH1, a[1]*float(k)/td+b[1]);
      Srv_drive(srv_CH2, a[2]*float(k)/td+b[2]);
      Srv_drive(srv_CH3, a[3]*float(k)/td+b[3]);
      Srv_drive(srv_CH4, a[4]*float(k)/td+b[4]);
      Srv_drive(srv_CH5, a[5]*float(k)/td+b[5]);

      delay(ts/td);
  }
}

void setup() {
  Serial.begin(151200);
  M5.begin(true, true, false, false);  // Init M5AtomS3.  初始化 M5AtomS3 液晶,USBシリアル,I2C(38,39),LED
  M5.Lcd.setRotation(4);
  Dabble.begin("M5AtomS3");       //set bluetooth name of your device
  
  pinMode(Srv0, OUTPUT);
  pinMode(Srv1, OUTPUT);
  pinMode(Srv2, OUTPUT);
  pinMode(Srv3, OUTPUT);
  pinMode(Srv4, OUTPUT);
  pinMode(Srv5, OUTPUT);
  
  //モータのPWMのチャンネル、周波数の設定
  ledcSetup(srv_CH0, PWM_Hz, PWM_level);
  ledcSetup(srv_CH1, PWM_Hz, PWM_level);
  ledcSetup(srv_CH2, PWM_Hz, PWM_level);
  ledcSetup(srv_CH3, PWM_Hz, PWM_level);
  ledcSetup(srv_CH4, PWM_Hz, PWM_level);
  ledcSetup(srv_CH5, PWM_Hz, PWM_level);

  //モータのピンとチャンネルの設定
  ledcAttachPin(Srv0, srv_CH0);
  ledcAttachPin(Srv1, srv_CH1);
  ledcAttachPin(Srv2, srv_CH2);
  ledcAttachPin(Srv3, srv_CH3);
  ledcAttachPin(Srv4, srv_CH4);
  ledcAttachPin(Srv5, srv_CH5);

  face_center();

  Initial_Value();
  home_position();
}

void loop() {
  M5.update();
  if ( M5.Btn.wasReleased() ) {
    Initial_Value();
  }

  if (angry_state == 0)
  {
    face_center_eye();
  }
  
  Dabble.processInput();             //this function is used to refresh data obtained from smartphone.Hence calling this function is mandatory in order to get data properly from your mobile.

  if (GamePad.isUpPressed())
  {
    forward_step();
    Serial.println("FWD");
  }

  if (GamePad.isDownPressed())
  {
    back_step();
    Serial.println("BACK");
  }

  if (GamePad.isLeftPressed())
  {
    left_step();
    Serial.println("LEFT STEP");
  }

  if (GamePad.isRightPressed())
  {
    right_step();
    Serial.println("RIGHT STEP");
  }
  
  if (GamePad.isSquarePressed())
  {
    if(arm_state == 0)
    {
      left_arm();
      arm_state = 1;
    }
    else if(arm_state == 1)
    {
      home_position();
      arm_state = 0;
    }
    Serial.println("Left ARM");
  }

  if (GamePad.isCirclePressed())
  {
    if(arm_state == 0)
    {
      right_arm();
      arm_state = 1;
    }
    else if(arm_state == 1)
    {
      home_position();
      arm_state = 0;
    }
    Serial.println("RIGHT ARM");
  }

  if (GamePad.isTrianglePressed())
  {
    if(angry_state == 0)
    {
      both_arm();
      angry_state = 1;
    }
    else
    {
      home_position();
      angry_state = 0;
    }
    Serial.println("BOTH ARM");
  }

  //delay(100);
}
