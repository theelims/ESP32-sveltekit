import type { UserConfig, Plugin } from 'vite';

export default function viteLittleFS(): Plugin[] {
	return [
		{
			name: 'vite-plugin-littlefs',
			enforce: 'post',
			apply: 'build',

			async config(config, _configEnv) {
				const { assetFileNames, chunkFileNames, entryFileNames } =
					config.build?.rollupOptions?.output;

        // Handle Server-build + Client Assets
        config.build.rollupOptions.output = {
          ...config.build?.rollupOptions?.output,
          assetFileNames: assetFileNames.replace('.[hash]', '')
        }

        // Handle Client-build
        if (config.build?.rollupOptions?.output.chunkFileNames.includes('hash')) {

          config.build.rollupOptions.output = {
            ...config.build?.rollupOptions?.output,
            chunkFileNames: chunkFileNames.replace('.[hash]', ''),
            entryFileNames: entryFileNames.replace('.[hash]', ''),
          }
        }
      }
    }
  ]
}
