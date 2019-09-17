import Finger from "./finger"

describe("Class Finger", () => {
    it(`Should return 0 for the first call of getIndex()`, () => {
        const fingers = new Finger();
        expect(fingers.getIndex(666)).toEqual(0);
    });
    it(`Should return the same index for the same identifier`, () => {
        const fingers = new Finger();
        fingers.getIndex(666);
        expect(fingers.getIndex(666)).toEqual(0);
    });
    it(`Should return 2 for the third finger`, () => {
        const fingers = new Finger();
        fingers.getIndex(11);
        fingers.getIndex(22);
        expect(fingers.getIndex(666)).toEqual(2);
    });
    it(`Should return 2 for the third finger event if the first one has been removed`, () => {
        const fingers = new Finger();
        fingers.getIndex(11);
        fingers.getIndex(22);
        fingers.getIndex(666);
        fingers.remove(11);
        expect(fingers.getIndex(666)).toEqual(2);
    });
    it(`Should find the first empty finger`, () => {
        const fingers = new Finger();
        fingers.getIndex(11);
        fingers.getIndex(22);
        fingers.getIndex(33);
        fingers.remove(22);
        fingers.getIndex(44);
        fingers.getIndex(55);
        expect(fingers.getIndex(44)).toEqual(1);
    });
})
