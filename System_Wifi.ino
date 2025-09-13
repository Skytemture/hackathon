#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <qrcode_gen.h>

const char* ssid = "神奇海螺";       // 雙引號內，修改為你要 ESP32 連上的 WiFi 網路名稱 SSID
const char* password = "12345678";   // 雙引號內，鍵入此網路的密碼

WiFiServer server(80);  //設定網路伺服器 port number 為 80

// Variable to store the HTTP request
String header;

String output32State = "off";
String output33State = "off";
String output34State = "off";
String output35State = "off";
String output36State = "off";  

double in_1 = 0;
double in_2 = 0;
double in_3 = 0;
double in_4 = 0;
double in_5 = 0;
double in_6 = 0;
double in_7 = 0;

int power = 0;
long int receivedNum = 0;

int state32 = 0;
int state33 = 0;
int state34 = 0;
int state35 = 0;
int state36 = 0;

// Assign output variables to GPIO pins
const int input19 = 19;
const int input23 = 23;
const int input24 = 24;
const int input25 = 25;
const int input26 = 26;
const int input27 = 27;

const int output32 = 32;
const int output33 = 33;
const int output34 = 12;
const int output35 = 13;
const int output36 = 14;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

//============OLED顯示QRCODE================
// OLED display dimensions
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// OLED display object
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200); //設定序列埠螢幕顯示的 baud rate 為 115200
  Serial2.begin(9600);

  Serial.println("9600開啟，開始接收..");
  // Initialize the output variables as outputs
  pinMode(input19, INPUT);
  pinMode(input23, INPUT);
  pinMode(input24, INPUT);
  pinMode(input25, INPUT);
  pinMode(input26, INPUT);
  pinMode(input27, INPUT);
  
  pinMode(output32, OUTPUT);
  pinMode(output33, OUTPUT);
  pinMode(output34, OUTPUT);
  pinMode(output35, OUTPUT);
  pinMode(output36, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output32, LOW);
  digitalWrite(output33, LOW);
  digitalWrite(output34, LOW);
  digitalWrite(output35, LOW);
  digitalWrite(output36, LOW);

  Serial.print("Connecting to ");  // 連上你所指定的Wi-Fi，並在序列埠螢幕中，印出 ESP32 web server 的 IP address
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  // Initialize the OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  
  // Display the QR code for "Marios Id"
  generateQRCode("http://172.20.10.2");
}
void generateQRCode(const char* text) {
  // Create a QR code object
  QRCode qrcode;
  
  // Define the size of the QR code (1-40, higher means bigger size)
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, text);

  // Clear the display
  display.clearDisplay();

  // Calculate the scale factor
  int scale = min(OLED_WIDTH / qrcode.size, OLED_HEIGHT / qrcode.size);
  
  // Calculate horizontal shift
  int shiftX = (OLED_WIDTH - qrcode.size*scale)/2;
  
  // Calculate horizontal shift
  int shiftY = (OLED_HEIGHT - qrcode.size*scale)/2;

  // Draw the QR code on the display
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(shiftX+x * scale, shiftY + y*scale, scale, scale, WHITE);
      }
    }
  }

  // Update the display
  display.display();
}
void loop(){
  WiFiClient client = server.available();   // 看有沒有外部裝置 client (如手機)，藉著瀏覽器 browser 連上 ESP32 web server
 
  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // 接收 client  的指令，點亮或關閉 連上 GPIO 的 LED。瀏覽器上，不同的按鈕，會發出不同的指令 (URLs requests)
            if (header.indexOf("GET /32/on") >= 0) {
              Serial.println("GPIO 32 on");
              output32State = "on";
              digitalWrite(output32, HIGH);
            } else if (header.indexOf("GET /32/off") >= 0) {
              Serial.println("GPIO 32 off");
              output32State = "off";
              digitalWrite(output32, LOW);
            }else if (header.indexOf("GET /33/on") >= 0) {
              Serial.println("GPIO 33 on");
              output33State = "on";
              digitalWrite(output33, HIGH);
            } else if (header.indexOf("GET /33/off") >= 0) {
              Serial.println("GPIO 33 off");
              output33State = "off";
              digitalWrite(output33, LOW);
            }  else if (header.indexOf("GET /34/on") >= 0) {
              Serial.println("GPIO 34 on");
              output34State = "on";
              digitalWrite(output34, HIGH);
            } else if (header.indexOf("GET /34/off") >= 0) {
              Serial.println("GPIO 34 off");
              output34State = "off";
              digitalWrite(output34, LOW);
            } else if (header.indexOf("GET /35/on") >= 0) {
              Serial.println("GPIO 35 on");
              output35State = "on";
              digitalWrite(output35, HIGH);
            } else if (header.indexOf("GET /35/off") >= 0) {
              Serial.println("GPIO 35 off");
              output35State = "off";
              digitalWrite(output35, LOW);
            } 
            
            // 設計 client 上的瀏覽器網頁格式，包括顏色、字型大小、有無邊框、字元等，這一部分是用 HTML 的語言寫成的
             //=========================================================================================
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" charset=\" utf-8\"/>");
            //client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<title>期中考全隊</title>");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 8px 20px;");
            client.println("text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}>");
            client.println(".wrap{width: 100%; height: 200px;display: flex;justify-content: space-between;}");
            client.println(".left{width: 50%;height: 200px;float:left;}");
            client.println(".right{width: 50%;height: 200px;float:right;}");
            client.println(".left2{width: 34%;height: 200px;float:left;}");
            client.println(".right2{width: 66%;height: 200px;float:right;}");
            client.println(".button { background-color: #4CAF50; border: none;color: white;padding: 16px 40px;text-decoration: none;font-size: 30px;margin: 2px;cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head");
            
            // Web Page Heading
           
            client.println("<body><h1>綠色健身房監控系統</h1>");

            if(Serial2.available()>0){        
              receivedNum = Serial2.parseInt(); //讀取整數
              Serial.print("receivedNum: ");
              Serial.println(receivedNum);
            
              in_1 = receivedNum/1000000;
              in_2 = receivedNum/1000%1000;
              in_7 = receivedNum%1000;

              in_1 = in_1/100;
              in_2 = in_2/100;
              in_3 = in_3/100;

              Serial.print("In_1: ");
              Serial.println(in_1);
              Serial.print("In_2: ");
              Serial.println(in_2);
              Serial.print("In_7: ");
              Serial.println(in_7);
            }

             if(in_1<3)in_1=3;
             else if(in_1>=4.5)in_1=4.5;
             if(in_2<3)in_2=3;
             else if(in_2>=4.5)in_2=4.5;
            power = abs(((in_1+in_2)-6)/4*25);
            power = power + 32;
            if(power>=100)power=100;
            String power_out = String(power);
            client.println("<h1>備用電量 "+ power_out +" %</h1>");
            
            client.println("<div class=\"wrap\"><div class=\"left\"><h2> 輸入</h2>");
            if (output32State=="off") {
              client.println("<p><a href=\"/32/on\"><button class=\"button button2\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/32/off\"><button class=\"button\">ON</button></a></p>");
            } 
            client.println("</div>");
            client.println("<div class=\"right\">");
            client.println("<h2> 設備</h2>");
            client.println("<div class=\"wrap\">");
            client.println("<div class=\"left\">");
            client.println("<h4>電池一</h4>");
            client.println("<h4>電池二</h4>");
            client.println("<h4>電池三</h4>");
            client.println("</div>");
            client.println("<div class=\"right\">");

            if(in_1!=0){
              
              String in_1_out = String(abs(in_1));
              client.println("<h4>"+ in_1_out +" V</h4>");
            }
            else{
              client.println("<h4>無資訊</h4>");
            }
            if(in_2!=0){
              String in_2_out = String(abs(in_2));
              client.println("<h4>"+ in_2_out +" V</h4>");
            }
            else{
              client.println("<h4>無資訊</h4>");
            }
            if(in_3!=0){
              String in_3_out = String(abs(in_3));
              client.println("<h4>"+ in_3_out +" V</h4>");
            }
            else{
              client.println("<h4>無資訊</h4>");
            }
            client.println("</div>");
            client.println("</div>");
            client.println("</div>");
            client.println("</div>");
            client.println("<div class=\"wrap\">");
            client.println("<div class=\"left\">");
              in_4=56.4;
              in_5=37.3;
              in_6=23.2;
              in_7 = 0.0;
              if(output33State=="on"){
                  in_7 += in_4;
              }
              if(output34State=="on"){
                  in_7 += in_5;
              }
              if(output35State=="on"){
                  in_7 += in_6;
              }
            String in_7_out = String(in_7);
            client.println("<h2> 輸出，消耗: "+ in_7_out +" mW</h2>");
            if(output33State=="on" && output34State=="off" && output35State=="off"){
              client.println("<img src=\"https://raw.githubusercontent.com/Liweizhe0411/0427_pic/main/設備一.png\" width=\"100%\" height=\"100%\" alt=\"圖片2\">");
            }
            else if(output33State=="on" && output34State=="on" && output35State=="off"){
              client.println("<img src=\"https://raw.githubusercontent.com/Liweizhe0411/0427_pic/main/設備一二.png\" width=\"100%\" height=\"100%\" alt=\"圖片3\">");
            }
            else if(output33State=="on" && output34State=="on" && output35State=="on"){
              client.println("<img src=\"https://raw.githubusercontent.com/Liweizhe0411/0427_pic/main/設備一二燈.png\" width=\"100%\" height=\"100%\" alt=\"圖片4\">");
            }
            else if(output33State=="off" && output34State=="on" && output35State=="off"){
              client.println("<img src=\"https://raw.githubusercontent.com/Liweizhe0411/0427_pic/main/設備二.png\" width=\"100%\" height=\"100%\" alt=\"圖片5\">");
            }
            else if(output33State=="off" && output34State=="off" && output35State=="on"){
              client.println("<img src=\"https://raw.githubusercontent.com/Liweizhe0411/0427_pic/main/燈.png\" width=\"100%\" height=\"100%\" alt=\"圖片6\">");
            }
            else if(output33State=="on" && output34State=="off" && output35State=="on"){
              client.println("<img src=\"https://raw.githubusercontent.com/Liweizhe0411/0427_pic/main/設備一燈.png\" width=\"100%\" height=\"100%\" alt=\"圖片7\">");
            }
            else if(output33State=="off" && output34State=="on" && output35State=="on"){
              client.println("<img src=\"https://raw.githubusercontent.com/Liweizhe0411/0427_pic/main/設備二燈.png\" width=\"100%\" height=\"100%\" alt=\"圖片8\">");
            }
            else{
              client.println("<img src=\"https://raw.githubusercontent.com/Liweizhe0411/0427_pic/main/底圖.png\" width=\"100%\" height=\"100%\" alt=\"圖片1\">");
            }
            client.println("</div>");
            client.println("<div class=\"right\">");
            client.println("<h2>設備</h2>");
            client.println("<div class=\"wrap\">");
            client.println("<div class=\"left\">");
            if (output33State=="off") {
              client.println("<p><a href=\"/33/on\"><button style=\"width=\" 100%\"\">設備一OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/33/off\"><button style=\"width=\" 100%\"\">設備一ON</button></a></p>");
            } if (output34State=="off") {
              client.println("<p><a href=\"/34/on\"><button style=\"width=\" 100%\"\">設備二OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/34/off\"><button style=\"width=\" 100%\"\">設備二ON</button></a></p>");
            } if (output35State=="off") {
              client.println("<p><a href=\"/35/on\"><button style=\"width=\" 100%\"\">設備三OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/35/off\"><button style=\"width=\" 100%\"\">設備三ON</button></a></p>");
            } 
            client.println("</div>");
            client.println("<div class=\"right\">");
          
             String in4_out = String(in_4);
             String in5_out = String(in_5);
             String in6_out = String(in_6);
             if(in_4>0 && output33State=="on"){
                client.println("<h4>"+ in4_out +"mW</h4>");
             }else{
                client.println("<h4>0mW</h4>");
             }
             if(in_5>0 && output34State=="on"){
                client.println("<h4>"+ in5_out +"mW</h4>");
             }else{
                client.println("<h4>0mW</h4>");
             }
             if(in_6>0 && output35State=="on"){
                client.println("<h4>"+ in6_out +"mW</h4>");
             }else{
                client.println("<h4>0mW</h4>");
             }
            client.println("</div>");
            client.println("</div>");
            client.println("</div>");
            client.println("</div>");
            

            /*
            // Display current state, and ON/OFF buttons for GPIO 26  
            client.println("<p>GPIO 26 - State " + output26State + "</p>");
            String in_1_out = String(in_1);
            client.println("<p>GPIO 26 - State " + in_1_out + "</p>");
            // If the output26State is off, it displays the ON button       
            if (output26State=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 27  
            client.println("<p>GPIO 27 - State " + output27State + "</p>");
            // If the output27State is off, it displays the ON button       
            if (output27State=="off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            */
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
