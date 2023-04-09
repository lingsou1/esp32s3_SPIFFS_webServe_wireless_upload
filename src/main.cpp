/*
接线说明:无

程序说明:建立网络服务器，允许用户通过网页将文件上传到SPIFFS

注意事项:(1)在下面的网站中有SPIFFS的下载方法:
         https://docs.platformio.org/en/latest/platforms/espressif32.html#uploading-files-to-file-system
         在下载程序之前要记得先把 data文件夹中的内容下载到 esp32s3的闪存中去,即先 Build Filesystem Image (编译),
         再 upload Filesystem Image(下载)即可
         相关的内容放在esp32s3中的闪存文件中(即 data 文件夹);网站显示的内容通过data文件夹中的文件决定
         (2)上传的文件大小未知,但是401KB的图片上传成功了
         (只是在SPIFFS中检测到了,但是在网页中没有出现成功的页面,而是出现了重连的提示)
         


函数示例:无

作者:灵首

时间:2023_3_15

*/

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <SPIFFS.h>



WiFiMulti wifi_multi;  //建立WiFiMulti 的对象,对象名称是 wifi_multi
WebServer esp32s3_webServe(80);//实例化一个网页服务的对象,端口是80

File spiffsUploadFile;              // 建立文件对象用于闪存文件上传


//相关函数的声明,要先声明了VSCODE才不会报错,否则会报错的
void handleUserRequest();
String getContentType(String filename);
bool handleFileRead(String path);
//上面的三个函数是在使用SPIFF,SwebServe中通用的(都要写)

//这两个函数用来处理文件上传的
void handleFileUpload();
void respondOK();




/*
# brief 连接WiFi的函数
# param 无
# retval 无
*/
void wifi_multi_con(void){
  int i=0;
  while(wifi_multi.run() != WL_CONNECTED){
    delay(1000);
    i++;
    Serial.print(i);
  }

}



/*
# brief 写入自己要连接的WiFi名称及密码,之后会自动连接信号最强的WiFi
# param 无
# retval  无
*/
void wifi_multi_init(void){
  wifi_multi.addAP("LINGSOU123","12345678");
  wifi_multi.addAP("LINGSOU12","12345678");
  wifi_multi.addAP("LINGSOU1","12345678");
  wifi_multi.addAP("LINGSOU234","12345678");   //通过 wifi_multi.addAP() 添加了多个WiFi的信息,当连接时会在这些WiFi中自动搜索最强信号的WiFi连接
}



/*
# brief 启动SPIFFS
# param 无
# retval 无
*/
void SPIFFS_start_init(){
  if(SPIFFS.begin()){
    Serial.print("\nSPIFFS Start!!!");
  }
  else{
    Serial.print("\nSPIFFS Failed to start!!!");
  }
}



/*
# brief esp32s3建立网页服务,使用esp32s3_webServe.on()函数处理上传文件的需求,
        我只是知道是这个函数,但不知道具体的使用方法

# param 无
# retval  无
*/
void esp32s3_webServe_init(void){
  esp32s3_webServe.on("/upload.html",   // 如果客户端通过upload页面
                    HTTP_POST,        // 向服务器发送文件(请求方法POST)
                    respondOK,        // 则回复状态码 200 给客户端
                    handleFileUpload);// 并且运行处理文件上传函数
  esp32s3_webServe.onNotFound(handleUserRequest);
  esp32s3_webServe.begin();   //开启网页服务
  Serial.print("\nHTTp esp32s3_webServe started");
}



/*
# brief   获取文件类型
# param   String filename:所请求的文件的文件名
# retval    String:返回字符串,返回该文件的文件类型(就是后缀名)
*/
String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}



/*
# brief   处理浏览器HTTP访问,在闪存文件中查询请求的文件

# param   String path:这是用户请求的文件路径,不需要手动输入,例如:
    (
    String webAddress = esp32s3_webServe.uri();   // 获取用户请求网址信息
    bool fileReadOK = handleFileRead(webAddress);   // 通过handleFileRead函数处处理用户访问
    )
    通过形如此类的调用获取(应该是的)

# retval bool型返回,当在闪存文件中查询到存在请求的文件时返回: true,否则返回:false
*/
bool handleFileRead(String path) {            //

  if (path.endsWith("/")) {                   // 如果访问地址以"/"为结尾
    path = "/index.html";                     // 则将访问地址修改为/index.html便于SPIFFS访问
  } 
  
  String contentType = getContentType(path);  // 获取文件类型
  
  //在SPIFFS中查找访问的文件,若有则打开文件并返回给服务器
  if (SPIFFS.exists(path)) {                     
    File file = SPIFFS.open(path, "r");          
    esp32s3_webServe.streamFile(file, contentType);
    file.close();                               
    return true;                                
  }
  return false;                                  
}




/*
# brief 获取用户请求的网址信息,并处理用户浏览器的HTTP访问
# param   无
# retval    无
*/
void handleUserRequet() {    
  // 获取用户请求网址信息     
  String reqResource = esp32s3_webServe.uri();   
  Serial.print("reqResource: ");
  Serial.println(reqResource);

  // 通过handleFileRead函数处处理用户访问
  bool fileReadOK = handleFileRead(reqResource);   

  // 如果在SPIFFS无法找到用户访问的资源，则回复404 (Not Found)
  if (!fileReadOK){                                                 
    esp32s3_webServe.send(404, "text/plain", "404 Not Found"); 
  }
}



/*
# brief  处理上传文件函数
# param 无
# retval 无
*/
void handleFileUpload(){  
  
  HTTPUpload& upload = esp32s3_webServe.upload();
  
  if(upload.status == UPLOAD_FILE_START){                     // 如果上传状态为UPLOAD_FILE_START
    
    String filename = upload.filename;                        // 建立字符串变量用于存放上传文件名
    if(!filename.startsWith("/")) filename = "/" + filename;  // 为上传文件名前加上"/"
    Serial.println("File Name: " + filename);                 // 通过串口监视器输出上传文件的名称

    spiffsUploadFile = SPIFFS.open(filename, "w");            // 在SPIFFS中建立文件用于写入用户上传的文件数据
    
  } else if(upload.status == UPLOAD_FILE_WRITE){          // 如果上传状态为UPLOAD_FILE_WRITE      
    
    if(spiffsUploadFile)
      spiffsUploadFile.write(upload.buf, upload.currentSize); // 向SPIFFS文件写入浏览器发来的文件数据
      
  } else if(upload.status == UPLOAD_FILE_END){            // 如果上传状态为UPLOAD_FILE_END 
    if(spiffsUploadFile) {                                    // 如果文件成功建立
      spiffsUploadFile.close();                               // 将文件关闭
      Serial.println(" Size: "+ upload.totalSize);        // 通过串口监视器输出文件大小
      esp32s3_webServe.sendHeader("Location","/success.html");  // 将浏览器跳转到/success.html（成功上传页面）
      esp32s3_webServe.send(303);                               // 发送相应代码303（重定向到新页面） 
    } else {                                              // 如果文件未能成功建立
      Serial.println("File upload failed");               // 通过串口监视器输出报错信息
      esp32s3_webServe.send(500, "text/plain", "500: couldn't create file"); // 向浏览器发送相应代码500（服务器错误）
    }    
  }
}


/*
# brief   回复状态码 200 给客户端
# param 无
# retval 无
*/
void respondOK(){
  esp32s3_webServe.send(200);
}



void setup() {
  //启动串口通信
  Serial.begin(9600);        
  Serial.println("");

  
  //WiFi设置
  wifi_multi_init();//储存多个WiFi
  wifi_multi_con();//自动连接WiFi

  
  //输出连接信息(连接的WIFI名称及开发板的IP地址)
  Serial.print("\nconnect wifi:");
  Serial.print(WiFi.SSID());
  Serial.print("\n");
  Serial.print("\nIP address:");
  Serial.print(WiFi.localIP());
  Serial.print("\n");
       
  //开启闪存文件系统
  SPIFFS_start_init();
  
  

    //开启网页服务器功能
  esp32s3_webServe_init();


}

void loop() {
  esp32s3_webServe.handleClient();
}

