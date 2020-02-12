/**
 * Listeners are function with only one argument of type TArgument.
 * @param name [description]
 */
export default class Listeners<TArgument> {
    private listeners: ((arg: TArgument) => void)[] = [];

    constructor(private name: string = "Listener") {}

    /**
     * How many listeners are currently registred.
     */
    get length() {
        return Listeners.length
    }

    add(listener: ((arg: TArgument) => void)) {
        this.remove( listener);
        this.listeners.push(listener);
    }

    remove(listener: ((arg: TArgument) => void)) {
        this.listeners = this.listeners.filter( x => x !== listener);
    }

    fire(argument: TArgument) {
        this.listeners.forEach((listener: ((arg: TArgument) => void)) => {
            try {
                listener(argument);
            } catch(ex) {
                console.error(`[${this.name}] Error in a listener!`);
                console.error(">  ex.: ", ex);
                console.error(">  arg.: ", argument);
            }
        });
    }
}
