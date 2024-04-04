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
	priority_RSSI: boolean;
	wifi_networks: networkItem[];
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
