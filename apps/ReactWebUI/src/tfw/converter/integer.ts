export default function castInteger(v: any, defaultValue = 0): number {
    const defVal = Math.floor(.5 + defaultValue);

    switch (typeof v) {
        case "boolean":
            return v ? 1 : 0;
        case "number":
            return Math.floor(.5 + v);
        case "string":
            const text = v.trim().toLowerCase();
            if (text.startsWith("0x")) {
                const hexa = parseInt(text.substr(2), 16);
                return isNaN(hexa) ? defVal : hexa;
            }
            if (text.startsWith("0b")) {
                const hexa = parseInt(text.substr(2), 2);
                return isNaN(hexa) ? defVal : hexa;
            }
            if (text.startsWith("0o")) {
                const hexa = parseInt(text.substr(2), 8);
                return isNaN(hexa) ? defVal : hexa;
            }
            const num = parseFloat(text);
            if (isNaN(num)) return defVal;
            return Math.floor(.5 + num);
        default:
            return defVal;
    }
}
