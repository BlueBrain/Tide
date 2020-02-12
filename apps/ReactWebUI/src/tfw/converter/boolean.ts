export default function castBoolean(v: any, defaultValue = false): boolean {
    switch (typeof v) {
        case "undefined":
            return defaultValue;
        case "boolean":
            return v;
        case "number":
            return v !== 0;
        case "string":
            const text = v.trim().toLowerCase();
            if (text === 'true' || text === 'yes') return true;
            const num = parseInt(text);
            if (!isNaN(num)) return num !== 0;
            return false;
        default:
            return false;
    }
}
