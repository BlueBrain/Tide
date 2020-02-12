import Iterator from './iterator'

describe('tfw/iterator', () => {
    describe('lines()', () => {
        function check(caption: string, input: string, expected: string[]) {
            it(caption, () => {
                const output = [];
                const itr = Iterator.lines(input);
                for( const line of itr ) {
                    output.push(line);
                }
                expect(output).toEqual(expected);
            })
        }

        check('empty strings', '', []);
        check(
            'single line',
            "I'm a fool, didn't you know?",
            ["I'm a fool, didn't you know?"]
        );
        check(
            'Linux style',
            "Good\nmorning\nVietnam!",
            ["Good", "morning", "Vietnam!"]
        );
        check(
            'Windows style',
            "Good\r\nmorning\r\nVietnam!",
            ["Good", "morning", "Vietnam!"]
        )
    })
})
