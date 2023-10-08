export function daisyColor(name: string, opacity: number = 100) {
	const color = getComputedStyle(document.documentElement).getPropertyValue(name);
	return 'hsla(' + color + '/ ' + Math.min(Math.max(Math.round(opacity), 0), 100) + '%)';
}
