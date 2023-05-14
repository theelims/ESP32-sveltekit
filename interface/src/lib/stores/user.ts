import { writable, derived } from 'svelte/store';
import jwt_decode from 'jwt-decode';

export type userProfile = {
	username: string;
	admin: boolean;
	bearer_token: string;
};

let empty = {
	username: '',
	admin: false,
	bearer_token: ''
};

function createStore() {
	const { subscribe, set } = writable(empty);

	// retrieve store from sessionStorage / localStorage if available
	const userdata = localStorage.getItem('user');
	if (userdata) {
		set(JSON.parse(userdata));
	}

	return {
		subscribe,
		init: (access_token: string) => {
			const decoded = jwt_decode(access_token);
			const userdata = {
				bearer_token: access_token,
				username: decoded.username,
				admin: decoded.admin
			};
			set(userdata);
			// persist store in sessionStorage / localStorage
			localStorage.setItem('user', JSON.stringify(userdata));
		},
		invalidate: () => {
			console.log('Log out user');
			set(empty);
			// remove localStorage "user"
			localStorage.removeItem('user');
		}
	};
}

export const user = createStore();
