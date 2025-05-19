import { writable } from 'svelte/store';
import { goto } from '$app/navigation';
import { jwtDecode } from 'jwt-decode';

export type userProfile = {
	username: string;
	admin: boolean;
	bearer_token: string;
};

type decodedJWT = {
	username: string;
	admin: boolean;
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
			const decoded: decodedJWT = jwtDecode(access_token);
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
			// redirect to login page
			goto('/');
		}
	};
}

export const user = createStore();
