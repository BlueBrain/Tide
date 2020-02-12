/**
 * The ESCAPE key is often used to close a dialog, collapse a combo list, ...
 * This module helps you nesting closable things.
 *
 * @example
 * import * from EscapeHandler from "./tfw/escape-handler"
 * EscapeHandler.add(() => myDialog.close());
 * EscapeHandler.fire();
 */

export default {add, fire};

type THandler = ()=>void;

let handlers:THandler[] = [];
let initialized = false;

export function add(listener: THandler) {
    if( !initialized) {
        initialized = true;
        addKeyboardListener();
    }

    handlers.push(listener);
}

export function fire() {
    if( handlers.length === 0 ) return;
    const handler = handlers.pop();
    if (typeof handler === 'function') handler();
}

function addKeyboardListener() {
    document.addEventListener("keydown", evt => {
        if( evt.key !== 'Escape') return;
        fire();
    }, true);
}
