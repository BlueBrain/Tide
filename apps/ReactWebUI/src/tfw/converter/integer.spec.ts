import castInteger from "./integer";

describe("castInteger( string )", () => {
    const defaultValue = 666;
    ["", "Prout", "*5656", "é485"].forEach(value => {
        it(`should return defaultValue (${defaultValue}) for "${value}"`, () => {
            const result = castInteger(value, defaultValue);
            expect(result).toEqual(defaultValue);
        });
    });
    [
        ["8", 8],
        ["-8", -8],
        ["487421", 487421],
        ["3.141", 3],
        ["-3.141", -3],
        ["2.3e2", 230],
        ["0xFAB", 0xFAB],
        ["0b11101001000101111010111010110", 0b11101001000101111010111010110],
        ["0o4517", 0o4517]
    ].forEach(testCase => {
        const [input: string, expected: number] = testCase;
        it(`should convert "${input}" into ${expected}`, () => {
            const result = castInteger(input, defaultValue);
            expect(result).toEqual(expected);
        });
    });
    it("Should round default value", () => {
        expect(castInteger("PI", 3.14)).toEqual(3);
        expect(castInteger("E", 1.618)).toEqual(2);
    })
});
