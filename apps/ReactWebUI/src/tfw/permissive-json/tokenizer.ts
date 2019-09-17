/**
 * @export {function} .tokenize
 * @param {string} source - Stringification of a permissive JSON object.
 * @return {array} Array of tokens. A token is an object like this: `{}`.
 */
const OBJ_OPEN = 1;
const OBJ_CLOSE = 2;
const ARR_OPEN = 3;
const ARR_CLOSE = 4;
const STRING = 5;
const NUMBER = 6;
const SPECIAL = 7;  // true, false, null or undefined.
const COLON = 8;

const TYPE_NAMES = [
    "???",
    "OBJ_OPEN", "OBJ_CLOSE", "ARR_OPEN", "ARR_CLOSE",
    "STRING", "NUMBER", "SPECIAL", "COLON"
];

export default {
    OBJ_OPEN,
    OBJ_CLOSE,
    ARR_OPEN,
    ARR_CLOSE,
    STRING,
    NUMBER,
    SPECIAL,  // true, false, null or undefined.
    COLON,
    tokenize(source: string) {
        const ctx = new Context();
        return ctx.tokenize(source);
    },
    getTypeName: function(type: string) { return TYPE_NAMES[type]; }
};

const RX_DECIMAL = /^-?(\.[0-9]+|[0-9]+(\.[0-9]+)?)([eE]-?[0-9]+)?$/;
const RX_HEXA = /^-?0x[0-9a-f]+$/i;
const RX_OCTAL = /^-?0o[0-7]+$/i;
const RX_BINARY = /^-?0b[01]+$/i;

interface IToken {
    type: number,
    index: number,
    value: string | number
}


class Context {
    private readonly eaters: (() => void)[]
    private source: string = ''
    private index: number = 0
    private end: number = 0
    private tokens: IToken[] = []

    constructor() {
        this.eaters = [
            this.eatBlanks,
            this.eatSymbol,
            this.eatComment,
            this.eatString,
            this.eatIdentifier
        ];
    }

    tokenize(source: string) {
        this.source = source;
        this.end = source.length;
        this.index = 0;
        this.tokens = [];

        while (this.index < this.end) {
            const currentSourceIndex = this.index;
            for (let eaterIndex = 0; eaterIndex < this.eaters.length; eaterIndex++) {
                const eater = this.eaters[eaterIndex];
                eater();
                if (this.index !== currentSourceIndex) break;
            }
            if (this.index === currentSourceIndex) {
                // No eater has been pleased with this char.
                this.fail(`Unexpected token at pos ${this.index}:\n${source.substr(this.index, 80)}`);
            }
        }

        return this.tokens;
    }

    fail(msg: string) {
        if (typeof msg === 'undefined') msg = "Invalid char at " + this.index + "!"

        throw { index: this.index, source: this.source, message: msg }
    }

    eos() {
        return this.index >= this.end
    }

    peek() {
        return this.eos() ? null : this.source[this.index];
    }

    next() {
        return this.eos() ? null : this.source[this.index++];
    }

    back() { if (this.index > 0) this.index--; };

    addToken(type: number, index: number = -1, value: any = '') {
        if (index === -1) index = this.index;
        this.tokens.push({ type, index, value })
    }

    eatBlanks = () => {
        while (" \t\n\r".indexOf(this.peek() || '?') !== -1) this.index++;
    }

    eatSymbol = () => {
        let tkn = null;
        const c = this.peek();
        switch (c) {
            case '{': tkn = OBJ_OPEN; break;
            case '}': tkn = OBJ_CLOSE; break;
            case '[': tkn = ARR_OPEN; break;
            case ']': tkn = ARR_CLOSE; break;
            case ':': tkn = COLON; break;
            case ',':
            // The comma is not mandatory.
            this.index++;
            return;
        }
        if (tkn) {
            this.addToken(tkn);
            this.index++;
        }
    }

    eatComment = () => {
        if (this.peek() !== '/') return;
        this.index++;
        const c = this.next();
        if (c == '/') {
            // Single line comment.
            const endOfSingleComment = this.source.indexOf('\n', this.index);
            if (endOfSingleComment === -1) {
                this.index = this.end;
            } else {
                this.index = endOfSingleComment + 1;
            }
        }
        else if (c == '*') {
            // Multi line comment.
            const endOfComment = this.source.indexOf('*/', this.index);
            if (endOfComment === -1) {
                this.index = this.end;
            } else {
                this.index = endOfComment + 1;
            }
        }
        else {
            this.fail("'/' looks like a comment, but it is not!");
        }
    }

    eatString = () => {
        const start = this.index;
        const quote = this.peek();
        if (quote !== '"' && quote !== "'") return;
        this.index++;
        let escape = false;
        let str = '';
        while (!this.eos()) {
            let c = this.next();
            if (escape) {
                escape = false;
                if (c === 'n') c = '\n';
                else if (c === 'r') c = '\r';
                else if (c === 't') c = '\t';
                str += c;
            }
            else if (c === "\\") {
                escape = true;
            }
            else if (c === quote) {
                this.addToken(STRING, start, str);
                return;
            }
            else {
                str += c;
            }
        }
        this.index = start;
        this.fail("Missing en of string");
    }

    eatIdentifier = () => {
        const start = this.index;
        let c = this.peek() || '?';
        if (" \t\n\r,:[]{}/".indexOf(c) !== -1) return;
        this.index++;
        let str: string | null | undefined | boolean = c;
        while (!this.eos()) {
            c = this.peek() || '?';
            if (" \t\n\r,:[]{}/".indexOf(c) !== -1) break;
            str += c;
            this.index++;
        }
        if (RX_DECIMAL.test(str)) {
            this.addToken(NUMBER, start, parseFloat(str));
        }
        else if (RX_HEXA.test(str)) {
            this.addToken(NUMBER, start, parseInt(str, 16));
        }
        else if (RX_OCTAL.test(str)) {
            if (str.charAt(0) == '-') {
                str = "-" + str.substr(3);
            } else {
                str = str.substr(2);
            }
            this.addToken(NUMBER, start, parseInt(str, 8));
        }
        else if (RX_BINARY.test(str)) {
            if (str.charAt(0) == '-') {
                str = "-" + str.substr(3);
            } else {
                str = str.substr(2);
            }
            this.addToken(NUMBER, start, parseInt(str, 2));
        }
        else {
            let type = SPECIAL;
            if (str === 'null') str = null;
            else if (str === 'undefined') str = undefined;
            else if (str === 'true') str = true;
            else if (str === 'false') str = false;
            else type = STRING;
            this.addToken(type, start, str);
        }
    }
}
