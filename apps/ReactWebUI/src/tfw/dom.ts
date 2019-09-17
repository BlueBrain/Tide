export default { div, tag }

interface IAttributes {
    [key: string]: string;
}

type IArg = IAttributes | string | HTMLElement[];

function div(...args: IArg[]) : HTMLDivElement {
    return tag("div", ...args) as HTMLDivElement;
}

function tag(name: string, ...args: IArg[] ): HTMLElement {
    const elem = document.createElement(name);
    args.forEach( arg => {
        if( typeof arg === 'string' ) {
            elem.classList.add( arg );
            return;
        }
        if( Array.isArray(arg) ) {
            arg.forEach( (child: HTMLElement) => {
                elem.appendChild(child);
            });
            return;
        }
        for( const attKey of Object.keys(arg) ) {
            const attVal = arg[attKey];
            elem.setAttribute( attKey, attVal );
        }
    });
    return elem;
}
