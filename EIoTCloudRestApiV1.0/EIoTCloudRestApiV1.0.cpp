/*
 Created by Igor Jarc
 See http://iot-playground.com for details
 Please use community forum on website do not contact author directly

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include "EIoTCloudRestApiV1.0.h"
#include <Arduino.h>

EIoTCloudRestApi::EIoTCloudRestApi()
{
}

void EIoTCloudRestApi::begin(const char *ssid, const char *password)
{
	begin(ssid, password, "");
}

void EIoTCloudRestApi::begin(const char *ssid, const char *password, String token)
{
	_ssid = ssid;
	_password = password;
	_token = token;

	wifiConnect();
}

String EIoTCloudRestApi::TokenNew(String instance)
{
	String ret = "";

	WiFiClient client;

	while (!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT))
	{
		DEBUG_PRINTLN("connection failed");
		wifiConnect();
	}

	DEBUG_PRINTLN("\r\nGET /RestApi/v1.0/Token/New\r\n");

	client.print(String("POST /RestApi/v1.0/Token/New HTTP/1.1\r\n") +
				 "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" +
				 "EIOT-Instance: " + String(instance) + "\r\n" +
				 "Connection: close\r\n" +
				 "Content-Length: 0\r\n" +
				 "\r\n");

	while (!client.available())
		;
	//delay(300);
	String line = "";
	while (client.available())
	{
		line += client.readStringUntil('\r');
	}

#ifdef DEBUG
	char buff[300];
	line.toCharArray(buff, 300);
	DEBUG_PRINTLN(buff);
#endif

	int pos = 0;

	while ((pos = line.indexOf('{', pos)) != -1)
	{
		if (line.substring(pos, pos + 10) == "{\"Token\":\"")
		{
			int ix1 = line.indexOf("\"}", pos + 11);

			if (ix1 != -1)
			{
				ret = line.substring(pos + 10, ix1);
#ifdef DEBUG
				DEBUG_PRINTLN("\r\n");
				ret.toCharArray(buff, 300);
				DEBUG_PRINTLN(buff);
				DEBUG_PRINTLN("\r\n");
#endif
			}
			break;
		}
		else
			pos++;
	}

	_token = ret;

	return ret;
}

// list all tokens
bool EIoTCloudRestApi::TokenList(String instance, int *ptrTokenCnt, String **ptrArr)
{
	WiFiClient client;

	while (!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT))
	{
		DEBUG_PRINTLN("connection failed");
		wifiConnect();
	}

	DEBUG_PRINTLN("GET TokenList: ");
#ifdef DEBUG
	char buff[300];
	//url.toCharArray(buff, 300);
	//DEBUG_PRINTLN(buff);
#endif

	client.print(String("GET /RestApi/v1.0/TokenList HTTP/1.1\r\n") +
				 "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" +
				 "EIOT-Instance: " + String(instance) + "\r\n" +
				 "Connection: close\r\n" +
				 "Content-Length: 0\r\n" +
				 "\r\n");

	while (!client.available())
		;
	//delay(300);
	String line = "";
	while (client.available())
	{
		line += client.readStringUntil('\r');
#ifdef DEBUG
//  line.toCharArray(buff, 300);
//  DEBUG_PRINTLN(buff);
#endif
	}

	int ix = line.indexOf('[');

	if (ix != -1)
		line = line.substring(ix);

	int tokenCnt = 0;
	int pos = 0;

	while ((pos = line.indexOf('T', pos)) != -1)
	{
		if (line.substring(pos, pos + 8) == "Token\":\"")
			tokenCnt++;
		pos++;
	}
	Serial.println(tokenCnt);

	String tokens[tokenCnt];

	*ptrTokenCnt = tokenCnt;
	ptrArr = (String **)&tokens;

	pos = 0;
	int ix1;
	int i = 0;
	while ((pos = line.indexOf('T', pos)) != -1)
	{
		if (line.substring(pos, pos + 8) == "Token\":\"")
		{
			line = line.substring(pos + 8);
			ix1 = line.indexOf("\"}");

			Serial.println(line.substring(0, ix1));

			tokens[i] = line.substring(0, ix1);
			i++;
			pos = 0;
		}
		else
			pos++;
	}
	return true;
}

void EIoTCloudRestApi::SetToken(String token)
{
	_token = token;
}

String EIoTCloudRestApi::GetToken()
{
	return _token;
}

String EIoTCloudRestApi::ModuleNew()
{
	String ret = "";

	WiFiClient client;

	while (!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT))
	{
		DEBUG_PRINTLN("connection failed");
		wifiConnect();
	}

	DEBUG_PRINTLN("\r\nGET /RestApi/v1.0/Module/New\r\n");

	client.print(String("POST /RestApi/v1.0/Module/New HTTP/1.1\r\n") +
				 "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" +
				 "EIOT-AuthToken: " + String(_token) + "\r\n" +
				 "Connection: close\r\n" +
				 "Content-Length: 0\r\n" +
				 "\r\n");

	return parseId(&client);
}

bool EIoTCloudRestApi::SetModulType(String id, String moduleType)
{
	WiFiClient client;

	while (!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT))
	{
		DEBUG_PRINTLN("connection failed");
		wifiConnect();
	}

	String url = "POST /RestApi/v1.0/Module/" + id + "/Type/" + moduleType;
	DEBUG_PRINTLN("\r\n");
#ifdef DEBUG
	char buff[300];
	url.toCharArray(buff, 300);
	DEBUG_PRINTLN(buff);
#endif
	DEBUG_PRINTLN("\r\n");
	;

	client.print(String(url + " HTTP/1.1\r\n") +
				 "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" +
				 "EIOT-AuthToken: " + String(_token) + "\r\n" +
				 "Connection: close\r\n" +
				 "Content-Length: 0\r\n" +
				 "\r\n");

	return parseResponse(&client);
}

bool EIoTCloudRestApi::SetModulName(String id, String name)
{
	WiFiClient client;

	while (!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT))
	{
		DEBUG_PRINTLN("connection failed");
		wifiConnect();
	}

	name.replace(" ", "%20");

	String url = "POST /RestApi/v1.0/Module/" + id + "/Name/" + name;
	DEBUG_PRINTLN("\r\n");
#ifdef DEBUG
	char buff[300];
	url.toCharArray(buff, 300);
	DEBUG_PRINTLN(buff);
#endif
	DEBUG_PRINTLN("\r\n");

	client.print(String(url + " HTTP/1.1\r\n") +
				 "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" +
				 "EIOT-AuthToken: " + String(_token) + "\r\n" +
				 "Connection: close\r\n" +
				 "Content-Length: 0\r\n" +
				 "\r\n");

	return parseResponse(&client);
}

String EIoTCloudRestApi::NewModuleParameter(String id)
{
	WiFiClient client;

	while (!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT))
	{
		DEBUG_PRINTLN("connection failed");
		wifiConnect();
	}

	String url = "POST /RestApi/v1.0/Module/" + id + "/NewParameter";
	DEBUG_PRINTLN("\r\n");
#ifdef DEBUG
	char buff[300];
	url.toCharArray(buff, 300);
	DEBUG_PRINTLN(buff);
#endif
	DEBUG_PRINTLN("\r\n");

	client.print(String(url + " HTTP/1.1\r\n") +
				 "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" +
				 "EIOT-AuthToken: " + String(_token) + "\r\n" +
				 "Connection: close\r\n" +
				 "Content-Length: 0\r\n" +
				 "\r\n");

	return parseId(&client);
}

String EIoTCloudRestApi::NewModuleParameter(String id, String name)
{
	WiFiClient client;

	while (!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT))
	{
		DEBUG_PRINTLN("connection failed");
		wifiConnect();
	}

	String url = "POST /RestApi/v1.0/Module/" + id + "/NewParameter/" + name;
	DEBUG_PRINTLN("\r\n");
#ifdef DEBUG
	char buff[300];
	url.toCharArray(buff, 300);
	DEBUG_PRINTLN(buff);
#endif
	DEBUG_PRINTLN("\r\n");

	client.print(String(url + " HTTP/1.1\r\n") +
				 "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" +
				 "EIOT-AuthToken: " + String(_token) + "\r\n" +
				 "Connection: close\r\n" +
				 "Content-Length: 0\r\n" +
				 "\r\n");

	return parseId(&client);
}

String EIoTCloudRestApi::GetModuleParameterByName(String id, String parameterName)
{
	WiFiClient client;

	while (!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT))
	{
		DEBUG_PRINTLN("connection failed");
		wifiConnect();
	}

	String url = "GET /RestApi/v1.0/Module/" + id + "/ParameterByName/" + parameterName;
	DEBUG_PRINTLN("\r\n");
#ifdef DEBUG
	char buff[300];
	url.toCharArray(buff, 300);
	DEBUG_PRINTLN(buff);
#endif
	DEBUG_PRINTLN("\r\n");

	client.print(String(url + " HTTP/1.1\r\n") +
				 "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" +
				 "EIOT-AuthToken: " + String(_token) + "\r\n" +
				 "Connection: close\r\n" +
				 "Content-Length: 0\r\n" +
				 "\r\n");

	return parseId(&client);
}

bool EIoTCloudRestApi::SetParameterValue(String parameterId, String value)
{
	return setParameterProperty(parameterId, "Value", value);
}

String EIoTCloudRestApi::GetParameterValue(String parameterId)
{
	return getParameterProperty(parameterId, "Value");
}

bool EIoTCloudRestApi::SetParameterName(String parameterId, String name)
{
	name.replace(" ", "%20");
	return setParameterProperty(parameterId, "Name", name);
}

String EIoTCloudRestApi::GetParameterName(String parameterId)
{
	String name = getParameterProperty(parameterId, "Name");

	name.replace("%20", " ");
	return name;
}

bool EIoTCloudRestApi::SetParameterDescription(String parameterId, String description)
{
	description.replace(" ", "%20");
	return setParameterProperty(parameterId, "Description", description);
}

String EIoTCloudRestApi::GetParameterDescription(String parameterId)
{
	String description = getParameterProperty(parameterId, "Description");
	description.replace("%20", " ");
	return description;
}

bool EIoTCloudRestApi::SetParameterUnit(String parameterId, String unit)
{
	return setParameterProperty(parameterId, "Unit", unit);
}

String EIoTCloudRestApi::GetParameterUnit(String parameterId)
{
	return getParameterProperty(parameterId, "Unit");
}

bool EIoTCloudRestApi::SetParameterUINotification(String parameterId, bool uiNotification)
{
	return setParameterProperty(parameterId, "UINotification", uiNotification);
}

String EIoTCloudRestApi::GetParameterUINotification(String parameterId)
{
	return getParameterProperty(parameterId, "UINotification");
}

bool EIoTCloudRestApi::SetParameterLogToDatabase(String parameterId, bool logToDatabase)
{
	return setParameterProperty(parameterId, "LogToDatabase", logToDatabase);
}

String EIoTCloudRestApi::GetParameterLogToDatabase(String parameterId)
{
	return getParameterProperty(parameterId, "LogToDatabase");
}

bool EIoTCloudRestApi::SetParameterAverageInterval(String parameterId, String avgInterval)
{
	return setParameterProperty(parameterId, "AverageInterval", avgInterval);
}

String EIoTCloudRestApi::GetParameterAverageInterval(String parameterId)
{
	return getParameterProperty(parameterId, "AverageInterval");
}

bool EIoTCloudRestApi::SetParameterChartSteps(String parameterId, bool chartSteps)
{
	return setParameterProperty(parameterId, "ChartSteps", chartSteps);
}

String EIoTCloudRestApi::GetParameterChartSteps(String parameterId)
{
	return getParameterProperty(parameterId, "ChartSteps");
}

String EIoTCloudRestApi::getParameterProperty(String parameterId, String property)
{

	WiFiClient client;

	while (!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT))
	{
		DEBUG_PRINTLN("connection failed");
		wifiConnect();
	}

	String url = "GET /RestApi/v1.0/Parameter/" + parameterId + "/" + property;
	DEBUG_PRINTLN("\r\n");
#ifdef DEBUG
	char buff[300];
	url.toCharArray(buff, 300);
	DEBUG_PRINTLN(buff);
#endif
	DEBUG_PRINTLN("\r\n");

	client.print(String(url + " HTTP/1.1\r\n") +
				 "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" +
				 "EIOT-AuthToken: " + String(_token) + "\r\n" +
				 "Connection: close\r\n" +
				 "Content-Length: 0\r\n" +
				 "\r\n");

	return parseParameter(&client, property);

	return "tes";
}

String EIoTCloudRestApi::parseParameter(WiFiClient *client, String property)
{
	String ret = "";

	while (!client->available())
		;
	//delay(300);
	String line = "";
	while (client->available())
	{
		line += client->readStringUntil('\r');
	}

#ifdef DEBUG
	char buff[300];
	line.toCharArray(buff, 300);
	DEBUG_PRINTLN(buff);
#endif

	int pos = 0;
	int len = line.length();
	int lenProp = property.length();

	//#ifdef DEBUG
	//  DEBUG_PRINTLN("\r\nlen:\r\n");
	//  String(len).toCharArray(buff, 300);
	//  DEBUG_PRINTLN(buff);
	//
	//  DEBUG_PRINTLN("\r\nlenProp:\r\n");
	//  String(lenProp).toCharArray(buff, 300);
	//  DEBUG_PRINTLN(buff);
	//#endif

	pos = 0;

	String propStr = "\"" + property + "\":\"";
	//#ifdef DEBUG
	//  DEBUG_PRINTLN("\r\npropStr:\r\n");
	//  propStr.toCharArray(buff, 300);
	//  DEBUG_PRINTLN(buff);
	//#endif

	int pos1 = 0;

	while (pos < len)
	{
		if (line.substring(pos, pos + 4 + lenProp) == propStr)
		{
			pos = pos + 4 + lenProp;
			pos1 = pos;

			while (pos < len)
			{
				if ((pos = line.indexOf('\"', pos)) != -1)
				{
					//#ifdef DEBUG
					//  DEBUG_PRINTLN("\r\nPos 2:\r\n");
					//  String(pos).toCharArray(buff, 300);
					//  DEBUG_PRINTLN(buff);
					//#endif

					ret = line.substring(pos1, pos);
#ifdef DEBUG
					DEBUG_PRINTLN("\r\n");
					ret.toCharArray(buff, 300);
					DEBUG_PRINTLN(buff);
					DEBUG_PRINTLN("\r\n");
#endif
					break;
				}
				else
					break;
			}
			break;
		}
		else
			pos++;
	}

	return ret;
}

bool EIoTCloudRestApi::SetParameterValues(String values)
{
	WiFiClient client;

	while (!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT))
	{
		DEBUG_PRINTLN("connection failed");
		wifiConnect();
	}

	String url = "POST /RestApi/v1.0/Parameter/Values";
	DEBUG_PRINTLN("\r\n");
#ifdef DEBUG
	char buff[300];
	url.toCharArray(buff, 300);
	DEBUG_PRINTLN(buff);
#endif
	DEBUG_PRINTLN("\r\n");

	client.print(String(url + " HTTP/1.1\r\n") +
				 "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" +
				 "EIOT-AuthToken: " + String(_token) + "\r\n" +
				 "Connection: close\r\n" +
				 "Content-Length: " + values.length() +
				 "\n\n" +
				 values +
				 "\r\n");

	return parseResponse(&client);
}

bool EIoTCloudRestApi::setParameterProperty(String parameterId, String property, String value)
{
	WiFiClient client;

	while (!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT))
	{
		DEBUG_PRINTLN("connection failed");
		wifiConnect();
	}

	String url = "POST /RestApi/v1.0/Parameter/" + parameterId + "/" + property + "/" + value;
	DEBUG_PRINTLN("\r\n");
#ifdef DEBUG
	char buff[300];
	url.toCharArray(buff, 300);
	DEBUG_PRINTLN(buff);
#endif
	DEBUG_PRINTLN("\r\n");

	client.print(String(url + " HTTP/1.1\r\n") +
				 "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" +
				 "EIOT-AuthToken: " + String(_token) + "\r\n" +
				 "Connection: close\r\n" +
				 "Content-Length: 0\r\n" +
				 "\r\n");

	return parseResponse(&client);
}

bool EIoTCloudRestApi::setParameterProperty(String parameterId, String property, bool value)
{
	WiFiClient client;

	while (!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT))
	{
		DEBUG_PRINTLN("connection failed");
		wifiConnect();
	}

	String url = "POST /RestApi/v1.0/Parameter/" + parameterId + "/" + property + "/";

	if (value)
		url += "true";
	else
		url += "false";

	DEBUG_PRINTLN("\r\n");
#ifdef DEBUG
	char buff[300];
	url.toCharArray(buff, 300);
	DEBUG_PRINTLN(buff);
#endif
	DEBUG_PRINTLN("\r\n");

	client.print(String(url + " HTTP/1.1\r\n") +
				 "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" +
				 "EIOT-AuthToken: " + String(_token) + "\r\n" +
				 "Connection: close\r\n" +
				 "Content-Length: 0\r\n" +
				 "\r\n");

	return parseResponse(&client);
}

String EIoTCloudRestApi::parseId(WiFiClient *client)
{
	String ret = "";

	while (!client->available())
		;
	//delay(300);
	String line = "";
	while (client->available())
	{
		line += client->readStringUntil('\r');
	}

#ifdef DEBUG
	char buff[300];
	line.toCharArray(buff, 300);
	DEBUG_PRINTLN(buff);
#endif

	// Disconnects client from server
	client->flush();
	client->stop();

	// Check for response and return result
	int pos = 0;

	while ((pos = line.indexOf('{', pos)) != -1)
	{
		if (line.substring(pos, pos + 7) == "{\"Id\":\"")
		{
			int ix1 = line.indexOf("\"}", pos + 8);

			if (ix1 != -1)
			{
				ret = line.substring(pos + 7, ix1);
#ifdef DEBUG
				DEBUG_PRINTLN("\r\n");
				ret.toCharArray(buff, 300);
				DEBUG_PRINTLN(buff);
				DEBUG_PRINTLN("\r\n");
#endif
			}
			break;
		}
		else
			pos++;
	}

	return ret;
}

bool EIoTCloudRestApi::parseResponse(WiFiClient *client)
{
	while (!client->available())
		;
	//delay(300);
	String line = "";
	while (client->available())
	{
		line += client->readStringUntil('\r');
	}

#ifdef DEBUG
	char buff[300];
	line.toCharArray(buff, 300);
	DEBUG_PRINTLN(buff);
#endif

	// Disconnects client from server
	client->flush();
	client->stop();

	// Check for response and return result
	int pos = 0;

	if ((pos = line.indexOf('{', pos)) != -1)
	{
		if (line.substring(pos, pos + 16) == "{\"Response\":\"0\"}")
			return true;
	}

	return false;
}

void EIoTCloudRestApi::wifiConnect()
{

	// check for the WiFi module:
	if (WiFi.status() == WL_NO_MODULE)
	{
		DEBUG_PRINTLN("Communication with WiFi module failed!");
		// don't continue
		while (true)
			;
	}

	// Check if right filmware is installed
	String fv = WiFi.firmwareVersion();
	if (fv < "1.0.0")
	{
		Serial.println("Please upgrade the firmware");
	}

	// attempt to connect to Wifi network:
	_status = WiFi.status(); // Get wifi status
	while (_status != WL_CONNECTED)
	{
		DEBUG_PRINTLN("Attempting to connect to WPA SSID: ");
		DEBUG_PRINTLN(_ssid);

		// Connect to WPA/WPA2 network:
		_status = WiFi.begin(_ssid, _password);

		// wait 10 seconds for connection:
		delay(500);
		DEBUG_PRINTLN(".");
	}

	// you're connected now, so print out the data:
	DEBUG_PRINTLN("You're connected to the network");
	;
}