export default function castString(value: any, defaultValue: string): string {
    const t = typeof value;
    if (t === 'number' && !isNaN(value)) {
        return `${value}`;
    }
    if (t === 'string') return value;
    return defaultValue;
}
