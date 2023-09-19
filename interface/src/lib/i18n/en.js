export default {
	"welcome": "Welcome to <br /> sveltekit",
	"lib": {
		"components": {
			"UpdateIndicator": {
				"available": "Firmware update available.",
				"confirmGithubUpdate": {
					"title": "Confirm flashing new firmware to the device",
					"message": "Are you sure you want to overwrite the existing firmware with a new one?"
				}
			}
		}
	},
	"routes": {
		"page": {
			"s1": "A simple, secure and extensible framework for IoT projects for ESP32 platforms with responsive",
			"s2": "Start Demo",
		},
		"connections": {
			"ipmust": "Must be a valid IPv4 address or URL",
			"mqtt": {
				"MQTT": {
					"disabled": "MQTT Disabled",
					"id": "Client ID",
					"change": "Change MQTT Settings",
					"enable": "Enable MQTT",
					"host": "Host",
					"portmust": "Port number must be between 0 and 65536",
					"secondsmust": "Must be between 1 and 600 seconds",
					"charmust": "Must be between 64 and 4096 characters",
					"clean": "Clean Session?",
					"keep": "Keep Alive",
					"length": "Max Topic Length",
					"disconnectReason": {
						"00": "TCP Disconnected",
						"01": "Unacceptable Protocol Version",
						"02": "Identifier Rejected",
						"03": "Server unavailable",
						"04": "Malformed Credentials",
						"05": "Not Authorized",
						"06": "Not Enough Memory",
						"07": "TLS Rejected"
					}
				},
				"MQTTConfig": {
					"title": "MQTT Broker Settings",
					"alert": "The LED is controllable via MQTT with the demo project designed to work with Home Assistant's auto discovery feature.",
					"uniqueid": "Unique ID",
					"idmust": "Unique ID must be between 3 and 32 characters long",
					"name": "Name",
					"namemust": "Name must be between 3 and 32 characters long",
					"path": "MQTT Path",
					"pathlimited": "MQTT path is limited to 64 characters"
				}
			},
			"ntp": {
				"time": "Network Time",
				"server": "NTP Server",
				"localtime": "Local Time",
				"utctime": "UTC Time",
				"uptime": "Uptime",
				"changesettings": "Change NTP Settings",
				"enable": "Enable NTP",
				"pick": "Pick Time Zone",
			}
		},
		"wifi": {
			"ip": "IP Address",
			"mac": "MAC Address",
			"gateway": "Gateway IP",
			"mask": "Subnet Mask",
			"ssidmust": "SSID must be between 2 and 32 characters long",
			"localip": "Local IP",
			"ipmust": "Must be a valid IPv4 address",
			"scannet": "Scan Networks",
			"channel": "Channel",
			"ap": {
				"title": "Access Point",
				"apclients": "AP Clients",
				"change": "Change AP Settings",
				"provideap": "Provide Access Point ...",
				"prefchannel": "Preferred Channel",
				"channelmust": "Must be channel 1 to 13",
				"maxcli": "Max Clients",
				"max8": "Maximum 8 clients allowed",
				"hidessid": "Hide SSID",
			},
			"wifi": {
				"change": "Change Wi-Fi Settings",
				"hostname": "Host Name",
				"hostnamemust": "Host name must be between 2 and 32 characters long",
				"staticip": "Static IP Config?",
			},
			"scan": {
				"open": "Open",
				"scanning": "Scanning ...",
				"security": "Security",
				"again": "Scan again",
			}
		},
		"user": {
			"title": "Manage Users",
			"admin": "Admin",
			"edit": "Edit",
			"securitysettings": "Security Settings",
			"jwt": "The JWT secret is used to sign authentication tokens. If you modify the JWT Secret, all users will be signed out.",
			"jwtsecret": "JWT Secret",
			"confirmdel": "Confirm Delete User",
			"messagedel": 'Are you sure you want to delete the user "',
			"edituser": "Edit User",
			"adduser": "Add User"
		},
		"edituser": {
			"usernamemust": "Username must be between 3 and 32 characters long",
			"isadmin": "Is Admin?",
		},
		"system": {
			"status": {
				"title": "System Status",
				"device": "Device (Platform / SDK)",
				"firmware": "Firmware Version",
				"cpufr": "CPU Frequency",
				"heap": "Heap (Free / Max Alloc)",
				"psram": "PSRAM (Size / Free)",
				"sketch": "Sketch (Used / Free)",
				"chip": "Flash Chip (Size / Speed)",
				"files": "File System (Used / Total)",
				"temperature": "Core Temperature",
				"confirmrestart": "Confirm Restart",
				"messagerestart": "Are you sure you want to restart the device?",
				"confirmreset": "Confirm Factory Reset",
				"messagereset": "Are you sure you want to reset the device to its factory defaults?",
				"confirmsleep": "Confirm Going to Sleep",
				"messagesleep": "Are you sure you want to put the device into sleep?",
			},
			"update": {
				"confirmflash": "Confirm flashing new firmware to the device",
				"messageflash": "Are you sure you want to overwrite the existing firmware with a new one?",
				"githubtitle": "Github Firmware Manager",
				"release": "Release",
				"releasedate": "Release Date",
				"experimental": "Experimental",
				"install": "Install",
				"pleaseconnect": "Please connect to a network with internet access to perform a firmware update.",
				"otatitle": "OTA Firmware Update",
				"otaenable": "Enable Remote OTA Updates?",
				"portmust1025": "Port number must be between 1025 and 65536",
				"confirmupload": "Confirm Flashing the Device",
				"messageupload": "Are you sure you want to overwrite the existing firmware with a new one?",
				"uploadfirmware": "Upload Firmware",
				"uploadwill": "Uploading a new firmware (.bin) file will replace the existing firmware.",
			}
		}
	},
	"menu": {
		"demo": "Demo App",
		"connections": "Connections",
		"connections.mqtt": "MQTT",
		"connections.ntp": "NTP",
		"wifi": "Wi-Fi",
		"users": "Users",
		"system": "System",
		"system.status": "System Status",
		"firmware.update": "Firmware Update"
	},
	"stores": {
		"user": {
			"logout": "Log out user"
		}
	},
	"ntfc": {
		"success": {
			"securityupdated": "Security settings updated.",
			"brokerupdated": "Broker settings updated.",
			"apupdated": "Access Point settings updated.",
			"wifiupdated": "Wi-Fi settings updated.",
			"otaupdated": "OTA settings updated.",
		},
		"error": {
			"usernotauth": "User not authorized."
		}
	},
	"ok": "OK",
	"cancel": "Cancel",
	"save": "Save",
	"and": "and",
	"built": "front-end built with",
	"updating.firmware": "Updating Firmware",
	"close": "Close",
	"loading": "Loading...",
	"error": "Error:",
	"abort": "Abort",
	"restart": "Restart",
	"reset": "Factory Reset",
	"sleep": "Sleep",
	"yes": "Yes",
	"update": "Update",
	"connected": "Connected",
	"seconds": "Seconds",
	"chars": "Chars",
	"applysettings": "Apply Settings",
	"status": "Status",
	"active": "Active",
	"inactive": "Inactive",
	"server": "Server",
	"day": " day",
	"hour": " hour",
	"minute": " minute",
	"second": " second",
	"password": "Password",
	"username": "Username",
	"used": "used",
	"of": "of",
	"free": "free",
	"port": "Port",
	"upload": "Upload"
};
