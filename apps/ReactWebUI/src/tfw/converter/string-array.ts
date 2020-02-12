import castString from "./string";

/**
 * If `value` is not an array, return `defaultArray`.
 * Else if `defaultString` is undefined, return `value`.
 * Else return a copy of `value` with a castString on all its elements.
 *
 * @param   value         [description]
 * @param   defaultArray  [description]
 * @param   defaultString [description]
 * @returns               [description]
 */
export default function castStringArray(
    value: any, defaultArray: string[] = [], defaultString: string = ""
) {
    if (typeof value === 'string') return [value];
    if (!Array.isArray(value)) return defaultArray;
    if (typeof defaultString === 'undefined') return value;
    return value.map(item => castString(item, defaultString));
}
