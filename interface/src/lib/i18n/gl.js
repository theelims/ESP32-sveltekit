export default {
    "welcome": "Benvido a <br /> sveltekit",
    "lib": {
        "components": {
            "UpdateIndicator": {
                "available": "Actualización de firmware dispoñible.",
                "confirmGithubUpdate": {
                    "title": "Confirmar a instalación do novo firmware no dispositivo",
                    "message": "Estás seguro de querer sobrescribir o firmware existente cun novo?"
                }
            }
        }
    },
    "routes": {
        "page": {
            "s1": "Un marco simple, seguro e extensible para proxectos IoT para plataformas ESP32 con resposta",
            "s2": "Comezar Demo",
        },
        "connections": {
            "ipmust": "Debe ser unha dirección IPv4 válida ou unha URL",
            "mqtt": {
                "MQTT": {
                    "disabled": "MQTT desactivado",
                    "id": "ID de cliente",
                    "change": "Cambiar configuración MQTT",
                    "enable": "Activar MQTT",
                    "host": "Anfitrión",
                    "portmust": "O número de porto debe estar entre 0 e 65536",
                    "secondsmust": "Debe estar entre 1 e 600 segundos",
                    "charmust": "Debe estar entre 64 e 4096 caracteres",
                    "clean": "Sesión limpa?",
                    "keep": "Manter vivo",
                    "length": "Lonxitude máxima do tema",
                    "disconnectReason": {
                        "00": "TCP desconectado",
                        "01": "Versión de protocolo inaceptable",
                        "02": "Identificador rexeitado",
                        "03": "Servidor non dispoñible",
                        "04": "Credenciais mal formadas",
                        "05": "Non autorizado",
                        "06": "Non hai memoria suficiente",
                        "07": "TLS rexeitado"
                    }
                },
                "MQTTConfig": {
                    "title": "Configuración do corretor MQTT",
                    "alert": "O LED é controlable a través de MQTT co proxecto de demostración deseñado para traballar coa función de descubrimento automático de Home Assistant.",
                    "uniqueid": "ID único",
                    "idmust": "O ID único debe ter entre 3 e 32 caracteres",
                    "name": "Nome",
                    "namemust": "O nome debe ter entre 3 e 32 caracteres",
                    "path": "Camiño MQTT",
                    "pathlimited": "O camiño MQTT está limitado a 64 caracteres"
                }
            },
            "ntp": {
                "time": "Hora da rede",
                "server": "Servidor NTP",
                "localtime": "Hora local",
                "utctime": "Hora UTC",
                "uptime": "Tempo de funcionamento",
                "changesettings": "Cambiar configuración NTP",
                "enable": "Activar NTP",
                "pick": "Elixir fuso horario",
            }
        },
        "wifi": {
            "ip": "Enderezo IP",
            "mac": "Enderezo MAC",
            "gateway": "IP da pasarela",
            "mask": "Máscara de subred",
            "ssidmust": "O SSID debe ter entre 2 e 32 caracteres",
            "localip": "IP local",
            "ipmust": "Debe ser un enderezo IPv4 válido",
            "scannet": "Escanear redes",
            "channel": "Canal",
            "ap": {
                "title": "Punto de acceso",
                "apclients": "Clientes AP",
                "change": "Cambiar configuración AP",
                "provideap": "Proporcionar punto de acceso ...",
                "prefchannel": "Canal preferido",
                "channelmust": "Debe ser o canal 1 a 13",
                "maxcli": "Clientes máx",
                "max8": "Máximo 8 clientes permitidos",
                "hidessid": "Ocultar SSID",
            },
            "wifi": {
                "change": "Cambiar configuración Wi-Fi",
                "hostname": "Nome do anfitrión",
                "hostnamemust": "O nome do anfitrión debe ter entre 2 e 32 caracteres",
                "staticip": "Config IP estática?",
            },
            "scan": {
                "open": "Abrir",
                "scanning": "Escanear ...",
                "security": "Seguridade",
                "again": "Escanear de novo",
            }
        },
        "user": {
            "title": "Xestionar usuarios",
            "admin": "Administrador",
            "edit": "Editar",
            "securitysettings": "Configuracións de seguridade",
            "jwt": "O segredo JWT úsase para asinar tokens de autenticación. Se modificas o segredo JWT, todos os usuarios serán desconectados.",
            "jwtsecret": "Segredo JWT",
            "confirmdel": "Confirmar eliminar usuario",
            "messagedel": 'Estás seguro de querer eliminar o usuario "',
            "edituser": "Editar usuario",
            "adduser": "Engadir usuario"
        },
        "edituser": {
            "usernamemust": "O nome de usuario debe ter entre 3 e 32 caracteres",
            "isadmin": "É administrador?",
        },
        "system": {
            "status": {
                "title": "Estado do sistema",
                "device": "Dispositivo (Plataforma / SDK)",
                "firmware": "Versión do firmware",
                "cpufr": "Frecuencia CPU",
                "heap": "Montón (Libre / Max Alloc)",
                "psram": "PSRAM (Tamaño / Libre)",
                "sketch": "Esbozo (Usado / Libre)",
                "chip": "Chip Flash (Tamaño / Velocidade)",
                "files": "Sistema de arquivos (Usado / Total)",
                "temperature": "Temperatura do núcleo",
                "confirmrestart": "Confirmar reinicio",
                "messagerestart": "Estás seguro de querer reiniciar o dispositivo?",
                "confirmreset": "Confirmar reconfiguración de fábrica",
                "messagereset": "Estás seguro de querer reconfigurar o dispositivo aos seus valores predeterminados de fábrica?",
                "confirmsleep": "Confirmar modo de espera",
                "messagesleep": "Estás seguro de querer poñer o dispositivo en modo de espera?",
            },
            "update": {
                "confirmflash": "Confirmar a instalación do novo firmware no dispositivo",
                "messageflash": "Estás seguro de querer sobrescribir o firmware existente cun novo?",
                "githubtitle": "Xestor de firmware Github",
                "release": "Lanzamento",
                "releasedate": "Data de lanzamento",
                "experimental": "Experimental",
                "install": "Instalar",
                "pleaseconnect": "Conéctate a unha rede con acceso a Internet para realizar unha actualización de firmware.",
                "otatitle": "Actualización de firmware OTA",
                "otaenable": "Activar actualizacións OTA remotas?",
                "portmust1025": "O número de porto debe estar entre 1025 e 65536",
                "confirmupload": "Confirmar a instalación no dispositivo",
                "messageupload": "Estás seguro de querer sobrescribir o firmware existente cun novo?",
                "uploadfirmware": "Subir firmware",
                "uploadwill": "Subir un novo arquivo de firmware (.bin) substituirá o firmware existente.",
            }
        }
    },
    "menu": {
        "demo": "Aplicación de demostración",
        "connections": "Conexións",
        "connections.mqtt": "MQTT",
        "connections.ntp": "NTP",
        "wifi": "Wi-Fi",
        "users": "Usuarios",
        "system": "Sistema",
        "system.status": "Estado do sistema",
        "firmware.update": "Actualización de firmware"
    },
    "stores": {
        "user": {
            "logout": "Desconectar usuario"
        }
    },
    "ntfc": {
        "success": {
            "securityupdated": "Configuracións de seguridade actualizadas.",
            "brokerupdated": "Configuracións do corretor actualizadas.",
            "apupdated": "Configuracións do punto de acceso actualizadas.",
            "wifiupdated": "Configuracións Wi-Fi actualizadas.",
            "otaupdated": "Configuracións OTA actualizadas.",
        },
        "error": {
            "usernotauth": "Usuario non autorizado."
        }
    },
    "ok": "OK",
    "cancel": "Cancelar",
    "save": "Gardar",
    "and": "e",
    "built": "front-end construído con",
    "updating.firmware": "Actualizando firmware",
    "close": "Pechar",
    "loading": "Cargando...",
    "error": "Erro:",
    "abort": "Abortar",
    "restart": "Reiniciar",
    "reset": "Reconfiguración de fábrica",
    "sleep": "Modo de espera",
    "yes": "Si",
    "update": "Actualizar",
    "connected": "Conectado",
    "seconds": "Segundos",
    "chars": "Caracteres",
    "applysettings": "Aplicar configuracións",
    "status": "Estado",
    "active": "Activo",
    "inactive": "Inactivo",
    "server": "Servidor",
    "day": " día",
    "hour": " hora",
    "minute": " minuto",
    "second": " segundo",
    "password": "Contrasinal",
    "username": "Nome de usuario",
    "used": "usado",
    "of": "de",
    "free": "libre",
    "port": "Porto",
    "upload": "Subir"
};
