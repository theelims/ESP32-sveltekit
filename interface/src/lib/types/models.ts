export type WifiStatus = {
	status: number;
	local_ip: string;
	mac_address: string;
	rssi: number;
	ssid: string;
	bssid: string;
	channel: number;
	subnet_mask: string;
	gateway_ip: string;
	dns_ip_1: string;
	dns_ip_2?: string;
};

export type WifiSettings = {
	hostname: string;
	connection_mode: number;
	wifi_networks: KnownNetworkItem[];
};

export type KnownNetworkItem = {
	ssid: string;
	password: string;
	static_ip_config: boolean;
	local_ip?: string;
	subnet_mask?: string;
	gateway_ip?: string;
	dns_ip_1?: string;
	dns_ip_2?: string;
};

export type NetworkItem = {
	rssi: number;
	ssid: string;
	bssid: string;
	channel: number;
	encryption_type: number;
};

export type ApStatus = {
	status: number;
	ip_address: string;
	mac_address: string;
	station_num: number;
};

export type ApSettings = {
	provision_mode: number;
	ssid: string;
	password: string;
	channel: number;
	ssid_hidden: boolean;
	max_clients: number;
	local_ip: string;
	gateway_ip: string;
	subnet_mask: string;
};

export type LightState = {
	led_on: boolean;
};

export type BrokerSettings = {
	mqtt_path: string;
	name: string;
	unique_id: string;
};

export type GatewayMQTTSettings = {
	mqtt_path: string;
};

export type NTPStatus = {
	status: number;
	utc_time: string;
	local_time: string;
	server: string;
	uptime: number;
};

export type NTPSettings = {
	enabled: boolean;
	server: string;
	tz_label: string;
	tz_format: string;
};

export type Analytics = {
	max_alloc_heap: number;
	psram_size: number;
	free_psram: number;
	free_heap: number;
	total_heap: number;
	min_free_heap: number;
	core_temp: number;
	fs_total: number;
	fs_used: number;
	uptime: number;
};

export type RSSI = {
	rssi: number;
	ssid: string;
};

export type Battery = {
	soc: number;
	charging: boolean;
};

export type DownloadOTA = {
	status: string;
	progress: number;
	error: string;
};

export type StaticSystemInformation = {
	esp_platform: string;
	firmware_version: string;
	cpu_freq_mhz: number;
	cpu_type: string;
	cpu_rev: number;
	cpu_cores: number;
	sketch_size: number;
	free_sketch_space: number;
	sdk_version: string;
	arduino_version: string;
	flash_chip_size: number;
	flash_chip_speed: number;
	cpu_reset_reason: string;
};

export type SystemInformation = Analytics & StaticSystemInformation;

export type MQTTStatus = {
	enabled: boolean;
	connected: boolean;
	client_id: string;
	last_error: string;
};

export type MQTTSettings = {
	enabled: boolean;
	uri: string;
	username: string;
	password: string;
	client_id: string;
	keep_alive: number;
	clean_session: boolean;
};

export type HekatronAlarm = {
	startTime: Date;
	endTime: Date;
	endingReason: number;
}

export type HekatronComponent = {
	model: number;
	sn: number;
	productionDate: Date;
};

export type HekatronDevice = {
	smokeDetector: HekatronComponent;
	radioModule: HekatronComponent;
	location: string;
	alarms: HekatronAlarm[];
	isAlarming: boolean;
};

export type HekatronDevices = {
	devices: HekatronDevice[];
};

export type AlarmState = {
	isAlarming: boolean;
};

export type VisualizerSettings = {
	showDetails: boolean;
	showMetadata: boolean;
};

export type PacketType = {
	name: string;
	cssClass: string;
	packetLength: number;
	description: string;
};

export const PacketTypes: {
	Commissioning: PacketType;
	DiscoveryRequest: PacketType;
	DiscoveryResponse: PacketType;
	LineTest: PacketType;
	Alarming: PacketType;
	Unknown: PacketType;
} = {
	Commissioning: {
		name: 'Commissioning',
		cssClass: 'type-comissioning',
		packetLength: 37,
		description: 'Commissioning of new alarm line.'
	},
	DiscoveryRequest: {
		name: 'Discovery Request?',
		cssClass: 'type-discovery-request',
		packetLength: 28,
		description: 'Purpose unknown, possibly a device discovery request.'
	},
	DiscoveryResponse: {
		name: 'Discovery Response?',
		cssClass: 'type-discovery-response',
		packetLength: 32,
		description: 'Purpose unknown, possibly a device discovery response.'
	},
	LineTest: {
		name: 'Line Test',
		cssClass: 'type-linetest',
		packetLength: 29,
		description: 'Packets sent during line test function.'
	},
	Alarming: {
		name: 'Alarming',
		cssClass: 'type-alarm',
		packetLength: 36,
		description: 'Packet starting and silencing/stopping alarm.'
	},
	Unknown: {
		name: 'Unknown',
		cssClass: 'type-unknown',
		packetLength: 0, // No specific length for unknown packets
		description: 'Unknown packet type.'
	}
};

export type GeneralInfo = {
	counter: number;
	firstRadioModuleSN: number;
	firstLocation: string;
	secondRadioModuleSN: number;
	secondLocation: string;
	lineID: number;
	hops: number;
};

export type CommissioningInfo = {
	newLineID: number;
	timeStr: string;
};

export type DiscoveryResponseInfo = {
	requestingRadioModule: number;
	requestingLocation: string;
};

export type Packet = {
	id: number;
	timestampFirst: number;
	timestampLast: number;
	type: PacketType;
	data: Uint8Array;
	counter: number;
	hash: number;
	generalInfo: GeneralInfo | null;
	specificInfo: CommissioningInfo | DiscoveryResponseInfo | null;
};

export type AlarmLine = {
	id: number;
	name: string;
	created: Date;
	acquisition: number;
};

export type AlarmLines = {
	lines: AlarmLine[];
};

export type CC1101State = {
	state_success: boolean;
	state: number;
}