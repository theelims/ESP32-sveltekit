export default {
	"welcome": "Willkommen bei <br /> sveltekit",
	"lib": {
		"components": {
			"UpdateIndicator": {
				"available": "Firmware-Update verfügbar.",
				"confirmGithubUpdate": {
					"title": "Bestätigen Sie das Flashen der neuen Firmware auf das Gerät",
					"message": "Sind Sie sicher, dass Sie die vorhandene Firmware mit einer neuen überschreiben möchten?"
				}
			}
		}
	},
	"routes": {
		"page": {
			"s1": "Ein einfacher, sicherer und erweiterbarer Rahmen für IoT-Projekte für ESP32-Plattformen mit responsivem",
			"s2": "Demo starten",
		},
		"connections": {
			"ipmust": "Muss eine gültige IPv4-Adresse oder URL sein",
			"mqtt": {
				"MQTT": {
					"disabled": "MQTT deaktiviert",
					"id": "Client-ID",
					"change": "MQTT-Einstellungen ändern",
					"enable": "MQTT aktivieren",
					"host": "Host",
					"portmust": "Portnummer muss zwischen 0 und 65536 liegen",
					"secondsmust": "Muss zwischen 1 und 600 Sekunden liegen",
					"charmust": "Muss zwischen 64 und 4096 Zeichen liegen",
					"clean": "Sitzung bereinigen?",
					"keep": "Verbindung halten",
					"length": "Maximale Topic-Länge",
					"disconnectReason": {
						"00": "TCP getrennt",
						"01": "Inakzeptable Protokollversion",
						"02": "Identifikator abgelehnt",
						"03": "Server nicht verfügbar",
						"04": "Fehlerhafte Anmeldeinformationen",
						"05": "Nicht autorisiert",
						"06": "Nicht genug Speicher",
						"07": "TLS abgelehnt"
					}
				},
				"MQTTConfig": {
					"title": "MQTT-Broker-Einstellungen",
					"alert": "Die LED kann über MQTT gesteuert werden. Das Demoprojekt wurde entwickelt, um mit der Auto-Discovery-Funktion von Home Assistant zu arbeiten.",
					"uniqueid": "Eindeutige ID",
					"idmust": "Eindeutige ID muss zwischen 3 und 32 Zeichen lang sein",
					"name": "Name",
					"namemust": "Name muss zwischen 3 und 32 Zeichen lang sein",
					"path": "MQTT-Pfad",
					"pathlimited": "MQTT-Pfad ist auf 64 Zeichen begrenzt"
				}
			},
			"ntp": {
				"time": "Netzwerkzeit",
				"server": "NTP-Server",
				"localtime": "Lokale Zeit",
				"utctime": "UTC-Zeit",
				"uptime": "Betriebszeit",
				"changesettings": "NTP-Einstellungen ändern",
				"enable": "NTP aktivieren",
				"pick": "Zeitzone auswählen",
			}
		},
		"wifi": {
			"ip": "IP-Adresse",
			"mac": "MAC-Adresse",
			"gateway": "Gateway-IP",
			"mask": "Subnetzmaske",
			"ssidmust": "SSID muss zwischen 2 und 32 Zeichen lang sein",
			"localip": "Lokale IP",
			"ipmust": "Muss eine gültige IPv4-Adresse sein",
			"scannet": "Netzwerke scannen",
			"channel": "Kanal",
			"ap": {
				"title": "Zugangspunkt",
				"apclients": "AP-Clients",
				"change": "AP-Einstellungen ändern",
				"provideap": "Zugangspunkt bereitstellen ...",
				"prefchannel": "Bevorzugter Kanal",
				"channelmust": "Muss Kanal 1 bis 13 sein",
				"maxcli": "Maximale Clients",
				"max8": "Maximal 8 Clients erlaubt",
				"hidessid": "SSID verstecken",
			},
			"wifi": {
				"change": "Wi-Fi-Einstellungen ändern",
				"hostname": "Hostname",
				"hostnamemust": "Hostname muss zwischen 2 und 32 Zeichen lang sein",
				"staticip": "Statische IP-Konfiguration?",
			},
			"scan": {
				"open": "Öffnen",
				"scanning": "Scannen ...",
				"security": "Sicherheit",
				"again": "Erneut scannen",
			}
		},
		"user": {
			"title": "Benutzer verwalten",
			"admin": "Administrator",
			"edit": "Bearbeiten",
			"securitysettings": "Sicherheitseinstellungen",
			"jwt": "Das JWT-Geheimnis wird verwendet, um Authentifizierungstoken zu signieren. Wenn Sie das JWT-Geheimnis ändern, werden alle Benutzer abgemeldet.",
			"jwtsecret": "JWT-Geheimnis",
			"confirmdel":"Benutzerlöschung bestätigen",
			"messagedel":'Sind Sie sicher, dass Sie den Benutzer "',
			"edituser":"Benutzer bearbeiten",
			"adduser":"Benutzer hinzufügen"
		},
		"edituser":{
			"usernamemust": "Benutzername muss zwischen 3 und 32 Zeichen lang sein",
			"isadmin":"Ist Administrator?",
		},
		"system":{
			"status":{
				"title":"Systemstatus",
				"device":"Gerät (Plattform / SDK)",
				"firmware":"Firmware-Version",
				"cpufr":"CPU-Frequenz",
				"heap":"Heap (Frei / Max Alloc)",
				"psram":"PSRAM (Größe / Frei)",
				"sketch":"Sketch (Verwendet / Frei)",
				"chip":"Flash-Chip (Größe / Geschwindigkeit)",
				"files":"Dateisystem (Verwendet / Gesamt)",
				"temperature":"Kerntemperatur",
				"confirmrestart":"Neustart bestätigen",
				"messagerestart":"Sind Sie sicher, dass Sie das Gerät neu starten möchten?",
				"confirmreset":"Werkseinstellungen bestätigen",
				"messagereset":"Sind Sie sicher, dass Sie das Gerät auf die Werkseinstellungen zurücksetzen möchten?",
				"confirmsleep":"Schlafmodus bestätigen",
				"messagesleep":"Sind Sie sicher, dass Sie das Gerät in den Schlafmodus versetzen möchten?",
			},
			"update":{
				"confirmflash":"Bestätigen Sie das Flashen der neuen Firmware auf das Gerät",
				"messageflash":"Sind Sie sicher, dass Sie die vorhandene Firmware mit einer neuen überschreiben möchten?",
				"githubtitle":"Github Firmware-Manager",
				"release":"Veröffentlichung",
				"releasedate":"Veröffentlichungsdatum",
				"experimental":"Experimentell",
				"install":"Installieren",
				"pleaseconnect":"Bitte verbinden Sie sich mit einem Netzwerk mit Internetzugang, um ein Firmware-Update durchzuführen.",
				"otatitle":"OTA-Firmware-Update",
				"otaenable":"Fern-OTA-Updates aktivieren?",
				"portmust1025":"Portnummer muss zwischen 1025 und 65536 liegen",
				"confirmupload":"Flashen des Geräts bestätigen",
				"messageupload":"Sind Sie sicher, dass Sie die vorhandene Firmware mit einer neuen überschreiben möchten?",
				"uploadfirmware":"Firmware hochladen",
				"uploadwill":"Das Hochladen einer neuen Firmware-Datei (.bin) ersetzt die vorhandene Firmware.",
			}
		}
	},
	"menu":{
		"demo":"Demo-App",
		"connections":"Verbindungen",
		"connections.mqtt":"MQTT",
		"connections.ntp":"NTP",
		"wifi":"Wi-Fi",
		"users":"Benutzer",
		"system":"System",
		"system.status":"Systemstatus",
		"firmware.update":"Firmware-Update"
	},
	"stores": {
		"user": {
			"logout": "Benutzer abmelden"
		}
	},
	"ntfc": {
		"success": {
			"securityupdated": "Sicherheitseinstellungen aktualisiert.",
			"brokerupdated": "Broker-Einstellungen aktualisiert.",
			"apupdated": "Zugangspunkt-Einstellungen aktualisiert.",
			"wifiupdated": "Wi-Fi-Einstellungen aktualisiert.",
			"otaupdated":"OTA-Einstellungen aktualisiert.",
		},
		"error": {
			"usernotauth": "Benutzer nicht autorisiert."
		}
	},
	"ok": "OK",
	"cancel": "Abbrechen",
	"save":"Speichern",
	"and": "und",
	"built": "Frontend erstellt mit",
	"updating.firmware": "Firmware wird aktualisiert",
	"close": "Schließen",
	"loading": "Laden...",
	"error": "Fehler:",
	"abort": "Abbrechen",
	"restart":"Neustart",
	"reset":"Werkseinstellungen",
	"sleep":"Schlafmodus",
	"yes": "Ja",
	"update": "Aktualisieren",
	"connected": "Verbunden",
	"seconds": "Sekunden",
	"chars": "Zeichen",
	"applysettings": "Einstellungen anwenden",
	"status": "Status",
	"active": "Aktiv",
	"inactive": "Inaktiv",
	"server": "Server",
	"day": " Tag",
	"hour": " Stunde",
	"minute": " Minute",
	"second": " Sekunde",
	"password": "Passwort",
	"username": "Benutzername",
	"used":"verwendet",
	"of":"von",
	"free":"frei",
	"port": "Port",
	"upload":"Hochladen"
};
