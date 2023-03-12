import { sveltekit } from '@sveltejs/kit/vite';
import type { UserConfig } from 'vite';
import Icons from 'unplugin-icons/vite'

const config: UserConfig = {
	plugins: [
        sveltekit(),
        Icons({
            compiler: 'svelte',
          })
    ],
    server: {
        proxy: {
          // Proxying REST: http://localhost:5173/api/bar -> http://192.168.1.83/api/bar
          '/api': {
            target: 'http://192.168.1.83',
            changeOrigin: true,
          },
          // Proxying websockets ws://localhost:5173/ws -> ws://192.168.1.83/ws
          '/ws': {
            target: 'ws://192.168.1.83',
            changeOrigin: true,
            ws: true,
          },
        },
      },
};

export default config;
