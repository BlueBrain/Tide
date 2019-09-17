export default function castArray(v: any, defaultValue: any[] = []): any[] {
    if (typeof v === 'undefined') return defaultValue;
    if (Array.isArray(v)) return v;
    return [v];
}
