#include <WebServer.h> // Biblioteca para la creación del servidor web y usar parametros http cliente
#include <WiFi.h> // Biblioteca que permite que la board se conecte a internet.
#include <esp32cam.h>// Biblioteca para poder utilizar la camara OV2640 del esp32


// Select camera model 
#define CAMERA_MODEL_WROVER_KIT 
//#define CAMERA_MODEL_ESP_EYE 
//#define CAMERA_MODEL_M5STACK_PSRAM 
//#define CAMERA_MODEL_M5STACK_WIDE 
//#define CAMERA_MODEL_AI_THINKER

// ESTE PROGRAMA ENVIA IMAGEN SI SE COLOCA EN IP WEB, PERO SI SE COLOCA EN PYTHON ENVIA VIDEO POR LAS ITERACIONES. . . (SI FUNCIONA EN PYTHON)
const char* WIFI_SSID = "Edwil Aguilar Gonzalez 19";
const char* WIFI_PASS = "edwilaguilar18";

#if defined(CAMERA_MODEL_WROVER_KIT)
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    21
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      19
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM       5
#define Y2_GPIO_NUM       4
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

#elif defined(CAMERA_MODEL_M5STACK_PSRAM)
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    15
#define XCLK_GPIO_NUM     27
#define SIOD_GPIO_NUM     25
#define SIOC_GPIO_NUM     23

#define Y9_GPIO_NUM       19
#define Y8_GPIO_NUM       36
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       39
#define Y5_GPIO_NUM        5
#define Y4_GPIO_NUM       34
#define Y3_GPIO_NUM       35
#define Y2_GPIO_NUM       32
#define VSYNC_GPIO_NUM    22
#define HREF_GPIO_NUM     26
#define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#else
#error "Camera model not selected"
#endif



WebServer server(80); //servidor en el puerto 80

static auto loRes = esp32cam::Resolution::find(320, 240); //baja resolucion
static auto hiRes = esp32cam::Resolution::find(800, 600); //alta resolucion 
//static auto hiRes = esp32cam::Resolution::find(640, 480); //alta resolucion  (para tazas de fps) (IP CAM APP)


void
serveJpg() //captura imagen .jpg
{
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);  // y envia a un cliente (en este caso sera python)
}

void
handleJpgLo()  //permite enviar la resolucion de imagen baja
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}

void
handleJpgHi() //permite enviar la resolucion de imagen alta
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}



void
setup()
{
  Serial.begin(115200);
  Serial.println();

  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMARA OK" : "CAMARA FAIL");
  }

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS); //nos conectamos a la red wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/cam-lo.jpg");//para conectarnos IP res baja

  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/cam-hi.jpg");//para conectarnos IP res alta

  server.on("/cam-lo.jpg",handleJpgLo);//enviamos al servidor
  server.on("/cam-hi.jpg", handleJpgHi);

  server.begin();
}

void loop()
{
  server.handleClient();
}
