import Automaton from './automaton'

export default { splitColumns }


/**
 * A line of CSV file is made of items separated by `sep`.
 * An item can be surrounded by double quotes.
 */
function splitColumns(line: string, separator: string = ','): string[] {
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
    const context = Automaton.exec(line, grammar, { items: [], item: '' });
    return context.items;
}
