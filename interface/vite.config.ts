import { sveltekit } from '@sveltejs/kit/vite';
import type { UserConfig, Plugin } from 'vite';
import Icons from 'unplugin-icons/vite'

const viteLittleFS = (): Plugin[] => {
  return [{
    name: 'vite-plugin-littlefs',
    enforce: 'post',

    async config(config, _configEnv) {
        const {
          assetFileNames, chunkFileNames, entryFileNames
        } = config.build?.rollupOptions?.output

      // Handle Server-build + Client Assets
      config.build.rollupOptions.output = {
        ...config.build?.rollupOptions?.output,
        assetFileNames: assetFileNames.replace('[hash].', '')
      }

      // Handle Client-build
      if (config.build?.rollupOptions?.output.chunkFileNames.includes('hash')) {

        config.build.rollupOptions.output = {
          ...config.build?.rollupOptions?.output,
          chunkFileNames: chunkFileNames.replace('[hash].', ''),
          entryFileNames: entryFileNames.replace('[hash].', ''),
        }
      }
    }
  }]
}

const config: UserConfig = {
	plugins: [
        sveltekit(),
        Icons({
            compiler: 'svelte',
          }),
        viteLittleFS()
    ],
    server: {
        proxy: {
          // Proxying REST: http://localhost:5173/rest/bar -> http://192.168.1.83/rest/bar
          '/rest': {
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
      }
};

export default config;
