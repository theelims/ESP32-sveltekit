export default {
	"welcome": "Benvenuto a <br /> sveltekit",
	"lib": {
		"components": {
			"UpdateIndicator": {
				"available": "Aggiornamento firmware disponibile.",
				"confirmGithubUpdate": {
					"title": "Conferma l'installazione del nuovo firmware sul dispositivo",
					"message": "Sei sicuro di voler sovrascrivere il firmware esistente con uno nuovo?"
				}
			}
		}
	},
	"routes": {
		"page": {
			"s1": "Un framework semplice, sicuro ed estensibile per progetti IoT per piattaforme ESP32 con risposta",
			"s2": "Inizia Demo",
		},
		"connections": {
			"ipmust": "Deve essere un indirizzo IPv4 valido o un URL",
			"mqtt": {
				"MQTT": {
					"disabled": "MQTT disabilitato",
					"id": "ID cliente",
					"change": "Cambia impostazioni MQTT",
					"enable": "Abilita MQTT",
					"host": "Host",
					"portmust": "Il numero della porta deve essere tra 0 e 65536",
					"secondsmust": "Deve essere tra 1 e 600 secondi",
					"charmust": "Deve essere tra 64 e 4096 caratteri",
					"clean": "Sessione pulita?",
					"keep": "Mantieni vivo",
					"length": "Lunghezza massima dell'argomento",
					"disconnectReason": {
						"00": "TCP disconnesso",
						"01": "Versione del protocollo inaccettabile",
						"02": "Identificatore rifiutato",
						"03": "Server non disponibile",
						"04": "Credenziali malformate",
						"05": "Non autorizzato",
						"06": "Memoria insufficiente",
						"07": "TLS rifiutato"
					}
				},
				"MQTTConfig": {
					"title": "Impostazioni del broker MQTT",
					"alert": "Il LED è controllabile tramite MQTT con il progetto demo progettato per funzionare con la funzione di scoperta automatica di Home Assistant.",
					"uniqueid": "ID unico",
					"idmust": "L'ID unico deve avere tra 3 e 32 caratteri",
					"name": "Nome",
					"namemust": "Il nome deve avere tra 3 e 32 caratteri",
					"path": "Percorso MQTT",
					"pathlimited": "Il percorso MQTT è limitato a 64 caratteri"
				}
			},
			"ntp": {
				"time": "Ora di rete",
				"server": "Server NTP",
				"localtime": "Ora locale",
				"utctime": "Ora UTC",
				"uptime": "Tempo di attività",
				"changesettings": "Cambia impostazioni NTP",
				"enable": "Abilita NTP",
				"pick": "Scegli fuso orario",
			}
		},
		"wifi": {
			"ip": "Indirizzo IP",
			"mac": "Indirizzo MAC",
			"gateway": "IP gateway",
			"mask": "Maschera di sottorete",
			"ssidmust": "L'SSID deve avere tra 2 e 32 caratteri",
			"localip": "IP locale",
			"ipmust": "Deve essere un indirizzo IPv4 valido",
			"scannet": "Scansiona reti",
			"channel": "Canale",
			"ap": {
				"title": "Punto di accesso",
				"apclients": "Clienti AP",
				"change": "Cambia impostazioni AP",
				"provideap": "Fornisci punto di accesso ...",
				"prefchannel": "Canale preferito",
				"channelmust": "Deve essere il canale 1 al 13",
				"maxcli": "Clienti massimi",
				"max8": "Massimo 8 clienti consentiti",
				"hidessid": "Nascondi SSID",
			},
			"wifi": {
				"change": "Cambia impostazioni Wi-Fi",
				"hostname": "Nome host",
				"hostnamemust": "Il nome host deve avere tra 2 e 32 caratteri",
				"staticip": "Configurazione IP statico?",
			},
			"scan": {
				"open": "Apri",
				"scanning": "Scansione ...",
				"security": "Sicurezza",
				"again": "Scansiona di nuovo",
			}
		},
		"user": {
			"title": "Gestisci utenti",
			"admin": "Amministratore",
			"edit": "Modifica",
			"securitysettings": "Impostazioni di sicurezza",
			"jwt": "Il segreto JWT viene utilizzato per firmare i token di autenticazione. Se modifichi il segreto JWT, tutti gli utenti verranno disconnessi.",
			"jwtsecret": "Segreto JWT",
			"confirmdel":"Conferma eliminazione utente",
			"messagedel":'Sei sicuro di voler eliminare l\'utente "',
			"edituser":"Modifica utente",
			"adduser":"Aggiungi utente"
		},
		"edituser":{
			"usernamemust": "Il nome utente deve avere tra 3 e 32 caratteri",
			"isadmin":"È amministratore?",
		},
		"system":{
			"status":{
				"title":"Stato del sistema",
				"device":"Dispositivo (Piattaforma / SDK)",
				"firmware":"Versione firmware",
				"cpufr":"Frequenza CPU",
				"heap":"Heap (Libero / Max Alloc)",
				"psram":"PSRAM (Dimensione / Libero)",
				"sketch":"Sketch (Usato / Libero)",
				"chip":"Chip Flash (Dimensione / Velocità)",
				"files":"Sistema di file (Usato / Totale)",
				"temperature":"Temperatura del core",
				"confirmrestart":"Conferma riavvio",
				"messagerestart":"Sei sicuro di voler riavviare il dispositivo?",
				"confirmreset":"Conferma ripristino di fabbrica",
				"messagereset":"Sei sicuro di voler ripristinare il dispositivo alle impostazioni di fabbrica?",
				"confirmsleep":"Conferma modalità di attesa",
				"messagesleep":"Sei sicuro di voler mettere il dispositivo in modalità di attesa?",
			},
			"update":{
				"confirmflash":"Conferma l'installazione del nuovo firmware sul dispositivo",
				"messageflash":"Sei sicuro di voler sovrascrivere il firmware esistente con uno nuovo?",
				"githubtitle":"Gestore firmware Github",
				"release":"Rilascio",
				"releasedate":"Data di rilascio",
				"experimental":"Sperimentale",
				"install":"Installa",
				"pleaseconnect":"Connettiti a una rete con accesso a Internet per eseguire un aggiornamento del firmware.",
				"otatitle":"Aggiornamento firmware OTA",
				"otaenable":"Abilitare gli aggiornamenti OTA remoti?",
				"portmust1025":"Il numero della porta deve essere tra 1025 e 65536",
				"confirmupload":"Conferma l'installazione sul dispositivo",
				"messageupload":"Sei sicuro di voler sovrascrivere il firmware esistente con uno nuovo?",
				"uploadfirmware":"Carica firmware",
				"uploadwill":"Caricando un nuovo file firmware (.bin) sostituirai il firmware esistente.",
			}
		}
	},
	"menu":{
		"demo":"App di demo",
		"connections":"Connessioni",
		"connections.mqtt":"MQTT",
		"connections.ntp":"NTP",
		"wifi":"Wi-Fi",
		"users":"Utenti",
		"system":"Sistema",
		"system.status":"Stato del sistema",
		"firmware.update":"Aggiornamento firmware"
	},
	"stores": {
		"user": {
			"logout": "Disconnetti utente"
		}
	},
	"ntfc": {
		"success": {
			"securityupdated": "Impostazioni di sicurezza aggiornate.",
			"brokerupdated": "Impostazioni del broker aggiornate.",
			"apupdated": "Impostazioni del punto di accesso aggiornate.",
			"wifiupdated": "Impostazioni Wi-Fi aggiornate.",
			"otaupdated":"Impostazioni OTA aggiornate.",
		},
		"error": {
			"usernotauth": "Utente non autorizzato."
		}
	},
	"ok": "OK",
	"cancel": "Annulla",
	"save":"Salva",
	"and": "e",
	"built": "front-end costruito con",
	"updating.firmware": "Aggiornamento firmware",
	"close": "Chiudi",
	"loading": "Caricamento...",
	"error": "Errore:",
	"abort": "Interrompi",
	"restart":"Riavvia",
	"reset":"Ripristino di fabbrica",
	"sleep":"Modalità di attesa",
	"yes": "Sì",
	"update": "Aggiorna",
	"connected": "Connesso",
	"seconds": "Secondi",
	"chars": "Caratteri",
	"applysettings": "Applica impostazioni",
	"status": "Stato",
	"active": "Attivo",
	"inactive": "Inattivo",
	"server": "Server",
	"day": " giorno",
	"hour": " ora",
	"minute": " minuto",
	"second": " secondo",
	"password": "Password",
	"username": "Nome utente",
	"used":"usato",
	"of":"di",
	"free":"libero",
	"port": "Porta",
	"upload":"Carica"
};
