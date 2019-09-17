import Matcher from './filter-matcher'


describe("tool/matcher", () => {
    it("should be true for any empty matcher", () => {
        const m = new Matcher("  , ");
        expect(m.matches("toto")).toBe(true);
    })
});
