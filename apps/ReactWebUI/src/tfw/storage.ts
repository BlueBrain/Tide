/**
 * @example
 * import Storage from "./tfw/storage";
 * Storage.local.set("state", {lang:"jp", currentPage: "checkpoint-list"});
 * const state = Storage.local.get("state", {lang:"jp", currentPage: "checkpoint-list"});
 * Storage.local.del("state");
 */

export default {
    local: makeStorage(window.localStorage),
    session: makeStorage(window.sessionStorage)
};

interface IStorage {
    getItem: (key: string)=>any;
    setItem: (key: string, val: any)=>void;
    removeItem: (key: string)=>void;
}

function makeStorage(storage:IStorage) {
    return {
        get(key: string, defaultValue: any): any {
            const value = storage.getItem(key);
            if( value === null) return defaultValue;
            try {
                return JSON.parse(value);
            }
            catch( ex ) {
                console.error(`storage["${key}"] is not parsable as a JSON object!\n`
                + `Its value is: "${value}".\nThis item will be deleted.`);
                storage.removeItem(key);
                return defaultValue;
            }
        },

        set(key: string, value: any) {
            storage.setItem(key, JSON.stringify(value));
        },

        del(key: string) {
            storage.removeItem(key);
        }
    }
}
