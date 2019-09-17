const RX_CSS_UNIT = /^(-?[.0-9]+)[ \n\r]*([a-z%]*)/;

export default function castUnit(v: any, defaultValue: string = "100%") {
    if (typeof v === 'number') return `${v}px`;
    if (typeof v !== 'string') return defaultValue;
    const text = `${v}`.trim().toLowerCase();
    if (text === 'auto' || text === 'inherit') return text;
    if (text.startsWith("calc(")) return text;
    const m = RX_CSS_UNIT.exec(text);
    if (!m) return defaultValue;
    const scalar = parseFloat(m[1]);
    if (isNaN(scalar) || scalar === 0) return "0";
    const unit = m[2].length < 1 ? "px" : m[2];
    return `${scalar}${unit}`;
}
