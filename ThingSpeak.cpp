#include "mbed.h"
#include "http_request.h"
#include "ESP32Interface.h"
#include "PrintfWrapper.h"

//#define NO_NETWORK

static ESP32Interface wifi(P5_3, P3_14, P7_1, P0_1);
static Serial pc(USBTX, USBRX);

static const char* ssid = "W04_78625678F45E";//書き換えてください
static const char* password = "6dqbfjgfyb6gi22";//書き換えてください

int wifi_init() {
#ifndef NO_NETWORK
    printf2("\nConnecting...\n");
    int ret = wifi.connect(ssid, password, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
    	printf2("\nConnection error\n");
        return -1;
    }
    printf2("Success\n\n");
    printf2("MAC: %s\n", wifi.get_mac_address());
    printf2("IP: %s\n", wifi.get_ip_address());
    printf2("Netmask: %s\n", wifi.get_netmask());
    printf2("Gateway: %s\n", wifi.get_gateway());
    printf2("RSSI: %d\n\n", wifi.get_rssi());
#endif
    return 0;
}

void ChannelUpdate(float field1, float field2)
{
#ifndef NO_NETWORK
	char* thingSpeakUrl = "http://api.thingspeak.com/update";
	char* thingSpeakKey = "DAUNON43Q966D9XS";

	char urlBuffer[256];

	urlBuffer[0] = 0;

	sprintf(urlBuffer, "%s?key=%s&field1=%05.1f&field2=%05.1f", thingSpeakUrl, thingSpeakKey, field1, field2);

	printf2("Request to %s\r\n", urlBuffer);
	printf2("\n----- HTTP GET request -----\n");

    HttpRequest* get_req = new HttpRequest(&wifi, HTTP_GET, urlBuffer);

    HttpResponse* get_res = get_req->send();
    if (!get_res) {
    	printf2("HttpRequest failed (error code %d)\n", get_req->get_error());
    	return;
    }
    printf2("\n----- HTTP GET response -----\n");

    printf2("%s\n", get_res->get_body_as_string().c_str());
    delete get_req;
#endif
}


//http://maker.ifttt.com/trigger/NotifyWeight/with/key/eR7bp41z1wV8xVngdINFIojxchslWUMUyBbArjE564Z?value1=123&value2=321

char *base = "http://maker.ifttt.com/trigger/";
char *key = "/with/key/eR7bp41z1wV8xVngdINFIojxchslWUMUyBbArjE564Z";
char *event[7] = {"NotifyDietAchieved","NotifyDietNotAchieved","NotifyUnlockFailedInvalidPass","NotifyUnlockFailedDoDiet","NotifyChangeTarget", "NotifyLocked", "NotifyUnlocked"};

void NotifyDietAchieved(float weight, float target)
{
#ifndef NO_NETWORK
	char urlBuffer[256];

	urlBuffer[0] = 0;

	sprintf(urlBuffer, "%s%s%s?value1=%.1f&value2=%.1f", base, event[0], key, target, weight);

	printf2("Request to %s\r\n", urlBuffer);
	printf2("\n----- HTTP GET request -----\n");

    HttpRequest* get_req = new HttpRequest(&wifi, HTTP_GET, urlBuffer);

    HttpResponse* get_res = get_req->send();
    if (!get_res) {
    	printf2("HttpRequest failed (error code %d)\n", get_req->get_error());
    	return;
    }
    printf2("\n----- HTTP GET response -----\n");

    printf2("%s\n", get_res->get_body_as_string().c_str());
    delete get_req;
#endif
}

void NotifyDietNotAchieved(float weight, float target, float diff)
{
#ifndef NO_NETWORK
	char urlBuffer[256];

	urlBuffer[0] = 0;

	sprintf(urlBuffer, "%s%s%s?value1=%.1f&value2=%.1f&value3=%.1f", base, event[1], key, target, weight, diff);

	printf2("Request to %s\r\n", urlBuffer);
	printf2("\n----- HTTP GET request -----\n");

    HttpRequest* get_req = new HttpRequest(&wifi, HTTP_GET, urlBuffer);

    HttpResponse* get_res = get_req->send();
    if (!get_res) {
    	printf2("HttpRequest failed (error code %d)\n", get_req->get_error());
    	return;
    }
    printf2("\n----- HTTP GET response -----\n");

    printf2("%s\n", get_res->get_body_as_string().c_str());
    delete get_req;
#endif
}



void NotifyUnlockFailedInvalidPass(void)
{
#ifndef NO_NETWORK
	char urlBuffer[256];

	urlBuffer[0] = 0;

	sprintf(urlBuffer, "%s%s%s", base, event[2], key);

	printf2("Request to %s\r\n", urlBuffer);
	printf2("\n----- HTTP GET request -----\n");

    HttpRequest* get_req = new HttpRequest(&wifi, HTTP_GET, urlBuffer);

    HttpResponse* get_res = get_req->send();
    if (!get_res) {
    	printf2("HttpRequest failed (error code %d)\n", get_req->get_error());
    	return;
    }
    printf2("\n----- HTTP GET response -----\n");

    printf2("%s\n", get_res->get_body_as_string().c_str());
    delete get_req;
#endif
}


void NotifyUnlockFailedDoDiet(void)
{
#ifndef NO_NETWORK
	char urlBuffer[256];

	urlBuffer[0] = 0;

	sprintf(urlBuffer, "%s%s%s", base, event[3], key);

	printf2("Request to %s\r\n", urlBuffer);
	printf2("\n----- HTTP GET request -----\n");

    HttpRequest* get_req = new HttpRequest(&wifi, HTTP_GET, urlBuffer);

    HttpResponse* get_res = get_req->send();
    if (!get_res) {
    	printf2("HttpRequest failed (error code %d)\n", get_req->get_error());
    	return;
    }
    printf2("\n----- HTTP GET response -----\n");

    printf2("%s\n", get_res->get_body_as_string().c_str());
    delete get_req;
#endif
}



void NotifyChangeTarget(int old_target, int new_target)
{
#ifndef NO_NETWORK
	char urlBuffer[256];

	urlBuffer[0] = 0;

	sprintf(urlBuffer, "%s%s%s?value1=%d&value2=%d", base, event[4], key, old_target, new_target);

	printf2("Request to %s\r\n", urlBuffer);
	printf2("\n----- HTTP GET request -----\n");

    HttpRequest* get_req = new HttpRequest(&wifi, HTTP_GET, urlBuffer);

    HttpResponse* get_res = get_req->send();
    if (!get_res) {
    	printf2("HttpRequest failed (error code %d)\n", get_req->get_error());
    	return;
    }
    printf2("\n----- HTTP GET response -----\n");

    printf2("%s\n", get_res->get_body_as_string().c_str());
    delete get_req;
#endif
}

void NotifyLocked(void)
{
#ifndef NO_NETWORK
	char urlBuffer[256];

	urlBuffer[0] = 0;

	sprintf(urlBuffer, "%s%s%s", base, event[5], key);

	printf2("Request to %s\r\n", urlBuffer);
	printf2("\n----- HTTP GET request -----\n");

    HttpRequest* get_req = new HttpRequest(&wifi, HTTP_GET, urlBuffer);

    HttpResponse* get_res = get_req->send();
    if (!get_res) {
    	printf2("HttpRequest failed (error code %d)\n", get_req->get_error());
    	return;
    }
    printf2("\n----- HTTP GET response -----\n");

    printf2("%s\n", get_res->get_body_as_string().c_str());
    delete get_req;
#endif
}

void NotifyUnlocked(void)
{
#ifndef NO_NETWORK
	char urlBuffer[256];

	urlBuffer[0] = 0;

	sprintf(urlBuffer, "%s%s%s", base, event[6], key);

	printf2("Request to %s\r\n", urlBuffer);
	printf2("\n----- HTTP GET request -----\n");

    HttpRequest* get_req = new HttpRequest(&wifi, HTTP_GET, urlBuffer);

    HttpResponse* get_res = get_req->send();
    if (!get_res) {
    	printf2("HttpRequest failed (error code %d)\n", get_req->get_error());
    	return;
    }
    printf2("\n----- HTTP GET response -----\n");

    printf2("%s\n", get_res->get_body_as_string().c_str());
    delete get_req;
#endif
}
