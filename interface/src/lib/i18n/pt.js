export default {
	"welcome": "Bem-vindo ao <br /> sveltekit",
	"lib": {
		"components": {
			"UpdateIndicator": {
				"available": "Atualização de firmware disponível.",
				"confirmGithubUpdate": {
					"title": "Confirme a instalação do novo firmware no dispositivo",
					"message": "Tem certeza de que deseja sobrescrever o firmware existente com um novo?"
				}
			}
		}
	},
	"routes": {
		"page": {
			"s1": "Um framework simples, seguro e extensível para projetos IoT para plataformas ESP32 com resposta",
			"s2": "Iniciar Demo",
		},
		"connections": {
			"ipmust": "Deve ser um endereço IPv4 válido ou URL",
			"mqtt": {
				"MQTT": {
					"disabled": "MQTT desativado",
					"id": "ID do cliente",
					"change": "Alterar configurações MQTT",
					"enable": "Ativar MQTT",
					"host": "Host",
					"portmust": "O número da porta deve estar entre 0 e 65536",
					"secondsmust": "Deve estar entre 1 e 600 segundos",
					"charmust": "Deve estar entre 64 e 4096 caracteres",
					"clean": "Sessão limpa?",
					"keep": "Manter vivo",
					"length": "Comprimento máximo do tópico",
					"disconnectReason": {
						"00": "TCP desconectado",
						"01": "Versão do protocolo inaceitável",
						"02": "Identificador rejeitado",
						"03": "Servidor indisponível",
						"04": "Credenciais malformadas",
						"05": "Não autorizado",
						"06": "Memória insuficiente",
						"07": "TLS rejeitado"
					}
				},
				"MQTTConfig": {
					"title": "Configurações do broker MQTT",
					"alert": "O LED é controlável via MQTT com o projeto de demonstração projetado para trabalhar com o recurso de descoberta automática do Home Assistant.",
					"uniqueid": "ID único",
					"idmust": "O ID único deve ter entre 3 e 32 caracteres",
					"name": "Nome",
					"namemust": "O nome deve ter entre 3 e 32 caracteres",
					"path": "Caminho MQTT",
					"pathlimited": "O caminho MQTT é limitado a 64 caracteres"
				}
			},
			"ntp": {
				"time": "Hora da rede",
				"server": "Servidor NTP",
				"localtime": "Hora local",
				"utctime": "Hora UTC",
				"uptime": "Tempo de atividade",
				"changesettings": "Alterar configurações NTP",
				"enable": "Ativar NTP",
				"pick": "Escolher fuso horário",
			}
		},
		"wifi": {
			"ip": "Endereço IP",
			"mac": "Endereço MAC",
			"gateway": "IP do gateway",
			"mask": "Máscara de sub-rede",
			"ssidmust": "SSID deve ter entre 2 e 32 caracteres",
			"localip": "IP local",
			"ipmust": "Deve ser um endereço IPv4 válido",
			"scannet": "Escanear redes",
			"channel": "Canal",
			"ap": {
				"title": "Ponto de acesso",
				"apclients": "Clientes AP",
				"change": "Alterar configurações AP",
				"provideap": "Fornecer ponto de acesso ...",
				"prefchannel": "Canal preferido",
				"channelmust": "Deve ser o canal 1 ao 13",
				"maxcli": "Máximo de clientes",
				"max8": "Máximo de 8 clientes permitidos",
				"hidessid": "Ocultar SSID",
			},
			"wifi": {
				"change": "Alterar configurações Wi-Fi",
				"hostname": "Nome do host",
				"hostnamemust": "O nome do host deve ter entre 2 e 32 caracteres",
				"staticip": "Configuração IP estático?",
			},
			"scan": {
				"open": "Abrir",
				"scanning": "Escanear ...",
				"security": "Segurança",
				"again": "Escanear novamente",
			}
		},
		"user": {
			"title": "Gerenciar usuários",
			"admin": "Administrador",
			"edit": "Editar",
			"securitysettings": "Configurações de segurança",
			"jwt": "O segredo JWT é usado para assinar tokens de autenticação. Se você modificar o segredo JWT, todos os usuários serão desconectados.",
			"jwtsecret": "Segredo JWT",
			"confirmdel":"Confirmar exclusão de usuário",
			"messagedel":'Tem certeza de que deseja excluir o usuário "',
			"edituser":"Editar usuário",
			"adduser":"Adicionar usuário"
		},
		"edituser":{
			"usernamemust": "O nome de usuário deve ter entre 3 e 32 caracteres",
			"isadmin":"É administrador?",
		},
		"system":{
			"status":{
				"title":"Status do sistema",
				"device":"Dispositivo (Plataforma / SDK)",
				"firmware":"Versão do firmware",
				"cpufr":"Frequência da CPU",
				"heap":"Heap (Livre / Max Alloc)",
				"psram":"PSRAM (Tamanho / Livre)",
				"sketch":"Sketch (Usado / Livre)",
				"chip":"Chip Flash (Tamanho / Velocidade)",
				"files":"Sistema de arquivos (Usado / Total)",
				"temperature":"Temperatura do núcleo",
				"confirmrestart":"Confirmar reinício",
				"messagerestart":"Tem certeza de que deseja reiniciar o dispositivo?",
				"confirmreset":"Confirmar reset de fábrica",
				"messagereset":"Tem certeza de que deseja redefinir o dispositivo para suas configurações de fábrica?",
					"confirmsleep":"Confirmar modo de espera",
				"messagesleep":"Tem certeza de que deseja colocar o dispositivo em modo de espera?",
			},
			"update":{
				"confirmflash":"Confirme a instalação do novo firmware no dispositivo",
				"messageflash":"Tem certeza de que deseja sobrescrever o firmware existente com um novo?",
				"githubtitle":"Gerenciador de firmware Github",
				"release":"Lançamento",
				"releasedate":"Data de lançamento",
				"experimental":"Experimental",
				"install":"Instalar",
				"pleaseconnect":"Conecte-se a uma rede com acesso à Internet para realizar uma atualização de firmware.",
				"otatitle":"Atualização de firmware OTA",
				"otaenable":"Ativar atualizações OTA remotas?",
				"portmust1025":"O número da porta deve estar entre 1025 e 65536",
				"confirmupload":"Confirmar instalação no dispositivo",
				"messageupload":"Tem certeza de que deseja sobrescrever o firmware existente com um novo?",
				"uploadfirmware":"Carregar firmware",
				"uploadwill":"Carregar um novo arquivo de firmware (.bin) substituirá o firmware existente.",
			}
		}
	},
	"menu":{
		"demo":"App de demonstração",
		"connections":"Conexões",
		"connections.mqtt":"MQTT",
		"connections.ntp":"NTP",
		"wifi":"Wi-Fi",
		"users":"Usuários",
		"system":"Sistema",
		"system.status":"Status do sistema",
		"firmware.update":"Atualização de firmware"
	},
	"stores": {
		"user": {
			"logout": "Desconectar usuário"
		}
	},
	"ntfc": {
		"success": {
			"securityupdated": "Configurações de segurança atualizadas.",
			"brokerupdated": "Configurações do broker atualizadas.",
			"apupdated": "Configurações do ponto de acesso atualizadas.",
			"wifiupdated": "Configurações Wi-Fi atualizadas.",
			"otaupdated":"Configurações OTA atualizadas.",
		},
		"error": {
			"usernotauth": "Usuário não autorizado."
		}
	},
	"ok": "OK",
	"cancel": "Cancelar",
	"save":"Salvar",
	"and": "e",
	"built": "front-end construído com",
	"updating.firmware": "Atualizando firmware",
	"close": "Fechar",
	"loading": "Carregando...",
	"error": "Erro:",
	"abort": "Abortar",
	"restart":"Reiniciar",
	"reset":"Reset de fábrica",
	"sleep":"Modo de espera",
	"yes": "Sim",
	"update": "Atualizar",
	"connected": "Conectado",
	"seconds": "Segundos",
	"chars": "Caracteres",
	"applysettings": "Aplicar configurações",
	"status": "Status",
	"active": "Ativo",
	"inactive": "Inativo",
	"server": "Servidor",
	"day": " dia",
	"hour": " hora",
	"minute": " minuto",
	"second": " segundo",
	"password": "Senha",
	"username": "Nome de usuário",
	"used":"usado",
	"of":"de",
	"free":"livre",
	"port": "Porta",
	"upload":"Carregar"
};
