//Arduino Nano V3.0
//SIM800L with AT Commands without library
#include <SoftwareSerial.h>
#define TX 10
#define RX 11
#define RING 2
SoftwareSerial sim800(TX, RX);// Wiring TX 10 , RX 11
String command = "", feedback = "", smsnumber = "", smstxt = "", smsdate = "", smstime = "";
bool time1_s = LOW;
bool time2_s = LOW;
int time1_1, time1_2, time1_i=20000;
int time2_1, time2_2, time2_i=30000;


void setup()
{
  pinMode(RING, INPUT_PULLUP);
  Serial.begin(9600);
  sim800.begin(9600);
  //Serial.println("initialising Sim Modul, wait 5 seconds");
  //delay(5000);
  //sim800test(); // unkomment for test SIM800L at setup 
  clear_sms(); // clear received and sendet sms at sim card
  AT("CMGF=1"); // Set Textmode
  updateserial();
  AT("CNMI=1,2,0,0,0"); // what to do with received sms ?
  updateserial();
}

void loop()
{
   // Continuous reading of the serial port and saving in the variable feedback
  feedback = updateserial();

  // if the variable feedback contains the text + CMT an SMS has arrived
  if (feedback.indexOf("+CMT:") > 0 ) handle_sms(); // what to do äf incoming sms

  // if the variable feedback contains the text RING a call has been received
  if (feedback.indexOf("RING") > 0 ) handle_ring(); // what to do äf incoming call

  //do other things while waiting for a call or sms
  loop1();

    //do other things while waiting for a call or sms
  loop2();
}

//**********************************************************
// function sending AT commands to SIM800L
//**********************************************************
String AT (String command)
{
  sim800.println(("AT+" + command));
  delay(100);
  while (sim800.available()) {
    return sim800.readString();
  }
}
//*********************************************************

//**********************************************************
// function writing text from serial port to SIM800L
//**********************************************************
void sim800write()
{
  while (Serial.available())
  {
    sim800.println(Serial.readString());
  }
}
//*********************************************************

//**********************************************************
// function writing received text from SIM800L to serial port/monitor
//**********************************************************
String sim800read()
{
  String readsim800;
  while (sim800.available())
  {
    readsim800 = sim800.readString();
    Serial.println(readsim800);
  }
  return readsim800;
}
//*********************************************************

//**********************************************************
// function update write and read booth serial ports
//**********************************************************
String updateserial()
{
  sim800write();
  return sim800read();
}
//*********************************************************

//**********************************************************
// function delete sms at sim card
//**********************************************************
void clear_sms()
{
  AT("CMGD=4");
  updateserial();
}
//*********************************************************

//**********************************************************
// function Compose number to send via AT command
//**********************************************************
String send_to(String number) //+xx165476465378
{
  String sendto = "";
  sendto = "CMGS=";
  sendto = sendto + "\"";
  sendto = sendto + number;
  sendto = sendto + "\"";
  return sendto;
}
//*********************************************************

//**********************************************************
// function send sms at number , txt >> send_sms("+491764569111","Sending this text");
//**********************************************************
void send_sms(String number, String smstxt)
{
  String sendto = send_to(number);
  Serial.println("Sending SMS to phone number : " + sendto.substring(5));
  Serial.print("Sending Text: ");
  AT(sendto);
  sim800.println(smstxt);// sending text direct println
  delay(100);
  updateserial();
  sim800.println((char)26); //sending direct chr(26) for ending sms sending
  delay(100);
  String issmssendet = updateserial();
  if (issmssendet.indexOf("+CMGS")) Serial.println("SMS successfully sent");
}
//*********************************************************

//**********************************************************
// function what to do with incoming sms ?
// sms is stored ind variable feedback
//**********************************************************
void handle_sms()
{
  smsdate = feedback.substring(28, 37);
  smstime = feedback.substring(38, 50);
  smsnumber = feedback.substring(9, 23);
  smstxt = feedback.substring(52); // sms begins at 52
  Serial.println("SMS Date: " + smsdate);
  Serial.println("SMS Time: " + smstime);
  Serial.println("SMS from: " + smsnumber);
  Serial.println("SMS Text: " + smstxt);
  clear_sms();
  if (feedback.indexOf("led1on")>0) LED_1_ON();
  if (feedback.indexOf("motor1start")>0) MOTOR_1_START();
  if (feedback.indexOf("Servo 3")>0) SERVO3(); // SMS begins at 52
  if (feedback.indexOf("settime")>0) settime();
  

}
//*********************************************************

//**********************************************************
// function what to do with incoming call ?
//**********************************************************
void handle_ring()
{
  delay(2000);
  AT("CHUP"); // Hang up call / end call
  send_sms("your smartphone number", "** incoming call **") ;
}
//*********************************************************

//**********************************************************
// function test SIM800 at begin
//**********************************************************
void sim800test()
{
  command = "";
  Serial.println("\n");
  command = AT("CCLK?");
  Serial.println("Date & Time: " + command.substring(19, 39)); // get Date & Time, set Time AT+CCLK="18/12/06,22:10:00+01"
  command = AT("CGMI");
  Serial.println("Manufacturer: " + command.substring(10, 20)); // get Manufacturer
  command = AT("CGMM");
  Serial.println("Model: " + command.substring(10, 20)); // get Model
  command = AT("CGMR");
  Serial.println("Revison: " + command.substring(19, 36)); // get Revision
  command = AT("CGSN");
  Serial.println("Serial Number: " + command.substring(10, 25)); // get Serial Number
  command = AT("GSN");
  Serial.println("IMEI Nummer: " + command.substring(13, 25)); // IMEI Nummer
  command = AT("CIMI");
  Serial.println("IMSIl Number: " + command.substring(10, 25)); // get IMSI Number
  command = AT("CSQ");
  Serial.println("Signal strength: " + command.substring(13, 20)); // get signal strength, value range is 0-31 , 31 is the best
  command = AT("CBC");
  Serial.println("Battery status: " + command.substring(17, 20) + " %" ); // get battery status (%) and Volatge (mV) // 17-23
  Serial.println("Battery Voltage: " + command.substring(21, 22) + "." + command.substring(22, 24) + " V" ); // get battery status (%) and Volatge (mV)
}
//*********************************************************

//**********************************************************
// function what to do waiting vor Call or incoming sms
//**********************************************************
void loop1()
{
  if (time1_s == LOW)
  {
    time1_s = HIGH;
    time1_1 = millis();
  }
  time1_2 = millis();
  if ((time1_2 - time1_1) > time1_i)
  {
    // do every 10 sec btween { ... }
    time1_s = LOW;
    Serial.println("10 second past with doing nothing");
    // do every 10 sec btween { ... }
  }
}
//*********************************************************

//**********************************************************
// function what to do waiting vor Call or incoming sms
//**********************************************************
void loop2()
{
  if (time2_s == LOW)
  {
    time2_s = HIGH;
    time2_1 = millis();
  }
  time2_2 = millis();
  if ((time2_2 - time2_1) > time2_i)
  {
    // do every 3 sec btween { ... }
    time2_s = LOW;
    Serial.println("3 second past with doing nothing");
    // do every 3 sec btween { ... }
  }
}
//*********************************************************


void LED_1_ON()
{
 // digitalWrite(1,HIGH);
 send_sms("your smartphone number ","LED 1 ON");
}

void MOTOR_1_START()
{
 // Motor 1 Start
  send_sms("your smartphone number ","Motor 1 started");
}

void SERVO3()
{
  int degree=0;
  degree=feedback.substring(59).toInt(); // sms begins at 52 + 7 chars for (Servo 3) and Space = 59
  Serial.println("Degree of Servo: " +String(degree));  
  send_sms("your smartphone number", "Servo 3 at "+String(degree)+" degree");
 }

 void settime()
 {
  String settime="";
  settime="AT+CCLK="+feedback.substring(28,50);
  Serial.println("Settime: "+settime);
  sim800.println(settime);
  delay(300);
  updateserial();
  delay(300);
  send_sms("your smartphone number","set Date/Time : "+settime.substring(9));
 }
 //Edit
