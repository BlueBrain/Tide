import Intl from "./intl.ts"

describe("Module Intl", () => {
    it("should translate single language without placeholders", () => {
        Intl.lang = "fr";
        const _ = Intl.make({ "fr": { "tree": "arbre" } });
        expect(_("tree")).toEqual("arbre");
    });

    it("should fallback to antoher language", () => {
        Intl.lang = "fr";
        const _ = Intl.make({ "it": { "tree": "albero" } });
        expect(_("tree")).toEqual("albero");
    });

    it("should fallback to the first valid language", () => {
        Intl.lang = "fr";
        const _ = Intl.make({
            "en": { "apple": "apple" },
            "it": { "tree": "albero" },
            "ge": { "cat": "Katze" }
        });
        expect(_("tree")).toEqual("albero");
    });

    it("should understand placeholders", () => {
        Intl.lang = "fr";
        const _ = Intl.make({
            "fr": {
                "welcome": "Welcome to $2, mister $1!"
            }
        });
        expect(_("welcome", "Hannibal", "Hell"))
            .toEqual("Welcome to Hell, mister Hannibal!");
    });
});
