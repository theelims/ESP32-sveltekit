import type { PluginOption, normalizePath, Plugin } from 'vite';
import { resolve, relative, sep } from 'path';
import { readdirSync, existsSync, unlinkSync, readFileSync, createWriteStream } from 'fs';
import { Zlib } from 'zlib';
import mime from 'mime-types';

export interface Options {
	/**
	 * Output Directory
	 */
	outputPath: string;
	/**
	 * bytes per line
	 * @default 20
	 */
	bytesPerLine?: number;
	/**
	 * Indention of header file
	 * @default "  "
	 */
	indent?: string;
	/**
	 * Includes
	 * @default 'ARDUINO_INCLUDES'
	 */
	includes?: string;
}

const ARDUINO_INCLUDES = '#include <Arduino.h>\n\n';

function getFilesSync(dir, files = []) {
	readdirSync(dir, { withFileTypes: true }).forEach((entry) => {
		const entryPath = resolve(dir, entry.name);
		if (entry.isDirectory()) {
			getFilesSync(entryPath, files);
		} else {
			files.push(entryPath);
		}
	});
	return files;
}

function coherseToBuffer(input) {
	return Buffer.isBuffer(input) ? input : Buffer.from(input);
}

function cleanAndOpen(path) {
	if (existsSync(path)) {
		unlinkSync(path);
	}
	return createWriteStream(path, { flags: 'w+' });
}

export default function ProgmemGenerator(options?: Options): PluginOption {
	const outputPath = options.outputPath;
	const bytesPerLine = options?.bytesPerLine || 20;
	const indent = options?.indent || '  ';
	const includes = options?.includes || ARDUINO_INCLUDES;

	return {
		name: 'vite-plugin-progmem',
		apply: 'build',
		closeBundle() {
			console.log('\x1b[36m%s\x1b[0m', `Creating PROGMEM Header file - "${outputPath}"`);
			const fileInfo = [];
			const root = process.cwd();
			const writeStream = cleanAndOpen(resolve(root, outputPath));
			try {
				const writeIncludes = () => {
					writeStream.write(includes);
				};
				const writeFile = (relativeFilePath, buffer) => {
					const variable = 'ESP_REACT_DATA_' + fileInfo.length;
					const mimeType = mime.lookup(relativeFilePath);
					var size = 0;
					writeStream.write('const uint8_t ' + variable + '[] PROGMEM = {');
					const zipBuffer = Zlib.gzipSync(buffer);
					zipBuffer.forEach((b) => {
						if (!(size % bytesPerLine)) {
							writeStream.write('\n');
							writeStream.write(indent);
						}
						writeStream.write('0x' + ('00' + b.toString(16).toUpperCase()).substr(-2) + ',');
						size++;
					});
					if (size % bytesPerLine) {
						writeStream.write('\n');
					}
					writeStream.write('};\n\n');
					fileInfo.push({
						uri: '/' + relativeFilePath.replace(sep, '/'),
						mimeType,
						variable,
						size
					});
				};
				const writeFiles = () => {
					// process static files
					const buildPath = '';  // TODO Where do I get the ouptut folder?
					for (const filePath of getFilesSync(buildPath)) {
						const readStream = readFileSync(filePath);
						const relativeFilePath = relative(buildPath, filePath);
						writeFile(relativeFilePath, readStream);
					}
					// process assets
					const { assets } = {}; // TODO Where do I get the assets from?
					Object.keys(assets).forEach((relativeFilePath) => {
						writeFile(relativeFilePath, coherseToBuffer(assets[relativeFilePath].source()));
					});
				};
				const generateWWWClass = () => {
					// prettier-disable-next-line max-len
					return `typedef std::function<void(const String& uri, const String& contentType, const uint8_t * content, size_t len)> RouteRegistrationHandler;
class WWWData {
${indent}public: 
${indent.repeat(2)}static void registerRoutes(RouteRegistrationHandler handler) {
${fileInfo
	.map(
		(file) =>
			`${indent.repeat(3)}handler("${file.uri}", "${file.mimeType}", ${file.variable}, ${
				file.size
			});`
	)
	.join('\n')}
${indent.repeat(2)}}
};
`;
				};
				const writeWWWClass = () => {
					writeStream.write(generateWWWClass());
				};
				writeIncludes();
				writeFiles();
				writeWWWClass();
				writeStream.on('finish', () => {
					console.log('\x1b[31m%s\x1b[0m', '  - Done');
				});
			} finally {
				writeStream.end();

				//	console.log('\x1b[31m%s\x1b[0m', '  - Something went wrong while building zip file!');
			}
		}
	};
}
