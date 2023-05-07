import type { PageLoad } from './$types';
import { user, security } from '$lib/stores/user';

let userdata = {};
let securitydata = {};

user.subscribe((data) => {
	userdata = data;
});

security.subscribe((data) => {
	securitydata = data;
});

export const load = (async ({ fetch }) => {
	let returnobj = { title: 'System' };
	try {
		const sysStatRes = await fetch('/rest/systemStatus', {
			method: 'GET',
			headers: {
				Authorization: securitydata.security ? 'Bearer ' + userdata.bearer_token : 'Basic',
				'Content-Type': 'application/json'
			}
		});
		returnobj.systemStatus = await sysStatRes.json();

		if (securitydata.admin_required) {
			const otaSettingsRes = await fetch('/rest/otaSettings', {
				method: 'GET',
				headers: {
					Authorization: 'Bearer ' + userdata.bearer_token,
					'Content-Type': 'application/json'
				}
			});
			returnobj.otaSettings = await otaSettingsRes.json();
		}
		return returnobj;
	} catch (error) {
		console.error('Error:', error);
	}
}) satisfies PageLoad;
