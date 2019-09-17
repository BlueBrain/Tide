type IAction = (char: string, context: {}) => string | undefined
type IMatcherFunction = (txt: string) => boolean
type IMatcher = string | RegExp | IMatcherFunction
type IRule = [IMatcher, IAction]
export interface IGrammar { [key: string]: IRule[] }


export default {
    /**
     * The grammar of an automaton describes all its states.
     * The first defined state is the initial one.
     * Each state has rules that are evaluated one by one.
     * If a rule matches, its action is executed and the other rules are skipped.
     * If an action returns a string, this must be the name of the next state.
     * A matcher is either a string (for perfect char matching),
     * a RegExp of a function returning a boolean.
     */
    exec(content: string, grammar: IGrammar, context: {} = {}): any {
        let state = Object.keys(grammar)[0];
        let rules: IRule[] = grammar[state];

        try {
            const execRules = (char: string) => {
                for (const rule of rules) {
                    if (!Array.isArray(rule) || rule.length < 1) {
                        throw Error(`[tfw/parser/automaton::exec] Rules must be arrays: [matcher, action]! But we got: ${rule}`);
                    }
                    const [matcher, action] = rule;
                    if (matches(matcher, char)) {
                        try {
                            const nextState = action(char, context);
                            if (typeof nextState === 'string') {
                                // Changing the current state.
                                rules = grammar[nextState];
                                if (!rules) {
                                    throw Error(`[tfw/parser/automaton::exec] In state "${state}" a rule tried to redirect to unexisting state "${nextState}"!`);
                                }
                                state = nextState;
                            }
                            break;
                        }
                        catch (ex) {
                            throw Error(`[tfw/parser/automaton::exec] Error in Action!\n${ex}`);
                        }
                    }
                }
            }
            for (let i = 0; i < content.length; i++) {
                const char = content.charAt(i);
                try {
                    execRules(char);
                }
                catch (ex) {
                    throw Error(`[tfw/parser/automaton::exec] Error at pos ${i} for char #${content.charCodeAt(i)} "${char}"!\n${ex}`);
                }
            }
            execRules('');
            return context;
        }
        catch (ex) {
            throw Error(`[tfw/parser/automaton::exec] Error in state "${state}".\n${ex}`)
        }
    }
}


function matches(matcher: IMatcher, char: string): boolean {
    if (matcher === null) return true;

    const type = typeof (matcher);
    if (type === 'undefined')
        throw Error("[tfw/parser/automaton::matches] Undefined matcher!")
    if (type === 'string') return char === matcher;
    if (type === 'function') {
        try {
            const matcherFunction = matcher as IMatcherFunction;
            return matcherFunction(char);
        }
        catch (ex) {
            throw Error(`[tfw/parser/automaton::matches] Error in function matcher!\n${ex}`)
        }
    }
    const rx = matcher as RegExp;
    if (typeof rx.test === 'function') {
        const tester = rx.test
        return tester(char);
    }
    throw Error("[tfw/parser/automaton::matches] Invalid matcher!");
}
