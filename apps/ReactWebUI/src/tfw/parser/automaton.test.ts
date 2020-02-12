import Automaton from './automaton'

describe('[tfw/parse/automaton]', () => {
    const separator = ',';
    const ruleEnd = ['', (chr: string, ctx: any) => {
        ctx.items.push(ctx.item)
    }]
    const ruleAny = [null, (chr: string, ctx: any) => { ctx.item += chr }]
    const pushItem = (ctx: any) => {
        ctx.items.push(ctx.item)
        ctx.item = ''
    }
    const grammar = {
        start: [
            ['\\', () => "escapeStart"],
            ['"', (chr: string, ctx: any) => "quote"],
            [separator, (chr: string, ctx: any) => { pushItem(ctx) }],
            ruleEnd,
            ruleAny
        ],
        escapeStart: [
            [null, (chr: string, ctx: any) => {
                ctx.item += chr;
                return "start"
            }]
        ],
        quote: [
            ['\\', () => "escapeQuote"],
            ['"', (chr: string, ctx: any) => {
                ctx.items.push(ctx.item);
                ctx.item = '';
                return "waitSep"
            }],
            ruleEnd,
            ruleAny
        ],
        escapeQuote: [
            [null, (chr: string, ctx: any) => {
                ctx.item += chr;
                return "quote"
            }]
        ],
        waitSep: [
            [separator, () => "start"]
        ]
    }

    it('should deal with CSV line grammar', () => {
        const input = '"one", two trees,"bob,un\\"defined\\"" ,end'
        const context = { items: [], item: '' };
        const output = Automaton.exec(input, grammar, context);
        const expected = ['one', ' two trees', 'bob,un"defined"', 'end'];
        expect(output.items).toEqual(expected);
    })
})
