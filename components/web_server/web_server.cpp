#include "web_server.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include <string.h>

static const char* TAG = "WEB_SERVER";
static httpd_handle_t server = NULL;

// بيانات الحساسات
static float s_temp = 0, s_hum = 0, s_weight = 0, s_batt = 0;
static int s_sound = 0, s_gas = 0, s_uv = 0, s_vibration = 0;
static bool s_motion_entrance = false, s_motion_inside = false;
static int s_colony = 0, s_health = 0;
static float s_top = 0, s_mid = 0, s_bottom = 0;

// صفحة HTML مضمنة بسيطة (مع AJAX Polling بدلاً من WebSocket)
static const char* HTML_PAGE = 
"<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1.0'>"
"<title>Smart Hive</title><style>"
"body{background:#0a0a0a;color:#fff;font-family:Arial;text-align:center;padding:20px;}"
"h1{color:#ffc107;}.card{background:#111;border:1px solid #ffc107;border-radius:15px;padding:20px;margin:10px;display:inline-block;width:180px;}"
".value{font-size:2rem;color:#ffc107;font-weight:bold;}"
"</style>"
"<script>"
"async function fetchData(){"
"try{const r=await fetch('/api/data');const d=await r.json();"
"document.getElementById('temp').innerHTML=d.temp;"
"document.getElementById('hum').innerHTML=d.hum;"
"document.getElementById('weight').innerHTML=d.weight;"
"document.getElementById('batt').innerHTML=d.batt;"
"document.getElementById('top').innerHTML=d.top_temp;"
"document.getElementById('mid').innerHTML=d.mid_temp;"
"document.getElementById('bottom').innerHTML=d.bottom_temp;"
"document.getElementById('sound').innerHTML=d.sound;"
"document.getElementById('gas').innerHTML=d.gas;"
"document.getElementById('uv').innerHTML=d.uv;"
"document.getElementById('vib').innerHTML=d.vibration;"
"if(d.temp>40)document.getElementById('temp').style.color='#ff3e3e';"
"else document.getElementById('temp').style.color='#ffc107';"
"}catch(e){}}"
"setInterval(fetchData,2000);fetchData();"
"</script>"
"</head><body>"
"<h1>🐝 SMART HIVE MASTER</h1>"
"<div class='card'><div>🌡️ الحرارة</div><div class='value' id='temp'>--</div><div>°C</div></div>"
"<div class='card'><div>💧 الرطوبة</div><div class='value' id='hum'>--</div><div>%</div></div>"
"<div class='card'><div>⚖️ الوزن</div><div class='value' id='weight'>--</div><div>kg</div></div>"
"<div class='card'><div>🔋 البطارية</div><div class='value' id='batt'>--</div><div>V</div></div>"
"<div class='card'><div>📢 الصوت</div><div class='value' id='sound'>--</div><div>dB</div></div>"
"<div class='card'><div>💨 الغاز</div><div class='value' id='gas'>--</div><div>ppm</div></div>"
"<div class='card'><div>☀️ UV</div><div class='value' id='uv'>--</div><div>%</div></div>"
"<div class='card'><div>📳 الاهتزاز</div><div class='value' id='vib'>--</div></div>"
"<div class='card'><div>⬆️ حرارة عليا</div><div class='value' id='top'>--</div><div>°C</div></div>"
"<div class='card'><div>👑 حرارة وسطى</div><div class='value' id='mid'>--</div><div>°C</div></div>"
"<div class='card'><div>⬇️ حرارة سفلى</div><div class='value' id='bottom'>--</div><div>°C</div></div>"
"<div style='margin-top:20px;color:#888;'>SMART HIVE | تحديث تلقائي</div>"
"</body></html>";

static esp_err_t root_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, HTML_PAGE, strlen(HTML_PAGE));
    return ESP_OK;
}

static esp_err_t api_handler(httpd_req_t *req) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "temp", s_temp);
    cJSON_AddNumberToObject(root, "hum", s_hum);
    cJSON_AddNumberToObject(root, "weight", s_weight);
    cJSON_AddNumberToObject(root, "batt", s_batt);
    cJSON_AddNumberToObject(root, "sound", s_sound);
    cJSON_AddNumberToObject(root, "gas", s_gas);
    cJSON_AddNumberToObject(root, "uv", s_uv);
    cJSON_AddNumberToObject(root, "vibration", s_vibration);
    cJSON_AddNumberToObject(root, "top_temp", s_top);
    cJSON_AddNumberToObject(root, "mid_temp", s_mid);
    cJSON_AddNumberToObject(root, "bottom_temp", s_bottom);
    cJSON_AddBoolToObject(root, "motion_entrance", s_motion_entrance);
    cJSON_AddBoolToObject(root, "motion_inside", s_motion_inside);
    cJSON_AddNumberToObject(root, "colony", s_colony);
    cJSON_AddNumberToObject(root, "health", s_health);
    
    char* json = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, strlen(json));
    cJSON_Delete(root);
    free(json);
    return ESP_OK;
}

bool start_web_server(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    
    config.server_port = 80; 
    config.stack_size = 10240;      // ذاكرة أكبر لمعالجة الـ JSON
    config.max_uri_handlers = 10;   // لضمان عدم رفض الروابط
    config.lru_purge_enable = true; // مسح الاتصالات الميتة فوراً
    
    // نزيد وقت الانتظار لمنع الـ Bad Gateway في المتصفحات البطيئة
    config.recv_wait_timeout = 30;
    config.send_wait_timeout = 30;

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // تسجيل الروابط
        httpd_uri_t root = { .uri = "/", .method = HTTP_GET, .handler = root_handler };
        httpd_register_uri_handler(server, &root);
        
        httpd_uri_t api = { .uri = "/api/data", .method = HTTP_GET, .handler = api_handler };
        httpd_register_uri_handler(server, &api);
        
        return true;
    }
    ESP_LOGE(TAG, "❌ Failed to start server!");
    return false;
}

void stop_web_server(void) { if (server) httpd_stop(server); server = NULL; }
const char* get_server_ip(void) { return "192.168.4.1"; }

void web_server_update_data(float temp, float hum, float weight, float batt, 
                            int sound, int gas, int uv, int vibration,
                            bool motion_entrance, bool motion_inside,
                            int colony_strength, int health_score,
                            float top_temp, float mid_temp, float bottom_temp) {
    s_temp = temp; s_hum = hum; s_weight = weight; s_batt = batt;
    s_sound = sound; s_gas = gas; s_uv = uv; s_vibration = vibration;
    s_motion_entrance = motion_entrance; s_motion_inside = motion_inside;
    s_colony = colony_strength; s_health = health_score;
    s_top = top_temp; s_mid = mid_temp; s_bottom = bottom_temp;
}
