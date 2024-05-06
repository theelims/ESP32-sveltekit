export function daisyColor(name: string, opacity: number = 100) {
	const color = getComputedStyle(document.documentElement).getPropertyValue(name);
	return `oklch(${color} / ${Math.min(Math.max(Math.round(opacity), 0), 100)}%)`;
}
