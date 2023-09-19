export default {
    "welcome": "Bienvenue à <br /> sveltekit",
    "lib": {
        "components": {
            "UpdateIndicator": {
                "available": "Mise à jour du firmware disponible.",
                "confirmGithubUpdate": {
                    "title": "Confirmer le flashage du nouveau firmware sur l'appareil",
                    "message": "Êtes-vous sûr de vouloir remplacer le firmware existant par un nouveau ?"
                }
            }
        }
    },
    "routes": {
        "page": {
            "s1": "Un cadre simple, sécurisé et extensible pour les projets IoT pour les plateformes ESP32 avec une réponse",
            "s2": "Démarrer la démo",
        },
        "connections": {
            "ipmust": "Doit être une adresse IPv4 valide ou une URL",
            "mqtt": {
                "MQTT": {
                    "disabled": "MQTT désactivé",
                    "id": "ID client",
                    "change": "Modifier les paramètres MQTT",
                    "enable": "Activer MQTT",
                    "host": "Hôte",
                    "portmust": "Le numéro de port doit être compris entre 0 et 65536",
                    "secondsmust": "Doit être entre 1 et 600 secondes",
                    "charmust": "Doit être entre 64 et 4096 caractères",
                    "clean": "Session propre ?",
                    "keep": "Garder en vie",
                    "length": "Longueur maximale du sujet",
                    "disconnectReason": {
                        "00": "TCP déconnecté",
                        "01": "Version de protocole inacceptable",
                        "02": "Identifiant rejeté",
                        "03": "Serveur indisponible",
                        "04": "Identifiants mal formés",
                        "05": "Non autorisé",
                        "06": "Mémoire insuffisante",
                        "07": "TLS rejeté"
                    }
                },
                "MQTTConfig": {
                    "title": "Paramètres du courtier MQTT",
                    "alert": "La LED est contrôlable via MQTT avec le projet de démonstration conçu pour fonctionner avec la fonction de découverte automatique de Home Assistant.",
                    "uniqueid": "ID unique",
                    "idmust": "L'ID unique doit comporter entre 3 et 32 caractères",
                    "name": "Nom",
                    "namemust": "Le nom doit comporter entre 3 et 32 caractères",
                    "path": "Chemin MQTT",
                    "pathlimited": "Le chemin MQTT est limité à 64 caractères"
                }
            },
            "ntp": {
                "time": "Heure réseau",
                "server": "Serveur NTP",
                "localtime": "Heure locale",
                "utctime": "Heure UTC",
                "uptime": "Temps de fonctionnement",
                "changesettings": "Modifier les paramètres NTP",
                "enable": "Activer NTP",
                "pick": "Choisir le fuseau horaire",
            }
        },
        "wifi": {
            "ip": "Adresse IP",
            "mac": "Adresse MAC",
            "gateway": "IP de la passerelle",
            "mask": "Masque de sous-réseau",
            "ssidmust": "Le SSID doit comporter entre 2 et 32 caractères",
            "localip": "IP locale",
            "ipmust": "Doit être une adresse IPv4 valide",
            "scannet": "Scanner les réseaux",
            "channel": "Canal",
            "ap": {
                "title": "Point d'accès",
                "apclients": "Clients AP",
                "change": "Modifier les paramètres AP",
                "provideap": "Fournir un point d'accès ...",
                "prefchannel": "Canal préféré",
                "channelmust": "Doit être le canal 1 à 13",
                "maxcli": "Clients max",
                "max8": "Maximum 8 clients autorisés",
                "hidessid": "Masquer le SSID",
            },
            "wifi": {
                "change": "Modifier les paramètres Wi-Fi",
                "hostname": "Nom d'hôte",
                "hostnamemust": "Le nom d'hôte doit comporter entre 2 et 32 caractères",
                "staticip": "Config IP statique ?",
            },
            "scan": {
                "open": "Ouvrir",
                "scanning": "Analyse ...",
                "security": "Sécurité",
                "again": "Scanner à nouveau",
            }
        },
        "user": {
            "title": "Gérer les utilisateurs",
            "admin": "Admin",
            "edit": "Modifier",
            "securitysettings": "Paramètres de sécurité",
            "jwt": "Le secret JWT est utilisé pour signer les jetons d'authentification. Si vous modifiez le secret JWT, tous les utilisateurs seront déconnectés.",
            "jwtsecret": "Secret JWT",
            "confirmdel": "Confirmer la suppression de l'utilisateur",
            "messagedel": 'Êtes-vous sûr de vouloir supprimer l\'utilisateur "',
            "edituser": "Modifier l'utilisateur",
            "adduser": "Ajouter un utilisateur"
        },
        "edituser": {
            "usernamemust": "Le nom d'utilisateur doit comporter entre 3 et 32 caractères",
            "isadmin": "Est administrateur ?",
        },
        "system": {
            "status": {
                "title": "Statut du système",
                "device": "Appareil (Plateforme / SDK)",
                "firmware": "Version du firmware",
                "cpufr": "Fréquence CPU",
                "heap": "Tas (Libre / Alloc max)",
                "psram": "PSRAM (Taille / Libre)",
                "sketch": "Esquisse (Utilisé / Libre)",
                "chip": "Puce Flash (Taille / Vitesse)",
                "files": "Système de fichiers (Utilisé / Total)",
                "temperature": "Température du cœur",
                "confirmrestart": "Confirmer le redémarrage",
                "messagerestart": "Êtes-vous sûr de vouloir redémarrer l'appareil ?",
                "confirmreset": "Confirmer la réinitialisation d'usine",
                "messagereset": "Êtes-vous sûr de vouloir réinitialiser l'appareil à ses paramètres d'usine ?",
                "confirmsleep": "Confirmer la mise en veille",
                "messagesleep": "Êtes-vous sûr de vouloir mettre l'appareil en veille ?",
            },
            "update": {
                "confirmflash": "Confirmer le flashage du nouveau firmware sur l'appareil",
                "messageflash": "Êtes-vous sûr de vouloir remplacer le firmware existant par un nouveau ?",
                "githubtitle": "Gestionnaire de firmware Github",
                "release": "Version",
                "releasedate": "Date de sortie",
                "experimental": "Expérimental",
                "install": "Installer",
                "pleaseconnect": "Veuillez vous connecter à un réseau avec accès à Internet pour effectuer une mise à jour du firmware.",
                "otatitle": "Mise à jour du firmware OTA",
                "otaenable": "Activer les mises à jour OTA à distance ?",
                "portmust1025": "Le numéro de port doit être compris entre 1025 et 65536",
                "confirmupload": "Confirmer le flashage de l'appareil",
                "messageupload": "Êtes-vous sûr de vouloir remplacer le firmware existant par un nouveau ?",
                "uploadfirmware": "Télécharger le firmware",
                "uploadwill": "Le téléchargement d'un nouveau fichier firmware (.bin) remplacera le firmware existant.",
            }
        }
    },
    "menu": {
        "demo": "Appli démo",
        "connections": "Connexions",
        "connections.mqtt": "MQTT",
        "connections.ntp": "NTP",
        "wifi": "Wi-Fi",
        "users": "Utilisateurs",
        "system": "Système",
        "system.status": "Statut du système",
        "firmware.update": "Mise à jour du firmware"
    },
    "stores": {
        "user": {
            "logout": "Déconnexion de l'utilisateur"
        }
    },
    "ntfc": {
        "success": {
            "securityupdated": "Paramètres de sécurité mis à jour.",
            "brokerupdated": "Paramètres du courtier mis à jour.",
            "apupdated": "Paramètres du point d'accès mis à jour.",
            "wifiupdated": "Paramètres Wi-Fi mis à jour.",
            "otaupdated": "Paramètres OTA mis à jour.",
        },
        "error": {
            "usernotauth": "Utilisateur non autorisé."
        }
    },
    "ok": "OK",
    "cancel": "Annuler",
    "save": "Sauvegarder",
    "and": "et",
    "built": "front-end construit avec",
    "updating.firmware": "Mise à jour du firmware",
    "close": "Fermer",
    "loading": "Chargement...",
    "error": "Erreur:",
    "abort": "Abandonner",
    "restart": "Redémarrer",
    "reset": "Réinitialisation d'usine",
    "sleep": "Veille",
    "yes": "Oui",
    "update": "Mettre à jour",
    "connected": "Connecté",
    "seconds": "Secondes",
    "chars": "Caractères",
    "applysettings": "Appliquer les paramètres",
    "status": "Statut",
    "active": "Actif",
    "inactive": "Inactif",
    "server": "Serveur",
    "day": " jour",
    "hour": " heure",
    "minute": " minute",
    "second": " seconde",
    "password": "Mot de passe",
    "username": "Nom d'utilisateur",
    "used": "utilisé",
    "of": "de",
    "free": "libre",
    "port": "Port",
    "upload": "Télécharger"
};
