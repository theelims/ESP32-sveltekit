import type { Alarm } from '$lib/types/models';

export function createAlarmState() {
let alarms: Alarm[] = $state([] as Alarm[]);

return ({
    get alarms() {
        return alarms;
    },
    set alarms(newAlarms: Alarm[]) {
        alarms = newAlarms;
    }
});
}

export const alarmLog = createAlarmState()