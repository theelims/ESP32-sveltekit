import { sveltekit } from '@sveltejs/kit/vite';
import type { UserConfig } from 'vite';
import Icons from 'unplugin-icons/vite';
import tailwindcss from '@tailwindcss/vite';

const config: UserConfig = {
	plugins: [
		sveltekit(),
		Icons({
			compiler: 'svelte'
		}),
		tailwindcss()
	],
	server: {
		proxy: {
			// Proxying REST: http://localhost:5173/rest/bar -> http://192.168.1.83/rest/bar
			'/rest': {
				target: 'http://192.168.1.111',
				changeOrigin: true
			},
			// Proxying websockets ws://localhost:5173/ws -> ws://192.168.1.83/ws
			'/ws': {
				target: 'ws://192.168.1.111',
				changeOrigin: true,
				ws: true
			}
		}
	},
	build: {
		minify: 'terser',
		sourcemap: false,
		rollupOptions: {
			output: {
				manualChunks(id) {
					if (id.includes('node_modules')) return 'vendor';
				}
			}
		},
		cssCodeSplit: true
	}
};

export default config;
