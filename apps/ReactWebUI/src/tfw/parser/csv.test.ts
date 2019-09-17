import CSV from './csv'

describe('[tfw/parse/csv]', () => {
    const line = 'One,two trees ,"bob, bill" ,end.';

    it(`Should split against comma`, () => {
        expect(CSV.splitColumns(line, ',')).toEqual([
            'One',
            'two trees ',
            'bob, bill',
            'end.'
        ]);
    });

    it(`Should split against space`, () => {
        expect(CSV.splitColumns(line, ' ')).toEqual([
            'One,two',
            'trees',
            ',bob, bill',
            ',end.'
        ]);
    });
});
