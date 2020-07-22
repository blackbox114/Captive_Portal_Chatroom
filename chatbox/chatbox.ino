/***********************************************************
  项目名：强制弹窗WIFI聊天室
  作者：BlackBox114
  当前版本描述：
  开发板：NodeMcu(ESP-12E)
  手机连接上AP点“小黑屋”后，会强制进行弹窗弹出聊天室界面
  可输入ID进行聊天。聊天信息会储存，如果不手动删除，掉电后还会存在
  双击顶部“小黑屋”可以修改ID和清屏
  连接热点后访问10.10.10.1也可访问聊天室
  修改人：BlackBox114
  修改时间：2019.4.27
  修改内容：html写到独立的文件chat.html
  修改时间：2019.5.15
  修改内容：增加删除聊天记录的功能
  修改时间：2019.5.19
  修改内容：增加了修改ID的功能
  修改时间：2019.6.2
  修改内容：增加了掉电保存聊天记录的功能
  接线：无
  /***********************************************************/
#include <ESP8266WiFi.h>
#include "./DNSServer.h"
#include <ESP8266WebServer.h>
#include "FS.h"
const byte        DNS_PORT = 53;
int num = 0;

const String      messagesFile = "/messages.txt";
const String      chatFile = "/chat.html";
const char*       wifiName = "陈俊霏的个人聊天室";
String            chatHtml;

IPAddress         apIP(10, 10, 10, 1);//定死的IP地址
DNSServer         dnsServer;
ESP8266WebServer  webServer(80);
void WELCOME() {
  Serial.println(" ");
  Serial.println(" = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =");
  Serial.println("||     ____     __                    __      ____                   ||");
  Serial.println("||    / __ )   / /  ____ _   _____   / /__   / __ )   ____     _  __ ||");
  Serial.println("||   / __  |  / /  / __ `/  / ___/  / //_/  / __  |  / __ \\   | |/_/ ||");
  Serial.println("||  / /_/ /  / /  / /_/ /  / /__   / ,<    / /_/ /  / /_/ /  _>  <   ||");
  Serial.println("|| /_____/  /_/   \\__,_/   \\___/  /_/|_|  /_____/   \\____/  /_/|_|   ||");
  Serial.println(" = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =");
  Serial.println(" ");
  Serial.println("小黑屋聊天室启动");
  Serial.println("客户端连接后会持续输出客户端尝试ping通的网址");
  Serial.println("正在等待客户端连接...");
}
void setup() {
  Serial.begin(115200);//初始化串口
  SPIFFS.begin();//初始化NodeMCU 闪存文件系统。记得使用sketch upload工具提前写入文件
  Serial.println("");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(wifiName);
  dnsServer.start(DNS_PORT, "*", apIP);//表示让所有的域名都指向 apIP(10, 10, 10, 1)这个IP地址，* 号表示所有域名
  chatHtml = fileRead(chatFile);
  webServer.begin();
  setupAppHandlers();
  handleSendMessage();
  showChatPage();
  WELCOME();//串口打印欢迎页面
}

void handleSendMessage() {
  if (webServer.hasArg("message")) {
    String message = webServer.arg("message");
    fileWrite(messagesFile, message + "\n" , "a+");
    webServer.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "text/plain", "Message Sent");
  }
}

void handleClearMessages() {
  SPIFFS.remove(messagesFile);
  webServer.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "text/plain", "File Deleted");
}

void showChatPage() {//主页面
  webServer.send(200, "text/html", chatHtml);
}

void showMessages() {
  String messages = fileRead(messagesFile);
  webServer.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "text/plain", messages);
}

void setupAppHandlers() {
  webServer.onNotFound([]() {
    showChatPage();
  });
  webServer.on("10.10.10.1", showChatPage);//开启主页面
  webServer.on("/sendMessage", handleSendMessage);//发送消息
  webServer.on("/readMessages", showMessages);//显示消息
  webServer.on("/clearMessages", handleClearMessages);//清除聊天室内的所有消息
}

String fileRead(String name) {
  String contents;
  int i;
  File file = SPIFFS.open(name, "a+");
  for (i = 0; i < file.size(); i++)
  {
    contents += (char)file.read();
  }
  file.close();
  return contents;
}

void fileWrite(String name, String content, String mode) {
  File file = SPIFFS.open(name.c_str(), mode.c_str());
  file.write((uint8_t *)content.c_str(), content.length());
  file.close();
}

void loop() {
  num++;
  dnsServer.processNextRequest();
  webServer.handleClient();
  if (num == 100000) {
    Serial.printf("聊天室内设备数量 = %d\n", WiFi.softAPgetStationNum());
    num = 0;
  }

}
