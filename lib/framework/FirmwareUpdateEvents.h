#ifndef FirmwareUpdateEvents_h
#define FirmwareUpdateEvents_h

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2025 hmbacher
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

/**
 * WebSocket event name for OTA firmware updates
 * Used by both UploadFirmwareService and DownloadFirmwareService
 */
#define EVENT_OTA_UPDATE "otastatus"

/**
 * Event JSON Structure:
 * 
 * {
 *   "status": string,        // Current update status
 *   "progress": number,      // Progress percentage (0-100)
 *   "bytes_written": number, // Optional: Bytes written so far
 *   "total_bytes": number,   // Optional: Total bytes to write
 *   "error": string          // Optional: Error message if status is "error"
 * }
 * 
 * Status Values:
 * - "none"      : Frontend-only initial state, no update in progress (never sent by backend)
 * - "preparing" : Update started, preparing to receive data
 * - "progress"  : Update in progress, progress field contains percentage
 * - "finished"  : Update completed successfully, device will restart
 * - "error"     : Update failed, error field contains error message
 * 
 * Example Events:
 * 
 * Preparing:
 * {"status":"preparing","progress":0}
 * 
 * Progress:
 * {"status":"progress","progress":45,"bytes_written":102400,"total_bytes":227328}
 * 
 * Finished:
 * {"status":"finished","progress":100}
 * 
 * Error:
 * {"status":"error","error":"Firmware update failed"}
 */

#endif
