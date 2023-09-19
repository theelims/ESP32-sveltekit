export default {
	"welcome": "Bienvenido a <br /> sveltekit",
	"lib": {
		"components": {
			"UpdateIndicator": {
				"available": "Actualización de firmware disponible.",
				"confirmGithubUpdate": {
					"title": "Confirmar la instalación del nuevo firmware en el dispositivo",
					"message": "¿Estás seguro de que quieres sobrescribir el firmware existente con uno nuevo?"
				}
			}
		}
	},
	"routes": {
		"page": {
			"s1": "Un marco simple, seguro y extensible para proyectos IoT para plataformas ESP32 con respuesta",
			"s2": "Iniciar demostración",
		},
		"connections": {
			"ipmust": "Debe ser una dirección IPv4 válida o URL",
			"mqtt": {
				"MQTT": {
					"disabled": "MQTT Desactivado",
					"id": "ID del cliente",
					"change": "Cambiar configuración de MQTT",
					"enable": "Activar MQTT",
					"host": "Anfitrión",
					"portmust": "El número de puerto debe estar entre 0 y 65536",
					"secondsmust": "Debe estar entre 1 y 600 segundos",
					"charmust": "Debe tener entre 64 y 4096 caracteres",
					"clean": "¿Sesión limpia?",
					"keep": "Mantener vivo",
					"length": "Longitud máxima del tema",
					"disconnectReason": {
						"00": "TCP Desconectado",
						"01": "Versión de protocolo inaceptable",
						"02": "Identificador rechazado",
						"03": "Servidor no disponible",
						"04": "Credenciales mal formadas",
						"05": "No autorizado",
						"06": "Memoria insuficiente",
						"07": "TLS Rechazado"
					}
				},
				"MQTTConfig": {
					"title": "Configuración del broker MQTT",
					"alert": "El LED se puede controlar a través de MQTT con el proyecto de demostración diseñado para trabajar con la función de descubrimiento automático de Home Assistant.",
					"uniqueid": "ID único",
					"idmust": "El ID único debe tener entre 3 y 32 caracteres de longitud",
					"name": "Nombre",
					"namemust": "El nombre debe tener entre 3 y 32 caracteres de longitud",
					"path": "Ruta MQTT",
					"pathlimited": "La ruta MQTT está limitada a 64 caracteres"
				}
			},
			"ntp": {
				"time": "Hora de la red",
				"server": "Servidor NTP",
				"localtime": "Hora local",
				"utctime": "Hora UTC",
				"uptime": "Tiempo de actividad",
				"changesettings": "Cambiar configuración de NTP",
				"enable": "Activar NTP",
				"pick": "Elegir zona horaria",
			}
		},
		"wifi": {
			"ip": "Dirección IP",
			"mac": "Dirección MAC",
			"gateway": "IP de la puerta de enlace",
			"mask": "Máscara de subred",
			"ssidmust": "El SSID debe tener entre 2 y 32 caracteres de longitud",
			"localip": "IP local",
			"ipmust": "Debe ser una dirección IPv4 válida",
			"scannet": "Escanear redes",
			"channel": "Canal",
			"ap": {
				"title": "Punto de acceso",
				"apclients": "Clientes AP",
				"change": "Cambiar configuración de AP",
				"provideap": "Proporcionar punto de acceso ...",
				"prefchannel": "Canal preferido",
				"channelmust": "Debe ser el canal 1 al 13",
				"maxcli": "Máx. Clientes",
				"max8": "Máximo 8 clientes permitidos",
				"hidessid": "Ocultar SSID",
			},
			"wifi": {
				"change": "Cambiar configuración de Wi-Fi",
				"hostname": "Nombre del host",
				"hostnamemust": "El nombre del host debe tener entre 2 y 32 caracteres de longitud",
				"staticip": "¿Configuración IP estática?",
			},
			"scan": {
				"open": "Abrir",
				"scanning": "Escaneando ...",
				"security": "Seguridad",
				"again": "Escanear de nuevo",
			}
		},
		"user": {
			"title": "Gestionar usuarios",
			"admin": "Administrador",
			"edit": "Editar",
			"securitysettings": "Configuración de seguridad",
			"jwt": "El secreto JWT se utiliza para firmar tokens de autenticación. Si modificas el Secreto JWT, todos los usuarios serán desconectados.",
			"jwtsecret": "Secreto JWT",
			"confirmdel": "Confirmar eliminar usuario",
			"messagedel": '¿Estás seguro de que quieres eliminar al usuario "',
			"edituser": "Editar usuario",
			"adduser": "Añadir usuario"
		},
		"edituser": {
			"usernamemust": "El nombre de usuario debe tener entre 3 y 32 caracteres de longitud",
			"isadmin": "¿Es administrador?",
		},
		"system": {
			"status": {
				"title": "Estado del Sistema",
				"device": "Dispositivo (Plataforma / SDK)",
				"firmware": "Versión del Firmware",
				"cpufr": "Frecuencia de la CPU",
				"heap": "Heap (Libre / Máx. Asignable)",
				"psram": "PSRAM (Tamaño / Libre)",
				"sketch": "Sketch (Usado / Libre)",
				"chip": "Chip de Memoria Flash (Tamaño / Velocidad)",
				"files": "Sistema de Archivos (Usado / Total)",
				"temperature": "Temperatura del Núcleo",
				"confirmrestart": "Confirmar Reinicio",
				"messagerestart": "¿Estás seguro de que quieres reiniciar el dispositivo?",
				"confirmreset": "Confirmar Restablecimiento de Fábrica",
				"messagereset": "¿Estás seguro de que quieres restablecer el dispositivo a sus valores predeterminados de fábrica?",
				"confirmsleep": "Confirmar Modo de Suspensión",
				"messagesleep": "¿Estás seguro de que quieres poner el dispositivo en modo de suspensión?"
			},
			"update": {
				"confirmflash": "Confirmar la instalación del nuevo firmware en el dispositivo",
				"messageflash": "¿Estás seguro de que quieres sobrescribir el firmware existente con uno nuevo?",
				"githubtitle": "Gestor de Firmware de Github",
				"release": "Versión",
				"releasedate": "Fecha de Lanzamiento",
				"experimental": "Experimental",
				"install": "Instalar",
				"pleaseconnect": "Por favor, conéctate a una red con acceso a internet para realizar una actualización de firmware.",
				"otatitle": "Actualización de Firmware vía OTA",
				"otaenable": "¿Habilitar Actualizaciones Remotas vía OTA?",
				"portmust1025": "El número de puerto debe estar entre 1025 y 65536",
				"confirmupload": "Confirmar la Instalación en el Dispositivo",
				"messageupload": "¿Estás seguro de que quieres sobrescribir el firmware existente con uno nuevo?",
				"uploadfirmware": "Subir Firmware",
				"uploadwill": "Subir un nuevo archivo de firmware (.bin) reemplazará el firmware existente."
			}
		}
	},
	"menu": {
		"demo": "App de Demostración",
		"connections": "Conexiones",
		"connections.mqtt": "MQTT",
		"connections.ntp": "NTP",
		"wifi": "Wi-Fi",
		"users": "Usuarios",
		"system": "Sistema",
		"system.status": "Estado del Sistema",
		"firmware.update": "Actualización de Firmware"
	},
	"stores": {
		"user": {
			"logout": "Cerrar sesión"
		}
	},
	"ntfc": {
		"success": {
			"securityupdated": "Configuración de seguridad actualizada.",
			"brokerupdated": "Configuración del broker actualizada.",
			"apupdated": "Configuración del punto de acceso actualizada.",
			"wifiupdated": "Configuración de Wi-Fi actualizada.",
			"otaupdated": "Configuraciones OTA actualizadas.",
		},
		"error": {
			"usernotauth": "Usuario no autorizado."
		}
	},
	"ok": "OK",
	"cancel": "Cancelar",
	"save": "Guardar",
	"and": "y",
	"built": "interfaz construida con",
	"updating.firmware": "Actualizando Firmware",
	"close": "Cerrar",
	"loading": "Cargando...",
	"error": "Error:",
	"abort": "Abortar",
	"restart": "Reiniciar",
	"reset": "Restablecer de Fábrica",
	"sleep": "Dormir",
	"yes": "Sí",
	"update": "Actualizar",
	"connected": "Conectado",
	"seconds": "Segundos",
	"chars": "Caracteres",
	"applysettings": "Aplicar Configuraciones",
	"status": "Estado",
	"active": "Activo",
	"inactive": "Inactivo",
	"server": "Servidor",
	"day": " día",
	"hour": " hora",
	"minute": " minuto",
	"second": " segundo",
	"password": "Contraseña",
	"username": "Nombre de usuario",
	"used": "usado",
	"of": "de",
	"free": "libre",
	"port": "Puerto",
	"upload": "Subir"
};
