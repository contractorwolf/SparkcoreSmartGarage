// This #include statement was automatically added by the Spark IDE.




//*************************************//
// --- WIDE.HK---//
// - SSD131x PMOLED Controller      -//
// - SCL, SDA, GND, VCC(3.3v 5v)   --//
//*************************************//


/*
****************************************
This code is rewritten from the original code for the Arduino to control the I2C OLED from http://Wide.HK

The code below works to use the SparkCore microcontrller (available at http://spark.io )
Pins used on the SparkCore:

    DO - SDA
    D1 - SCL
    3V
    GND

see here for pins: http://docs.spark.io/#/hardware/pins-and-i-o-i2c
****************************************
*/

/*
test status of door
open door
close door
*/






const char OLED_Address  = 0x3c;
const char OLED_Command_Mode = 0x80;
const char OLED_Data_Mode = 0x40;


const String version = "v1";


const int pinLED1 = D6;
const int pinLED2 = D7;
const int pinReedSensor = A0;
const int pinRelayGND = D3;
const int pinRelaySignal = D2;


long lastClosedTime;
int isClosed = 0;
bool isCleared = false;
int screenOffDelay = 30000;//time before screen turns off when door is closed
int doorButtonDelay = 1000;

void setup()
{
    //pin initializations
    pinMode(pinLED1,OUTPUT);
    digitalWrite(pinLED1, LOW);     
    pinMode(pinLED2,OUTPUT); 
    digitalWrite(pinLED2, LOW);    
    
    pinMode(pinRelayGND,OUTPUT);    
    digitalWrite(pinRelayGND, LOW);    
    pinMode(pinRelaySignal,OUTPUT);    
    digitalWrite(pinRelaySignal, LOW);    
   
    pinMode(pinReedSensor,INPUT); 
    
    
    isClosed = digitalRead(pinReedSensor);
    
    //begin Wire communication with OLED
    Wire.begin();
  
    //set the one external function to call to update the OLED with a message
    Spark.function("UpdateScreen", UpdateScreen);
    Spark.function("OperateDoor", OperateDoor);  
    
    
    
    
    Spark.variable("isClosed", &isClosed, INT);
    
  
   // //initialize screen
    SetupScreen();
    
    sendCommand(0x01);    // ** Clear display
    sendMessage("Smart Garage");
    sendCommand(0xC0);      // ** New Line
    sendMessage(version + " started");
    
    
     
    delay(1000);//wait one sec to see the started message
     
    sendCommand(0x01);    // ** Clear display
    sendMessage("Smart Garage");
    sendCommand(0xC0);      // ** New Line
    sendMessage("READY");
    
}

void loop()
{
    //nothing happens in loop in this example
    //just call 'update' from the web to update screen
    
    if(isClosed==0 && digitalRead(pinReedSensor)==1){
        sendCommand(0x01);    // ** Clear display
        sendMessage("DOOR CLOSED");
        lastClosedTime = millis();
        isCleared = false;
    }
    if(isClosed==1 && digitalRead(pinReedSensor)==0){
        sendCommand(0x01);    // ** Clear display
        sendMessage("DOOR OPEN");
    }    
    
    
    
    if((lastClosedTime + screenOffDelay)<millis() && !isCleared && isClosed == 1){
        sendCommand(0x01);    // ** Clear display      
        isCleared = true;
    }
    
    
    isClosed = digitalRead(pinReedSensor);
}

int OperateDoor(String args){
    int status_code = -1;
    
    if(args == "OPEN" && isClosed==1){
        sendCommand(0x01);    // ** Clear display
        sendMessage("OPENING DOOR");
        digitalWrite(pinRelaySignal, HIGH);
        delay(doorButtonDelay);
        digitalWrite(pinRelaySignal, LOW);
        status_code = 1;        
        
    }else if(args == "CLOSE" && isClosed == 0){
        sendCommand(0x01);    // ** Clear display
        sendMessage("CLOSING DOOR");
        digitalWrite(pinRelaySignal, HIGH);
        delay(doorButtonDelay);
        digitalWrite(pinRelaySignal, LOW);
        status_code = 0;
    }
    
    return status_code;
}


int UpdateScreen(String args)
{
    int status_code = 0;
    
    sendCommand(0x01);    // ** Clear display
    
    //fix string
    args.replace("%20"," ");
    int commaPosition = args.indexOf(",");//find if there is a delim character
    
    if(commaPosition>-1){
        //two lines
        sendMessage(args.substring(0,commaPosition));//send first part
        sendCommand(0xC0);      // ** New Line
        sendMessage(args.substring(commaPosition+1, args.length()));//send second part
        
        status_code = 2;//lines
    }else{
        //one line
        sendMessage(args);
        status_code = 1;//lines
    }
    
    //how many lines sent to display
    return status_code;
}


//********************************************
// OLED SCREEN METHODS
// i2c screen initialization
void SetupScreen(void){
   // * I2C initial * //
    delay(100);
    sendCommand(0x2A);    // ** Set "RE"=1    00101010B
    sendCommand(0x71);
    sendCommand(0x5C);
    sendCommand(0x28);
    
    sendCommand(0x08);    // ** Set Sleep Mode On
    sendCommand(0x2A);    // ** Set "RE"=1    00101010B
    sendCommand(0x79);    // ** Set "SD"=1    01111001B
    
    sendCommand(0xD5);
    sendCommand(0x70);
    sendCommand(0x78);    // ** Set "SD"=0
    
    sendCommand(0x08);    // ** Set 5-dot, 3 or 4 line(0x09), 1 or 2 line(0x08)
    
    sendCommand(0x06);    // ** Set Com31->Com0  Seg0->Seg99
    
    // ** Set OLED Characterization * //
    sendCommand(0x2A);      // ** Set "RE"=1
    sendCommand(0x79);      // ** Set "SD"=1
    
    // ** CGROM/CGRAM Management * //
    sendCommand(0x72);      // ** Set ROM
    sendCommand(0x00);      // ** Set ROM A and 8 CGRAM
    
    sendCommand(0xDA);     // ** Set Seg Pins HW Config
    sendCommand(0x10);   
    
    sendCommand(0x81);      // ** Set Contrast
    sendCommand(0xFF);
    
    sendCommand(0xDB);      // ** Set VCOM deselect level
    sendCommand(0x30);      // ** VCC x 0.83
    
    sendCommand(0xDC);      // ** Set gpio - turn EN for 15V generator on.
    sendCommand(0x03);
    
    sendCommand(0x78);      // ** Exiting Set OLED Characterization
    sendCommand(0x28);
    sendCommand(0x2A);
    //sendCommand(0x05);     // ** Set Entry Mode
    sendCommand(0x06);     // ** Set Entry Mode
    sendCommand(0x08);  
    sendCommand(0x28);     // ** Set "IS"=0 , "RE" =0 //28
    sendCommand(0x01);
    sendCommand(0x80);     // ** Set DDRAM Address to 0x80 (line 1 start)
    
    delay(100);
    sendCommand(0x0C);      // ** Turn on Display
  
}


//send data character
void sendData(unsigned char data)
{
    Wire.beginTransmission(OLED_Address);      // ** Start I2C
    Wire.write(OLED_Data_Mode);             // ** Set OLED Data mode
    Wire.write(data);
    Wire.endTransmission();                     // ** End I2C
}

//send command character
void sendCommand(unsigned char command)
{
    Wire.beginTransmission(OLED_Address);      // ** Start I2C
    Wire.write(OLED_Command_Mode);              // ** Set OLED Command mode
    Wire.write(command);
    Wire.endTransmission();                      // ** End I2C
    delay(10);
}

//send string
void sendMessage(String message)
{
    unsigned char i=0;
    while(message[i])
    {
        sendData(message[i]);      // * Show String to OLED
        i++;
    }
}






