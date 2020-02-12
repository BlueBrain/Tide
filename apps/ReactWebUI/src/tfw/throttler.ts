type Action = (...args: any[]) => void;

/**
 * The function to call as much as you want. It will perform the debouce for you.
 * Put in the same args as the `action` function.
 *
 * * action -  Action to call. Two consecutive actions cannot be  called if there is
 * less than `delay` ms between them.
 * * delay - Number of milliseconds.
 */
export default function(action: Action, delay: number): Action {
    let timer = 0;
    let timestamp = 0;
    let nextAction: Action = () => {};
    let nextArgs: any[] = [];
    const throttleAction = () => {
        timer = 0;
        nextAction(...nextArgs);
    }

    return function(this: { delay: number }, ...args: any[]) {
        nextAction = action;
        nextArgs = args;
        const now = Date.now();
        const elapsedTime = now - timestamp;
        timestamp = now;
        if (elapsedTime > delay) {
            throttleAction();
        }
        else if (timer === 0) {
            timer = window.setTimeout(throttleAction, delay - elapsedTime);
        }
    }
}
