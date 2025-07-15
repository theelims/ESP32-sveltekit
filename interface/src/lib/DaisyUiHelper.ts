export function daisyColor(name: string, opacity: number = 100) {
	const color = getComputedStyle(document.documentElement).getPropertyValue(name);
	// console.debug(`daisyColor: name=${name}, color=${color}, opacity=${opacity}`);
	// console.debug(`${color}`);
	// add transparency to the color if opacity is less than 100
	if (opacity < 100) {
		// Convert opacity to a percentage
		const alpha = Math.min(Math.max(Math.round(opacity), 0), 100) / 100;
		// Remove any existing alpha value and trailing ')' from the oklch color
		const oklchColor = color.replace(/(\/\s*\d+(\.\d+)?\))|\)$/, '').trim();
		// Append the new alpha value
		// console.debug(`oklchColor: ${oklchColor} / ${alpha})`);
		return `${oklchColor} / ${alpha})`;
	}
	return `${color}`; //   / ${Math.min(Math.max(Math.round(opacity), 0), 100)}%)`;
}
